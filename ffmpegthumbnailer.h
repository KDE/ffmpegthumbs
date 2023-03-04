/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KFFMPEG_THUMBNAILER_H
#define KFFMPEG_THUMBNAILER_H

#include <KIO/ThumbnailCreator>
#include <QCache>
#include <QObject>

#include <ffmpegthumbnailer/videothumbnailer.h>
#include <ffmpegthumbnailer/filmstripfilter.h>

class QCheckBox;
class QLineEdit;
class QSpinBox;

class FFMpegThumbnailer : public KIO::ThumbnailCreator
{
    Q_OBJECT

private:
    typedef QCache<QString, QImage> ThumbCache;

public:
    explicit FFMpegThumbnailer(QObject *parent, const QVariantList &args);
    ~FFMpegThumbnailer() override;
    KIO::ThumbnailResult create(const KIO::ThumbnailRequest &request) override;

private:
    float updatedSequenceIndexWraparoundPoint(float offset);

    // Assume that the video file has an embedded thumb, in which case it gets inserted before the
    // regular seek percentage-based thumbs. If we find out that the video doesn't have one, we can
    // correct that overestimation.
    KIO::ThumbnailResult pass(const QImage &img, float sequenceIndexWraparoundPoint = 1.0f)
    {
        auto res = KIO::ThumbnailResult::pass(img);
        res.setSequenceIndexWraparoundPoint(sequenceIndexWraparoundPoint);
        return res;
    }

private:
    ffmpegthumbnailer::VideoThumbnailer m_Thumbnailer;
    ffmpegthumbnailer::FilmStripFilter  m_FilmStrip;
    ThumbCache                          m_thumbCache;
};

#endif
