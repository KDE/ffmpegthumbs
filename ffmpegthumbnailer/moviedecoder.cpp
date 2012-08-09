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

#include "moviedecoder.h"

#include <kdebug.h>
#include <QFileInfo>

extern "C" {
#include <libswscale/swscale.h>
}

using namespace std;

namespace ffmpegthumbnailer
{

MovieDecoder::MovieDecoder(const QString& filename, AVFormatContext* pavContext)
        : m_VideoStream(-1)
        , m_pFormatContext(pavContext)
        , m_pVideoCodecContext(NULL)
        , m_pVideoCodec(NULL)
        , m_pVideoStream(NULL)
        , m_pFrame(NULL)
        , m_pFrameBuffer(NULL)
        , m_pPacket(NULL)
        , m_FormatContextWasGiven(pavContext != NULL)
        , m_AllowSeek(true)
        , m_initialized(false)
{
    initialize(filename);
}

MovieDecoder::~MovieDecoder()
{
    destroy();
}

void MovieDecoder::initialize(const QString& filename)
{
    av_register_all();
    avcodec_register_all();

    QFileInfo fileInfo(filename);

    if ((!m_FormatContextWasGiven) && avformat_open_input(&m_pFormatContext, fileInfo.absoluteFilePath().toLocal8Bit().data(), NULL, NULL) != 0) {
        kDebug() <<  "Could not open input file: " << fileInfo.absoluteFilePath();
        return;
    }

    if (avformat_find_stream_info(m_pFormatContext, 0) < 0) {
        kDebug() << "Could not find stream information";
        return;
    }

    initializeVideo();
    m_pFrame = avcodec_alloc_frame();

    if (m_pFrame) {
        m_initialized=true;
    }
}

bool MovieDecoder::getInitialized()
{
    return m_initialized;
}


void MovieDecoder::destroy()
{
    if (m_pVideoCodecContext) {
        avcodec_close(m_pVideoCodecContext);
        m_pVideoCodecContext = NULL;
    }

    if ((!m_FormatContextWasGiven) && m_pFormatContext) {
        avformat_close_input(&m_pFormatContext);
        m_pFormatContext = NULL;
    }

    if (m_pPacket) {
        av_free_packet(m_pPacket);
        delete m_pPacket;
        m_pPacket = NULL;
    }

    if (m_pFrame) {
        av_free(m_pFrame);
        m_pFrame = NULL;
    }

    if (m_pFrameBuffer) {
        av_free(m_pFrameBuffer);
        m_pFrameBuffer = NULL;
    }
}

QString MovieDecoder::getCodec()
{
    QString codecName;
    if (m_pVideoCodec) {
        codecName=QString::fromLatin1(m_pVideoCodec->name);
    }
    return codecName;
}

void MovieDecoder::initializeVideo()
{
    for (unsigned int i = 0; i < m_pFormatContext->nb_streams; i++) {
        if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_pVideoStream = m_pFormatContext->streams[i];
            m_VideoStream = i;
            break;
        }
    }

    if (m_VideoStream == -1) {
        kDebug() << "Could not find video stream";
        return;
    }

    m_pVideoCodecContext = m_pFormatContext->streams[m_VideoStream]->codec;
    m_pVideoCodec = avcodec_find_decoder(m_pVideoCodecContext->codec_id);

    if (m_pVideoCodec == NULL) {
        // set to NULL, otherwise avcodec_close(m_pVideoCodecContext) crashes
        m_pVideoCodecContext = NULL;
        kDebug() << "Video Codec not found";
        return;
    }

    m_pVideoCodecContext->workaround_bugs = 1;

    if (avcodec_open2(m_pVideoCodecContext, m_pVideoCodec, 0) < 0) {
        kDebug() << "Could not open video codec";
    }
}

int MovieDecoder::getWidth()
{
    if (m_pVideoCodecContext) {
        return m_pVideoCodecContext->width;
    }

    return -1;
}

int MovieDecoder::getHeight()
{
    if (m_pVideoCodecContext) {
        return m_pVideoCodecContext->height;
    }

    return -1;
}

int MovieDecoder::getDuration()
{
    if (m_pFormatContext) {
        return static_cast<int>(m_pFormatContext->duration / AV_TIME_BASE);
    }

    return 0;
}

void MovieDecoder::seek(int timeInSeconds)
{
    if (!m_AllowSeek) {
        return;
    }

    qint64 timestamp = AV_TIME_BASE * static_cast<qint64>(timeInSeconds);

    if (timestamp < 0) {
        timestamp = 0;
    }

    int ret = av_seek_frame(m_pFormatContext, -1, timestamp, 0);
    if (ret >= 0) {
        avcodec_flush_buffers(m_pFormatContext->streams[m_VideoStream]->codec);
    } else {
        kDebug() << "Seeking in video failed";
        return;
    }

    int keyFrameAttempts = 0;
    bool gotFrame = 0;

    do {
        int count = 0;
        gotFrame = 0;

        while (!gotFrame && count < 20) {
            getVideoPacket();
            gotFrame = decodeVideoPacket();
            ++count;
        }

        ++keyFrameAttempts;
    } while ((!gotFrame || !m_pFrame->key_frame) && keyFrameAttempts < 200);

    if (gotFrame == 0) {
        kDebug() << "Seeking in video failed";
    }
}


