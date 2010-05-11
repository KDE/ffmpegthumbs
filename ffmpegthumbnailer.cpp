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
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "ffmpegthumbnailer.h"
#include <kdebug.h>
#include <QImage>

extern "C"
{
    KDE_EXPORT ThumbCreator* new_creator()
    {
        return new FFMpegThumbnailer();
    }
}


FFMpegThumbnailer::FFMpegThumbnailer()
{
    m_Thumbnailer.addFilter(&m_FilmStrip);
}

FFMpegThumbnailer::~FFMpegThumbnailer()
{
}

bool FFMpegThumbnailer::create(const QString& path, int width, int /*heigth*/, QImage& img)
{
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
    return (Flags)(DrawFrame);
}

