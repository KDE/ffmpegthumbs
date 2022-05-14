/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGE_WRITER_H
#define IMAGE_WRITER_H

#include "videoframe.h"
#include <string>
#include <inttypes.h>
#include <QImage>
#include <QImageIOHandler>

namespace ffmpegthumbnailer
{

class ImageWriter
{
public:
    ImageWriter();
    virtual ~ImageWriter() {}

    virtual void writeFrame(VideoFrame& frame, QImage& image, const QImageIOHandler::Transformations transformations);
};

}

#endif
