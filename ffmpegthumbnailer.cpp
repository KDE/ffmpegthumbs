//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//    Copyright (C) 2020 Heiko Schäfer <heiko@rangun.de>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "ffmpegthumbnailer.h"
#include "ffmpegthumbnailersettings5.h"
#include "ffmpegthumbs_debug.h"

#include <mp4file.h>

#include <QImage>
#include <QCheckBox>
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
}

FFMpegThumbnailer::~FFMpegThumbnailer()
{
}

bool FFMpegThumbnailer::create(const QString& path, int width, int /*height*/, QImage& img)
{
    QByteArray ba = path.toLocal8Bit();
    TagLib::MP4::File f(ba.data(), false);

    if (f.isValid()) {

        TagLib::MP4::Tag* tag = f.tag();
        TagLib::MP4::ItemListMap itemsListMap = tag->itemListMap();
        TagLib::MP4::Item coverItem = itemsListMap["covr"];
        TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();

        if (!coverArtList.isEmpty()) {
            TagLib::MP4::CoverArt coverArt = coverArtList.front();
            img.loadFromData((const uchar *)coverArt.data().data(),
                         coverArt.data().size());

            if (!img.isNull()) return true;
        }
    }

    m_Thumbnailer.setThumbnailSize(width);
    // 20% seek inside the video to generate the preview
    m_Thumbnailer.setSeekPercentage(20);
    //Smart frame selection is very slow compared to the fixed detection
    //TODO: Use smart detection if the image is single colored.
    //m_Thumbnailer.setSmartFrameSelection(true);
    m_Thumbnailer.generateThumbnail(path, img);

    return !img.isNull();
}

ThumbCreator::Flags FFMpegThumbnailer::flags() const
{
    return (Flags)(None);
}

QWidget *FFMpegThumbnailer::createConfigurationWidget()
{
    QCheckBox *filmstripCheckBox = new QCheckBox(i18nc("@option:check", "Embed filmstrip effect"));
    filmstripCheckBox->setChecked(FFMpegThumbnailerSettings::filmstrip());
    return filmstripCheckBox;
}

void FFMpegThumbnailer::writeConfiguration(const QWidget *configurationWidget)
{
    const QCheckBox *filmstripCheckBox = qobject_cast<const QCheckBox*>(configurationWidget);
    if (filmstripCheckBox) {
        FFMpegThumbnailerSettings* settings = FFMpegThumbnailerSettings::self();
        settings->setFilmstrip(filmstripCheckBox->isChecked());
        settings->save();
    }
}
