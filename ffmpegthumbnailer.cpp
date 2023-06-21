/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
    SPDX-FileCopyrightText: 2020 Heiko Schäfer <heiko@rangun.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ffmpegthumbnailer.h"
#include "ffmpegthumbnailersettings5.h"
#include "ffmpegthumbs_debug.h"

#include <limits>

#include <KLocalizedString>
#include <KPluginFactory>
#include <QCheckBox>
#include <QFormLayout>
#include <QImage>
#include <QLineEdit>
#include <QSpinBox>

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

FFMpegThumbnailer::FFMpegThumbnailer(QObject *parent, const QVariantList &args)
    : KIO::ThumbnailCreator(parent, args)
{
    FFMpegThumbnailerSettings* settings = FFMpegThumbnailerSettings::self();
    if (settings->filmstrip()) {
        m_Thumbnailer.addFilter(&m_FilmStrip);
    }
    m_thumbCache.setMaxCost(settings->cacheSize());
}

FFMpegThumbnailer::~FFMpegThumbnailer()
{
}

KIO::ThumbnailResult FFMpegThumbnailer::create(const KIO::ThumbnailRequest &request)
{
    int seqIdx = static_cast<int>(request.sequenceIndex());
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

    const QString path = request.url().toLocalFile();
    const QString cacheKey = QStringLiteral("%1$%2@%3").arg(path).arg(request.sequenceIndex()).arg(request.targetSize().width());

    QImage* cachedImg = m_thumbCache[cacheKey];
    if (cachedImg) {
        return pass(*cachedImg);
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
                    QString filename(QString::fromUtf8(fname->value));
                    QString noextname = filename.section(QLatin1Char('.'), 0);
                    // Prefer landscape and larger
                    if (noextname == QStringLiteral("cover_land")) {
                        prio = std::numeric_limits<int>::max();
                    }
                    else if (noextname == QStringLiteral("small_cover_land")) {
                        prio = std::numeric_limits<int>::max()-1;
                    }
                    else if (noextname == QStringLiteral("cover")) {
                        prio = std::numeric_limits<int>::max()-2;
                    }
                    else if (noextname == QStringLiteral("small_cover")) {
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

    auto res = KIO::ThumbnailResult::fail();
    if (pic) {
        QImage img;
        img.loadFromData(pic->data, pic->size);
        res = pass(img);
    }
    avformat_close_input(&ct);

    float wraparoundPoint = 1.0f;
    if (!res.image().isNull()) {
        // Video file has an embedded thumbnail -> return it for seqIdx=0 and shift the regular
        // seek percentages one to the right

        res.setSequenceIndexWraparoundPoint(updatedSequenceIndexWraparoundPoint(1.0f));

        if (seqIdx == 0) {
            return res;
        }

        seqIdx--;
    } else {
        wraparoundPoint = updatedSequenceIndexWraparoundPoint(0.0f);
    }

    // The previous modulo could be wrong now if the video had an embedded thumbnail.
    seqIdx %= seekPercentages.size();

    m_Thumbnailer.setThumbnailSize(request.targetSize().width());
    m_Thumbnailer.setSeekPercentage(seekPercentages[seqIdx]);
    //Smart frame selection is very slow compared to the fixed detection
    //TODO: Use smart detection if the image is single colored.
    //m_Thumbnailer.setSmartFrameSelection(true);
    QImage img;
    m_Thumbnailer.generateThumbnail(path, img);

    if (!img.isNull()) {
        // seqIdx 0 will be served from KIO's regular thumbnail cache.
        if (static_cast<int>(request.sequenceIndex()) != 0) {
            const int cacheCost = static_cast<int>((img.sizeInBytes() + 1023) / 1024);
            m_thumbCache.insert(cacheKey, new QImage(img), cacheCost);
        }

        return pass(img, wraparoundPoint);
    }

    return KIO::ThumbnailResult::fail();
}

float FFMpegThumbnailer::updatedSequenceIndexWraparoundPoint(float offset)
{
    float wraparoundPoint = offset;
    if (!FFMpegThumbnailerSettings::sequenceSeekPercentages().isEmpty()) {
        wraparoundPoint += FFMpegThumbnailerSettings::sequenceSeekPercentages().size();
    } else {
        wraparoundPoint += 1.0f;
    }

    return wraparoundPoint;
}

K_PLUGIN_CLASS_WITH_JSON(FFMpegThumbnailer, "ffmpegthumbs.json")

#include "ffmpegthumbnailer.moc"
#include "moc_ffmpegthumbnailer.cpp"
