/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILMSTRIPFILTER_H
#define FILMSTRIPFILTER_H

#include "ifilter.h"

namespace ffmpegthumbnailer
{

class FilmStripFilter : public IFilter
{
public:
    ~FilmStripFilter() override {}
    void process(VideoFrame& videoFrame) override;
};

}

#endif
