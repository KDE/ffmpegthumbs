/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <string.h>

namespace ffmpegthumbnailer
{

template <typename T>
struct Histogram {
    T r[256];
    T g[256];
    T b[256];

    Histogram() {
        memset(r, 0, 255 * sizeof(T));
        memset(g, 0, 255 * sizeof(T));
        memset(b, 0, 255 * sizeof(T));
    }
};

}

#endif
