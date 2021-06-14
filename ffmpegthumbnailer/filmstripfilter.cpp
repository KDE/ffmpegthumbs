/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filmstripfilter.h"
#include "filmstrip.h"

namespace ffmpegthumbnailer
{

static const int FILMHOLE_WIDTH = 12;
static const int FILMHOLE_HEIGHT = 10;

static const quint8* determineFilmStrip(quint32 videoWidth, quint32& filmStripWidth, quint32& filmStripHeight)
{
    if (videoWidth <= SMALLEST_FILM_STRIP_WIDTH * 2)
    {
        return nullptr;
    }
    
    if (videoWidth <= 96)
    {
        filmStripWidth = filmStripHeight = 4;
        return filmStrip4;
    }

    if (videoWidth <= 192)
    {
        filmStripWidth = filmStripHeight = 8;
        return filmStrip8;
    }

    if (videoWidth <= 384)
    {
        filmStripWidth = filmStripHeight = 16;
        return filmStrip16;
    }

    if (videoWidth <= 768)
    {
        filmStripWidth = filmStripHeight = 32;
        return filmStrip32;
    }

    filmStripWidth = filmStripHeight = 64;
    return filmStrip64;
}



void FilmStripFilter::process(VideoFrame& videoFrame)
{
    quint32 filmStripWidth;
    quint32 filmStripHeight;
    const quint8* filmHole = determineFilmStrip(videoFrame.width, filmStripWidth, filmStripHeight);
    
    if (!filmHole)
    {
        return;
    }
    
    int frameIndex = 0;
    int filmHoleIndex = 0;
    int offset = (videoFrame.width * 3) - 3;

    for (quint32 i = 0; i < videoFrame.height; ++i)
    {
        for (quint32 j = 0; j < filmStripWidth * 3; j+=3)
        {
            int currentFilmHoleIndex = filmHoleIndex + j;

            videoFrame.frameData[frameIndex + j]     = filmHole[currentFilmHoleIndex];
            videoFrame.frameData[frameIndex + j + 1] = filmHole[currentFilmHoleIndex + 1];
            videoFrame.frameData[frameIndex + j + 2] = filmHole[currentFilmHoleIndex + 2];

            videoFrame.frameData[frameIndex + offset - j]     = filmHole[currentFilmHoleIndex];
            videoFrame.frameData[frameIndex + offset - j + 1] = filmHole[currentFilmHoleIndex + 1];
            videoFrame.frameData[frameIndex + offset - j + 2] = filmHole[currentFilmHoleIndex + 2];
        }
        frameIndex += videoFrame.lineSize;
        filmHoleIndex = (i % filmStripHeight) * filmStripWidth * 3;
    }   
}

}
