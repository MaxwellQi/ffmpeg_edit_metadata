//
//  MTVideoFromatConvert.cpp
//  MediaTools
//
//  Created by zhangqi on 20/9/2017.
//  Copyright © 2017 zhangqi. All rights reserved.
//

#include "MTVideoFromatConvert.h"


MTVideoFromatConvert::MTVideoFromatConvert()
{
    
}

MTVideoFromatConvert::~MTVideoFromatConvert()
{
    
}

MTVideoFromatConvert * MTVideoFromatConvert::m_instance = NULL;
MTVideoFromatConvert * MTVideoFromatConvert::getInstance()
{
    if (m_instance == NULL) {
        m_instance = new MTVideoFromatConvert();
    }
    return m_instance;
}


int MTVideoFromatConvert::decoder()
{
    FILE *fp_yuv = fopen(output_file_yuv, "wb+");
    FILE *fp_h264 = fopen(output_file_h264, "wb+");
    
    
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    printf("inputFilePath:%s\n",inputFilePath);
    if (avformat_open_input(&pFormatCtx, inputFilePath, NULL, NULL) != 0) {
        printf("could not open input stream..\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("could not find stream info..\n");
        return -1;
    }
    videoindex = -1;
    int i;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    }
    if (videoindex == -1) {
        printf("could not find video stream..\n");
        return -1;
    }
    
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        printf("could not find codec..\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("could not open codec..\n");
        return -1;
    }
    
    
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    
    av_dump_format(pFormatCtx, 0, inputFilePath, 0);
    
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    
    frame_cnt = 0;

    while(av_read_frame(pFormatCtx, packet) >= 0)
    {
        
        // write h264 data
        fwrite(packet->data, 1, packet->size, fp_h264);
        
        
        
        
        if (packet->stream_index == videoindex) {
            
            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet); // decode one packet
            if (ret < 0) {
                printf("Deocde Error..\n");
                return -1;
            }
            if (got_picture) {
                sws_scale(img_convert_ctx,(const uint8_t* const*) pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
                printf("decoded frame index:%d\n",frame_cnt);
                
                
                // write yuv data
                int y_size = pCodecCtx->width*pCodecCtx->height;
                
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);
                fwrite(pFrameYUV->data[1], 1, y_size/4, fp_yuv);
                fwrite(pFrameYUV->data[2], 1, y_size/4, fp_yuv);
                
                frame_cnt++;
            }
        }
        av_free_packet(packet);
    }
    
    // flush decoder
    // fix: Flush Frames remained in codec
    int frame_cnt1 = 0;
    while (1) {
       int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0)
            break;
        if (!got_picture)
            break;
        sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                  pFrameYUV->data, pFrameYUV->linesize);
        
        int y_size = pCodecCtx->width*pCodecCtx->height;
        printf("Flush Decoder: %d\n", frame_cnt1);
        fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
        fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
        fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
        frame_cnt1++;
    }
    
    
    sws_freeContext(img_convert_ctx);
    
    //关闭文件以及释放内存
    fclose(fp_yuv);
    fclose(fp_h264);
    
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    
    
    return 0;
    
    
    
}
