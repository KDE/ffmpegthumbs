/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H

#include <inttypes.h>
#include <vector>
#include <QtGlobal>

namespace ffmpegthumbnailer
{

struct VideoFrame {
    VideoFrame()
            : width(0), height(0), lineSize(0) {}

    VideoFrame(int width, int height, int lineSize)
            : width(width), height(height), lineSize(lineSize) {}

    quint32 width;
    quint32 height;
    quint32 lineSize;

    std::vector<quint8> frameData;
};

}

#endif
