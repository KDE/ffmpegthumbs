/*
    SPDX-FileCopyrightText: 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MOVIEDECODER_H
#define MOVIEDECODER_H

#include "videoframe.h"
#include <QString>
#include <QImageIOHandler>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

namespace ffmpegthumbnailer
{

class MovieDecoder
{
public:
    explicit MovieDecoder(const QString& filename, AVFormatContext* pavContext = nullptr);
    ~MovieDecoder();

    QString getCodec();
    void seek(int timeInSeconds);
    bool decodeVideoFrame();
    void getScaledVideoFrame(int scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame);

    int getWidth();
    int getHeight();
    int getDuration();

    void initialize(const QString& filename);
    void destroy();
    bool getInitialized();

    QImageIOHandler::Transformations transformations();

private:
    bool initializeVideo();

    bool decodeVideoPacket();
    bool getVideoPacket();
    void convertAndScaleFrame(AVPixelFormat format, int scaledSize, bool maintainAspectRatio, int& scaledWidth, int& scaledHeight);
    void createAVFrame(AVFrame** avFrame, quint8** frameBuffer, int width, int height, AVPixelFormat format);
    void calculateDimensions(int squareSize, bool maintainAspectRatio, int& destWidth, int& destHeight);

    void deleteFilterGraph();
    bool initFilterGraph(enum AVPixelFormat pixfmt, int width, int height);
    bool processFilterGraph(AVFrame *dst, const AVFrame *src, enum AVPixelFormat pixfmt, int width, int height);

private:
    int                     m_VideoStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pVideoCodecContext;
#if LIBAVCODEC_VERSION_MAJOR < 59
    AVCodec*                m_pVideoCodec;
#else
    const AVCodec*          m_pVideoCodec;
#endif
    AVStream*               m_pVideoStream;
    AVFrame*                m_pFrame;
    quint8*                 m_pFrameBuffer;
    AVPacket*               m_pPacket;
    bool                    m_FormatContextWasGiven;
    bool                    m_AllowSeek;
    bool                    m_initialized;
    AVFilterContext*        m_bufferSinkContext;
    AVFilterContext*        m_bufferSourceContext;
    AVFilterGraph*          m_filterGraph;
    AVFrame*                m_filterFrame;
    int                     m_lastWidth;
    int                     m_lastHeight;
    enum AVPixelFormat      m_lastPixfmt;
};

}

#endif
