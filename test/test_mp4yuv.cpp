//#ifndef UINT64_C(val)
//#define UINT64_C(val) val##ULL
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

//ffmpeg都是使用C语言编写，如果是用C++引入头文件时记得加上extern "C"
extern "C" {
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"

}

#define CONFIG_AVDEVICE 1

const char* SRC_FILE = "../../videoDB/t12.mp4";

int getRotateAngle(const AVStream *avStream) {
    AVDictionaryEntry *tag = NULL;
    int rotate = -1;
    tag = av_dict_get(avStream->metadata, "rotate", tag, 0);
    if (tag==NULL) {
        rotate = 0;
    } else {
        int angle = atoi(tag->value);
        angle %= 360;
        if (angle == 90) {
            rotate = 90;
        } else if (angle == 180) {
            rotate = 180;
        } else if (angle == 270) {
            rotate = 270;
        } else {
            rotate = 0;
        }
    }

    return rotate;
}

void Rotate90(const AVFrame* src, AVFrame* dst) {
    int half_width   = src->width >> 1;
    int half_height  = src->height >> 1;
    int size         = src->linesize[0] * src->height;
    int half_size    = size >> 2;
    
    for (int j = 0, n = 0; j < src->width; j++) {
        int pos = size;
        for (int i = src->height - 1; i >= 0; i--) {
            pos -= src->linesize[0];
            dst->data[0][n++] = src->data[0][pos + j];
        }
    }
    for (int j = 0, n = 0; j < half_width; j++) {
        int pos = half_size;
        for (int i = half_height - 1; i >= 0; i--) {
            pos -= src->linesize[1];
            dst->data[1][n] = src->data[1][pos + j];
            dst->data[2][n++] = src->data[2][pos + j];
        }
    }

    dst->height = src->width;
    dst->width  = src->height;
}


void Rotate270(const AVFrame* src, AVFrame* dst) {
    int half_width = src->linesize[0] >> 1;
    int half_height = src->height >> 1;

    for (int i = src->width - 1, n = 0; i >= 0; i--) {
        for (int j = 0, pos = 0; j < src->height; j++) {
            dst->data[0][n++] = src->data[0][pos + i];
            pos += src->linesize[0];
        }
    }

    for (int i = (src->width >> 1) - 1, n = 0; i >= 0; i--) {
        for (int j = 0, pos = 0; j < half_height; j++) {
            dst->data[1][n] = src->data[1][pos + i];
            dst->data[2][n++] = src->data[2][pos + i];
            pos += half_width;
        }
    }
    dst->width  = src->height;
    dst->height = src->width;
}

int main()
{
    FILE *yuv_file = fopen("yuv_file.yuv","wb");
    if (!yuv_file)
        return 0;

    // 1. register all codecs, demux and protocols
    avdevice_register_all();

    //2. 得到一个ffmpeg的上下文（上下文里面封装了视频的比特率，分辨率等等信息...非常重要）
    AVFormatContext* pContext = avformat_alloc_context();
    if (!pContext) {
        std::cout << "could not allocate context." << std::endl;
        return 0;
    }

    //3. 打开视频
    if (avformat_open_input(&pContext, SRC_FILE, NULL, NULL) < 0) {
        std::cout << "open failed." << std::endl;
        return 0;
    }

    //4. 获取视频信息，视频信息封装在上下文中
    if (avformat_find_stream_info(pContext, NULL) < 0) {
        std::cout << "get the information failed." << std::endl;
        return 0;
    }

    //5. 用来记住视频流的索引
    int video_stream_idx = -1;
    //从上下文中寻找找到视频流
    AVStream *st = NULL;
    for (int i = 0; i < pContext->nb_streams; ++i) {
        //codec：每一个流 对应的解码上下文
        //codec_type：流的类型
        st = pContext->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    if (video_stream_idx == -1) {
        std::cout << "can not find video stream." << std::endl;
        return 0;
    }

    int rotate = getRotateAngle(st);
    std::cout << "---->rotate: " << rotate << std::endl;
    av_dump_format(pContext, 0, SRC_FILE, 0);

    //6. 获取编码器上下文和编码器
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(NULL);
    if (!pCodecCtx) {
        std::cout << "get codec context failed." << std::endl;
        return 0;
    }

    if (avcodec_parameters_to_context(pCodecCtx, pContext->streams[video_stream_idx]->codecpar) < 0) {
        std::cout << "get codec parameters failed." << std::endl;
        return 0;
    }

    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    
    //7. 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        std::cout << "decode the video stream failed." << std::endl;
        return 0;
    }

    AVPacket *pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(pkt);
    AVFrame *pFrame = av_frame_alloc();

    int f = -1;
    int f1 = -1;
    int f2 = -1;

    while (true) {
        if (av_read_frame(pContext, pkt) < 0) {
            fclose(yuv_file);
            break;
        }

        if (pkt->stream_index != video_stream_idx) {
            continue;
        }
        
        AVFrame *pFrame = av_frame_alloc();
        AVFrame *pFrameR = av_frame_alloc();

        int got_picture = 0;
        int ret = 0;

        ret = avcodec_decode_video2(pCodecCtx, pFrameR, &got_picture, pkt);
        ++f1;

        if (ret < 0) {
            break;
        }

        if (rotate == 90) {
            uint8_t* buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1));
            av_image_fill_arrays(pFrame->data, pFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1);
            Rotate90(pFrameR, pFrame);
        } else if (rotate == 270) {
            uint8_t* buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1));
            av_image_fill_arrays(pFrame->data, pFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1);
            Rotate270(pFrameR, pFrame);
        } else {
            pFrame = pFrameR;
        }

        ++f2;

        // std::cout << "got_picture: " << got_picture << std::endl;
        if (got_picture) {
            ++f;
            char* buf = new char[pCodecCtx->height * pCodecCtx->width * 3 / 2];
            memset(buf, 0, pCodecCtx->height * pCodecCtx->width * 3 / 2);

            int width, height;

            if (rotate == 90 || rotate == 270) {
                height = pCodecCtx->width;
                width  = pCodecCtx->height;
            } else {
                width = pCodecCtx->width;
                height = pCodecCtx->height;
            }
            
            int a = 0, i;
            for (i = 0; i<height; i++) {
                memcpy(buf + a, pFrame->data[0] + i * pFrame->linesize[0], width);
                a += width;
            }
            for (i = 0; i<height / 2; i++) {
                memcpy(buf + a, pFrame->data[1] + i * pFrame->linesize[1], width / 2);
                a += width / 2;
            }
            for (i = 0; i<height / 2; i++) {
                memcpy(buf + a, pFrame->data[2] + i * pFrame->linesize[2], width / 2);
                a += width / 2;
            }
            fwrite(buf, 1, pCodecCtx->height * pCodecCtx->width * 3 / 2, yuv_file);
            
            delete[] buf;
            buf = NULL;
        }
        av_frame_free(&pFrame);
    }

    // std::cout << "the f is : " << f << ", f1 is : " << f1 << ", f2 is : " << f2 << std::endl;

    fclose(yuv_file);
    avcodec_close(pCodecCtx);
    avformat_free_context(pContext);

    return 0;
}