void MovieDecoder::decodeVideoFrame()
{
    bool frameFinished = false;

    while (!frameFinished && getVideoPacket()) {
        frameFinished = decodeVideoPacket();
    }

    if (!frameFinished) {
        kDebug() << "decodeVideoFrame() failed: frame not finished";
        return;
    }
}

bool MovieDecoder::decodeVideoPacket()
{
    if (m_pPacket->stream_index != m_VideoStream) {
        return false;
    }

    avcodec_get_frame_defaults(m_pFrame);

    int frameFinished = 0;

#if LIBAVCODEC_VERSION_MAJOR < 53
    int bytesDecoded = avcodec_decode_video(m_pVideoCodecContext, m_pFrame, &frameFinished, m_pPacket->data, m_pPacket->size);
#else
    int bytesDecoded = avcodec_decode_video2(m_pVideoCodecContext, m_pFrame, &frameFinished, m_pPacket);
#endif

    if (bytesDecoded < 0) {
        kDebug() << "Failed to decode video frame: bytesDecoded < 0";
    }

    return (frameFinished > 0);
}

bool MovieDecoder::getVideoPacket()
{
    bool framesAvailable = true;
    bool frameDecoded = false;

    int attempts = 0;

    if (m_pPacket) {
        av_free_packet(m_pPacket);
        delete m_pPacket;
    }

    m_pPacket = new AVPacket();

    while (framesAvailable && !frameDecoded && (attempts++ < 1000)) {
        framesAvailable = av_read_frame(m_pFormatContext, m_pPacket) >= 0;
        if (framesAvailable) {
            frameDecoded = m_pPacket->stream_index == m_VideoStream;
            if (!frameDecoded) {
                av_free_packet(m_pPacket);
            }
        }
    }

    return frameDecoded;
}

void MovieDecoder::getScaledVideoFrame(int scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame)
{
    if (m_pFrame->interlaced_frame) {
        avpicture_deinterlace((AVPicture*) m_pFrame, (AVPicture*) m_pFrame, m_pVideoCodecContext->pix_fmt,
                              m_pVideoCodecContext->width, m_pVideoCodecContext->height);
    }

    int scaledWidth, scaledHeight;
    convertAndScaleFrame(PIX_FMT_RGB24, scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);

    videoFrame.width = scaledWidth;
    videoFrame.height = scaledHeight;
    videoFrame.lineSize = m_pFrame->linesize[0];

    videoFrame.frameData.clear();
    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((&(videoFrame.frameData.front())), m_pFrame->data[0], videoFrame.lineSize * videoFrame.height);
}

void MovieDecoder::convertAndScaleFrame(PixelFormat format, int scaledSize, bool maintainAspectRatio, int& scaledWidth, int& scaledHeight)
{
    calculateDimensions(scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);
    SwsContext* scaleContext = sws_getContext(m_pVideoCodecContext->width, m_pVideoCodecContext->height,
                               m_pVideoCodecContext->pix_fmt, scaledWidth, scaledHeight,
                               format, SWS_BICUBIC, NULL, NULL, NULL);

    if (NULL == scaleContext) {
        kDebug() << "Failed to create resize context";
        return;
    }

    AVFrame* convertedFrame = NULL;
    uint8_t* convertedFrameBuffer = NULL;

    createAVFrame(&convertedFrame, &convertedFrameBuffer, scaledWidth, scaledHeight, format);

    sws_scale(scaleContext, m_pFrame->data, m_pFrame->linesize, 0, m_pVideoCodecContext->height,
              convertedFrame->data, convertedFrame->linesize);
    sws_freeContext(scaleContext);

    av_free(m_pFrame);
    av_free(m_pFrameBuffer);

    m_pFrame        = convertedFrame;
    m_pFrameBuffer  = convertedFrameBuffer;
}

void MovieDecoder::calculateDimensions(int squareSize, bool maintainAspectRatio, int& destWidth, int& destHeight)
{
    if (!maintainAspectRatio) {
        destWidth = squareSize;
        destHeight = squareSize;
    } else {
        int srcWidth            = m_pVideoCodecContext->width;
        int srcHeight           = m_pVideoCodecContext->height;
        int ascpectNominator    = m_pVideoCodecContext->sample_aspect_ratio.num;
        int ascpectDenominator  = m_pVideoCodecContext->sample_aspect_ratio.den;

        if (ascpectNominator != 0 && ascpectDenominator != 0) {
            srcWidth = srcWidth * ascpectNominator / ascpectDenominator;
        }

        if (srcWidth > srcHeight) {
            destWidth  = squareSize;
            destHeight = static_cast<int>(static_cast<float>(squareSize) / srcWidth * srcHeight);
        } else {
            destWidth  = static_cast<int>(static_cast<float>(squareSize) / srcHeight * srcWidth);
            destHeight = squareSize;
        }
    }
}

void MovieDecoder::createAVFrame(AVFrame** avFrame, quint8** frameBuffer, int width, int height, PixelFormat format)
{
    *avFrame = avcodec_alloc_frame();

    int numBytes = avpicture_get_size(format, width, height);
    *frameBuffer = reinterpret_cast<quint8*>(av_malloc(numBytes));
    avpicture_fill((AVPicture*) *avFrame, *frameBuffer, format, width, height);
}

}
