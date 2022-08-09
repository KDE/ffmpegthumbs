/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
    SPDX-FileCopyrightText: 2020 Heiko Schäfer <heiko@rangun.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ffmpegthumbnailer.h"
#include "ffmpegthumbnailersettings5.h"
#include "ffmpegthumbs_debug.h"

#include <limits>

#include <QCheckBox>
#include <QFormLayout>
#include <QImage>
#include <QLineEdit>
#include <QSpinBox>
#include <KLocalizedString>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/log.h>
}

namespace {
struct FFmpegLogHandler {
    static void handleMessage(void *ptr, int level, const char *fmt, va_list vargs) {
        Q_UNUSED(ptr);

        const QString message = QString::vasprintf(fmt, vargs);

        switch(level) {
        case AV_LOG_PANIC: // ffmpeg will crash now
            qCCritical(ffmpegthumbs_LOG) << message;
            break;
        case AV_LOG_FATAL: // fatal as in can't decode, not crash
        case AV_LOG_ERROR:
        case AV_LOG_WARNING:
            qCWarning(ffmpegthumbs_LOG) << message;
            break;
        case AV_LOG_INFO:
            qCInfo(ffmpegthumbs_LOG) << message;
            break;
        case AV_LOG_VERBOSE:
        case AV_LOG_DEBUG:
        case AV_LOG_TRACE:
            qCDebug(ffmpegthumbs_LOG) << message;
            break;
        default:
            qCWarning(ffmpegthumbs_LOG) << "unhandled log level" << level << message;
            break;
        }
    }

    FFmpegLogHandler() {
        av_log_set_callback(&FFmpegLogHandler::handleMessage);
    }
};
} //namespace

extern "C"
{
    Q_DECL_EXPORT ThumbCreator *new_creator()
    {
        // This is a threadsafe way to ensure that we only register it once
        static FFmpegLogHandler handler;

        return new FFMpegThumbnailer();
    }
}

FFMpegThumbnailer::FFMpegThumbnailer()
{
    FFMpegThumbnailerSettings* settings = FFMpegThumbnailerSettings::self();
    if (settings->filmstrip()) {
        m_Thumbnailer.addFilter(&m_FilmStrip);
    }
    m_thumbCache.setMaxCost(settings->cacheSize());

    // Assume that the video file has an embedded thumb, in which case it gets inserted before the
    // regular seek percentage-based thumbs. If we find out that the video doesn't have one, we can
    // correct that overestimation.
    updateSequenceIndexWraparoundPoint(1.0f);
}

FFMpegThumbnailer::~FFMpegThumbnailer()
{
}

bool FFMpegThumbnailer::create(const QString& path, int width, int /*height*/, QImage& img)
{
    int seqIdx = static_cast<int>(sequenceIndex());
    if (seqIdx < 0) {
        seqIdx = 0;
    }

    QList<int> seekPercentages = FFMpegThumbnailerSettings::sequenceSeekPercentages();
    if (seekPercentages.isEmpty()) {
        seekPercentages.append(20);
    }

    // We might have an embedded thumb in the video file, so we have to add 1. This gets corrected
    // later if we don't have one.
    seqIdx %= static_cast<int>(seekPercentages.size()) + 1;

    const QString cacheKey = QString("%1$%2@%3").arg(path).arg(seqIdx).arg(width);

    QImage* cachedImg = m_thumbCache[cacheKey];
    if (cachedImg) {
        img = *cachedImg;
        return true;
    }

    // Try reading thumbnail embedded into video file
    QByteArray ba = path.toLocal8Bit();
    AVFormatContext* ct = avformat_alloc_context();
    AVPacket* pic = nullptr;

    // No matter the seqIdx, we have to know if the video has an embedded cover, even if we then don't return
    // it. We could cache it to avoid repeating this for higher seqIdx values, but this should be fast enough
    // to not be noticeable and caching adds unnecessary complexity.
    if (ct && !avformat_open_input(&ct,ba.data(), nullptr, nullptr)) {

        // Using an priority system based on size or filename (matroska specification) to select the most suitable picture
        int bestPrio = 0;
        for (size_t i = 0; i < ct->nb_streams; ++i) {
            if (ct->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                int prio = 0;
                AVDictionaryEntry* fname = av_dict_get(ct->streams[i]->metadata, "filename", nullptr ,0);
                if (fname) {
                    QString filename(fname->value);
                    QString noextname = filename.section('.', 0);
                    // Prefer landscape and larger
                    if (noextname == "cover_land") {
                        prio = std::numeric_limits<int>::max();
                    }
                    else if (noextname == "small_cover_land") {
                        prio = std::numeric_limits<int>::max()-1;
                    }
                    else if (noextname == "cover") {
                        prio = std::numeric_limits<int>::max()-2;
                    }
                    else if (noextname == "small_cover") {
                        prio = std::numeric_limits<int>::max()-3;
                    }
                    else {
                        prio = ct->streams[i]->attached_pic.size;
                    }
                }
                else {
                    prio = ct->streams[i]->attached_pic.size;
                }
                if (prio > bestPrio) {
                    pic = &(ct->streams[i]->attached_pic);
                    bestPrio = prio;
                }
            }
        }
    }
    
    if (pic) {
        img.loadFromData(pic->data, pic->size);
    }
    avformat_close_input(&ct);

    if (!img.isNull()) {
        // Video file has an embedded thumbnail -> return it for seqIdx=0 and shift the regular
        // seek percentages one to the right

        updateSequenceIndexWraparoundPoint(1.0f);

        if (seqIdx == 0) {
            return true;
        }

        seqIdx--;
    } else {
        updateSequenceIndexWraparoundPoint(0.0f);
    }

    // The previous modulo could be wrong now if the video had an embedded thumbnail.
    seqIdx %= seekPercentages.size();

    m_Thumbnailer.setThumbnailSize(width);
    m_Thumbnailer.setSeekPercentage(seekPercentages[seqIdx]);
    //Smart frame selection is very slow compared to the fixed detection
    //TODO: Use smart detection if the image is single colored.
    //m_Thumbnailer.setSmartFrameSelection(true);
    m_Thumbnailer.generateThumbnail(path, img);

    if (!img.isNull()) {
        // seqIdx 0 will be served from KIO's regular thumbnail cache.
        if (static_cast<int>(sequenceIndex()) != 0) {
            const int cacheCost = static_cast<int>((img.sizeInBytes() + 1023) / 1024);
            m_thumbCache.insert(cacheKey, new QImage(img), cacheCost);
        }

        return true;
    }

    return false;
}

void FFMpegThumbnailer::updateSequenceIndexWraparoundPoint(float offset)
{
    float wraparoundPoint = offset;

    if (!FFMpegThumbnailerSettings::sequenceSeekPercentages().isEmpty()) {
        wraparoundPoint += FFMpegThumbnailerSettings::sequenceSeekPercentages().size();
    } else {
        wraparoundPoint += 1.0f;
    }

    setSequenceIndexWraparoundPoint(wraparoundPoint);
}
