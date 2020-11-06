#include <android/log.h>
#define LOGE(...) __android_log_print(6, "ffmpeg", __VA_ARGS__);
#include "com_zhougang_camera2encode_H264Encoder.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavfilter/avfilter.h"
    #include "libswscale/swscale.h"
    #include "libavutil/time.h"

    #include <ffmpeg.h>
}
AVFormatContext *ofmt_ctx;
AVStream* video_st;
AVCodecContext* pCodecCtx;
AVCodec* pCodec;
AVPacket enc_pkt;
AVFrame* pFrameYUV;
int framecnt = 0;
int yuv_width;
int yuv_height;
int y_length;
int uv_length;
int64_t start_time;
extern "C"
JNIEXPORT jint JNICALL Java_com_zhougang_camera2encode_H264Encoder_encodeInitial
  (JNIEnv * env, jclass obj,jint width, jint height, jstring js){
    const char* out_path = (char*)env->GetStringUTFChars(js,0);
    LOGE("output %s ",out_path);
    yuv_width = width;
    yuv_height = height;
    y_length = width * height;
    uv_length = width * height / 4;
    av_register_all();
    avformat_alloc_output_context2(&ofmt_ctx,NULL,NULL,out_path);
    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pCodec){
        LOGE("Cannot find encoder!\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 30;
    pCodecCtx->bit_rate = 800000;
    pCodecCtx->gop_size = 300;
    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER){
        pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    pCodecCtx->qmin =10;
    pCodecCtx->qmax = 51;
    pCodecCtx->max_b_frames = 3;
    AVDictionary* param = 0;
    av_dict_set(&param, "preset", "slow", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);
    if (avcodec_open2(pCodecCtx,pCodec,&param) < 0){
        LOGE("Failed to open encoder!\n");
        return -1;
    }
    video_st = avformat_new_stream(ofmt_ctx,pCodec);
    if (video_st == NULL){
        return -1;
    }
    video_st->time_base.num = 1;
    video_st->time_base.den = 30;
    video_st->codec = pCodecCtx;
    if (avio_open(&ofmt_ctx->pb,out_path,AVIO_FLAG_READ_WRITE) < 0){
        LOGE("Failed to open output file!\n");
        return -1;
    }
    avformat_write_header(ofmt_ctx,NULL);
    start_time = av_gettime();
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL Java_com_zhougang_camera2encode_H264Encoder_videoEncode
  (JNIEnv * env, jclass obj, jbyteArray yuv){
    LOGE("========================84==================");
    int enc_got_frame = 0;
    pFrameYUV = av_frame_alloc();
    uint8_t* out_buffer = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P,
        pCodecCtx->width,pCodecCtx->height));
    avpicture_fill((AVPicture*)pFrameYUV,out_buffer,AV_PIX_FMT_YUV420P,
        pCodecCtx->width,pCodecCtx->height);
    jbyte* in = (jbyte*)env->GetByteArrayElements(yuv,0);
    memcpy(pFrameYUV->data[0],in,y_length);
    for (int i = 0;i < uv_length;i++){
        *(pFrameYUV->data[2] + i) = *(in + y_length + uv_length);
        *(pFrameYUV->data[1] + i) = *(in + y_length);
    }
    pFrameYUV->format = AV_PIX_FMT_YUV420P;
    pFrameYUV->width = yuv_width;
    pFrameYUV->height = yuv_height;
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);
    if (avcodec_encode_video2(pCodecCtx,&enc_pkt,pFrameYUV,&enc_got_frame) < 0){
        LOGE("Encode error!\n");
        return -1;
    };
    av_frame_free(&pFrameYUV);
    if (enc_got_frame == 1){
        LOGE("Encode frame_cnt = %d packet_size = %d",framecnt,enc_pkt.size);
        framecnt++;
        enc_pkt.stream_index = video_st->index;
        AVRational time_base = ofmt_ctx->streams[0]->time_base;
        AVRational r_framerate1 = {60, 2};
        AVRational time_base_q = {1, AV_TIME_BASE};
        int64_t calc_duration = (double)(AV_TIME_BASE) * (1 / av_q2d(r_framerate1));
        enc_pkt.pts = av_rescale_q(framecnt * calc_duration,time_base_q,time_base);
        enc_pkt.dts = enc_pkt.pts;
        enc_pkt.duration = av_rescale_q(calc_duration,time_base_q,time_base);
        enc_pkt.pos = -1;
        int64_t pts_time = av_rescale_q(enc_pkt.dts,time_base,time_base_q);
        int64_t now_time = av_gettime() - start_time;
        LOGE("pts %d now %d",pts_time,now_time);
        if (pts_time > now_time){
            av_usleep(pts_time - now_time);
        }
        int ret = av_interleaved_write_frame(ofmt_ctx,&enc_pkt);
        LOGE("ret %d ",ret);
        av_free_packet(&enc_pkt);
        LOGE("=====================129=====================");
    }
    LOGE("======================130=================");
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL Java_com_zhougang_camera2encode_H264Encoder_flush
  (JNIEnv * env, jclass obj){
    int got_frame;
    AVPacket enc_pkt;
    LOGE("=========================flush========================");
    if (!(ofmt_ctx->streams[0]->codec->codec->capabilities & AV_CODEC_CAP_DELAY)){
        return 0;
    }
    while (1){
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        if (avcodec_encode_video2(ofmt_ctx->streams[0]->codec,&enc_pkt,NULL,&got_frame) < 0){
            break;
        }
        if (!got_frame){
            break;
        }
        AVRational time_base = ofmt_ctx->streams[0]->time_base;
        AVRational r_framerate1 = {60, 2};
        AVRational time_base_q = {1, AV_TIME_BASE};
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_framerate1));
        enc_pkt.pts = av_rescale_q(framecnt * calc_duration,time_base_q,time_base);
        enc_pkt.dts = enc_pkt.pts;
        enc_pkt.duration = av_rescale_q(calc_duration,time_base_q,time_base);
        enc_pkt.pos = -1;
        framecnt++;
        ofmt_ctx->duration = enc_pkt.duration * framecnt;
        if (av_interleaved_write_frame(ofmt_ctx,&enc_pkt) < 0){
            break;
        }
    }
    av_write_trailer(ofmt_ctx);
    if (video_st){
        avcodec_close(video_st->codec);
    }
    avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    LOGE("=============================172======================");
    return 0;
}


