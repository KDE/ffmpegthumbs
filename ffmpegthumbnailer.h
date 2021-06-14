//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#ifndef KFFMPEG_THUMBNAILER_H
#define KFFMPEG_THUMBNAILER_H

#include <QObject>
#include <QCache>
#include <kio/thumbsequencecreator.h>

#include <ffmpegthumbnailer/videothumbnailer.h>
#include <ffmpegthumbnailer/filmstripfilter.h>

class QCheckBox;
class QLineEdit;
class QSpinBox;

class FFMpegThumbnailer : public QObject, public ThumbSequenceCreator
{
    Q_OBJECT

private:
    typedef QCache<QString, QImage> ThumbCache;

public:
    FFMpegThumbnailer();
    ~FFMpegThumbnailer() override;
    bool create(const QString& path, int width, int height, QImage& img) override;
    Flags flags() const override;
    QWidget* createConfigurationWidget() override;
    void writeConfiguration(const QWidget* configurationWidget) override;

private:
    void updateSequenceIndexWraparoundPoint(float offset);

private:
    ffmpegthumbnailer::VideoThumbnailer m_Thumbnailer;
    ffmpegthumbnailer::FilmStripFilter  m_FilmStrip;
    ThumbCache                          m_thumbCache;
};

#endif
