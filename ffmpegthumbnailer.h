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
#include <kio/thumbcreator.h>

#include <ffmpegthumbnailer/videothumbnailer.h>
#include <ffmpegthumbnailer/filmstripfilter.h>

class FFMpegThumbnailer : public QObject, public ThumbCreator
{
    Q_OBJECT
public:
    FFMpegThumbnailer();
    virtual ~FFMpegThumbnailer();
    virtual bool create(const QString& path, int width, int height, QImage& img) override;
    virtual Flags flags() const override;
    virtual QWidget* createConfigurationWidget() override;
    virtual void writeConfiguration(const QWidget* configurationWidget) override;
private:
    ffmpegthumbnailer::VideoThumbnailer m_Thumbnailer;
    ffmpegthumbnailer::FilmStripFilter  m_FilmStrip;
};

#endif
