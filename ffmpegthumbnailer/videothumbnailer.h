/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIDEO_THUMBNAILER_H
#define VIDEO_THUMBNAILER_H

#include <string>
#include <vector>
#include <map>
#include <inttypes.h>

#include "ifilter.h"
#include "histogram.h"
#include <QString>
#include <QImage>


namespace ffmpegthumbnailer
{

struct VideoFrame;
class ImageWriter;
class MovieDecoder;

class VideoThumbnailer
{
public:
    VideoThumbnailer();
    VideoThumbnailer(int thumbnailSize, bool workaroundIssues, bool maintainAspectRatio, bool smartFrameSelection);
    ~VideoThumbnailer();

    void generateThumbnail(const QString& videoFile, QImage &image);

    void setThumbnailSize(int size);
    void setSeekPercentage(int percentage);
    void setSeekTime(const QString& seekTime);
    void setWorkAroundIssues(bool workAround);
    void setMaintainAspectRatio(bool enabled);
    void setSmartFrameSelection(bool enabled);
    void addFilter(IFilter* filter);
    void removeFilter(IFilter* filter);
    void clearFilters();

private:
    void generateThumbnail(const QString& videoFile, ImageWriter& imageWriter, QImage& image);
    void generateSmartThumbnail(MovieDecoder& movieDecoder, VideoFrame& videoFrame);

    QString getMimeType(const QString& videoFile);
    QString getExtension(const QString& videoFilename);

    void generateHistogram(const VideoFrame& videoFrame, Histogram<int>& histogram);
    int getBestThumbnailIndex(std::vector<VideoFrame>& videoFrames, const std::vector<Histogram<int> >& histograms);
    void applyFilters(VideoFrame& frameData);

private:
    int                         m_ThumbnailSize;
    quint16                     m_SeekPercentage;
    bool                        m_OverlayFilmStrip;
    bool                        m_WorkAroundIssues;
    bool                        m_MaintainAspectRatio;
    bool                        m_SmartFrameSelection;
    QString                     m_SeekTime;
    std::vector<IFilter*>       m_Filters;
};

}

#endif
