/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
    SPDX-FileCopyrightText: 2020 Heiko Schäfer <heiko@rangun.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ffmpegthumbnailer.h"
#include "ffmpegthumbnailersettings5.h"
#include "ffmpegthumbs_debug.h"

#include <limits>

#include <mp4file.h>

#include <QCheckBox>
#include <QFormLayout>
#include <QImage>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QSpinBox>
#include <KLocalizedString>

extern "C" {
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
    TagLib::MP4::File f(ba.data(), false);

    // No matter the seqIdx, we have to know if the video has an embedded cover, even if we then don't return
    // it. We could cache it to avoid repeating this for higher seqIdx values, but this should be fast enough
    // to not be noticeable and caching adds unnecessary complexity.
    if (f.isValid()) {
        TagLib::MP4::Tag* tag = f.tag();
        TagLib::MP4::ItemListMap itemsListMap = tag->itemListMap();
        TagLib::MP4::Item coverItem = itemsListMap["covr"];
        TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();

        if (!coverArtList.isEmpty()) {
            TagLib::MP4::CoverArt coverArt = coverArtList.front();
            img.loadFromData((const uchar *)coverArt.data().data(),
                         coverArt.data().size());
        }
    }

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

ThumbCreator::Flags FFMpegThumbnailer::flags() const
{
    return (Flags)(None);
}

QWidget *FFMpegThumbnailer::createConfigurationWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* formLayout = new QFormLayout(widget);

    QCheckBox* addFilmStripCheckBox = new QCheckBox (
            i18nc("@option:check For checkbox labeled 'Embed filmstrip effect'", "Embed"));
    addFilmStripCheckBox->setObjectName("FFMpegThumbnailer::addFilmStripCheckBox");
    addFilmStripCheckBox->setChecked(FFMpegThumbnailerSettings::filmstrip());
    formLayout->addRow(i18nc("@label:checkbox", "Embed filmstrip effect"), addFilmStripCheckBox);

    QString seekPercentagesStr;
    for (const int sp : FFMpegThumbnailerSettings::sequenceSeekPercentages()) {
        if (!seekPercentagesStr.isEmpty()) {
            seekPercentagesStr.append(' ');
        }
        seekPercentagesStr.append(QString::number(sp));
    }

    QLineEdit* sequenceSeekPercentagesLineEdit = new QLineEdit();
    sequenceSeekPercentagesLineEdit->setObjectName("FFMpegThumbnailer::sequenceSeekPercentagesLineEdit");
    sequenceSeekPercentagesLineEdit->setText(seekPercentagesStr);
    sequenceSeekPercentagesLineEdit->setToolTip(i18nc("@info:tooltip",
            "List of integers, separated by space or comma"));
    formLayout->addRow(i18nc("@label:textbox", "Sequence seek percentages"), sequenceSeekPercentagesLineEdit);

    QSpinBox* thumbCacheSizeSpinBox = new QSpinBox();
    thumbCacheSizeSpinBox->setObjectName("FFMpegThumbnailer::thumbCacheSizeSpinBox");
    thumbCacheSizeSpinBox->setRange(0, std::numeric_limits<int>::max());
    thumbCacheSizeSpinBox->setValue(FFMpegThumbnailerSettings::cacheSize());
    thumbCacheSizeSpinBox->setSuffix(i18nc("Kibibyte used as a spinbox suffix", " KiB"));
    formLayout->addRow(i18nc("@label:spinbox", "Cache size"), thumbCacheSizeSpinBox);

    return widget;
}

void FFMpegThumbnailer::writeConfiguration(const QWidget* configurationWidget)
{
    if (!configurationWidget) {
        qCCritical(ffmpegthumbs_LOG) << "Invalid configuration widget";
        return;
    }

    QCheckBox* addFilmStripCheckBox = configurationWidget->findChild<QCheckBox*> (
            "FFMpegThumbnailer::addFilmStripCheckBox");
    QLineEdit* sequenceSeekPercentagesLineEdit = configurationWidget->findChild<QLineEdit*> (
            "FFMpegThumbnailer::sequenceSeekPercentagesLineEdit");
    QSpinBox* thumbCacheSizeSpinBox = configurationWidget->findChild<QSpinBox*> (
            "FFMpegThumbnailer::thumbCacheSizeSpinBox");

    if (!addFilmStripCheckBox || !sequenceSeekPercentagesLineEdit || !thumbCacheSizeSpinBox) {
        qCCritical(ffmpegthumbs_LOG) << "Invalid configuration widget";
        return;
    }

    FFMpegThumbnailerSettings* settings = FFMpegThumbnailerSettings::self();

    settings->setFilmstrip(addFilmStripCheckBox->isChecked());

    const QString seekPercentagesStr = sequenceSeekPercentagesLineEdit->text();
    const QVector<QStringRef> seekPercentagesStrList = seekPercentagesStr.splitRef (
            QRegularExpression("(\\s*,\\s*)|\\s+"), Qt::SkipEmptyParts);
    QList<int> seekPercentages;
    bool seekPercentagesValid = true;

    for (const QStringRef& str : seekPercentagesStrList) {
        const int sp = str.toInt(&seekPercentagesValid);

        if (!seekPercentagesValid) {
            break;
        }

        seekPercentages << sp;
    }

    if (seekPercentagesValid) {
        settings->setSequenceSeekPercentages(seekPercentages);
    }

    settings->setCacheSize(thumbCacheSizeSpinBox->value());
    m_thumbCache.setMaxCost(thumbCacheSizeSpinBox->value());

    // Assume that the video file has an embedded thumb, in which case it gets inserted before the
    // regular seek percentage-based thumbs. If we find out that the video doesn't have one, we can
    // correct that overestimation.
    updateSequenceIndexWraparoundPoint(1.0);

    settings->save();
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
