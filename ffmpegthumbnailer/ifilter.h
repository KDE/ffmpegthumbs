/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IFILTER_H
#define IFILTER_H

#include "videoframe.h"

namespace ffmpegthumbnailer
{

class IFilter
{
public:
    virtual ~IFilter() {};
    virtual void process(VideoFrame& frameData) = 0;
};

}

#endif
