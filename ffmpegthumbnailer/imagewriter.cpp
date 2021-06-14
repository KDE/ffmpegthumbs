/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imagewriter.h"
#include <iostream>

using namespace std;

namespace ffmpegthumbnailer
{

ImageWriter::ImageWriter()
{
}

void ImageWriter::writeFrame(VideoFrame& frame,  QImage& image)
{
    QImage previewImage(frame.width, frame.height, QImage::Format_RGB888);
    for (quint32 y = 0; y < frame.height; y++) {
        // Copy each line ..
        memcpy(previewImage.scanLine(y), &frame.frameData[y*frame.lineSize], frame.width*3);
    }

    image = previewImage;
}
}
