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
    
    printf("decoder info:    time_base.num=%d,time_base.den=%d,bit_rate=%d,gop_size=%d,qmin=%d,qmax=%d,qcompress=%f\n",pCodecCtx->time_base.num,pCodecCtx->time_base.den,pCodecCtx->bit_rate,pCodecCtx->gop_size,pCodecCtx->qmin,pCodecCtx->qmax,pCodecCtx->qcompress);
    
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


int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index);
int MTVideoFromatConvert::encoder()
{
   
    
    FILE *in_file = fopen(inputFilePath, "rb");
    if (!in_file) {
        printf("could not open input file..\n");
        return -1;
    }
    avcodec_register_all();
    av_register_all();
    
    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, outputFilePath);
    fmt = pFormatCtx->oformat;
    if (avio_open(&pFormatCtx->pb, outputFilePath, AVIO_FLAG_READ_WRITE)) {
        printf("output file open fail!\n");
        return -1;
    }
    
    video_st = avformat_new_stream(pFormatCtx, 0);
    if (video_st == NULL) {
        printf("failed allocating output stream.\n");
        return -1;
    }
    video_st->time_base.num = 1;
    video_st->time_base.den = 30;
    
    pCodecCtx = video_st->codec;
    pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->width = 1920;
    pCodecCtx->height = 1080;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 30;
    pCodecCtx->bit_rate = 5000000;
    pCodecCtx->gop_size = 12;

    if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        pCodecCtx->qmin = 2;
        pCodecCtx->qmax = 31;
        pCodecCtx->qcompress = 0.5;
    }
    // what's meain ?
    if (pCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        pCodecCtx->max_b_frames = 2;
    }
    if (pCodecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        pCodecCtx->mb_decision = 2;
    }
    
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        printf("have not find encoder..\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("open encoder failed..\n");
        return -1;
    }
    
    av_dump_format(pFormatCtx,0,outputFilePath,1);
    
    // init farme
    picture = av_frame_alloc();
    picture->width = pCodecCtx->width;
    picture->height=pCodecCtx->height;
    picture->format=pCodecCtx->pix_fmt;
    int size = avpicture_get_size(pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height);
    picture_buf = (uint8_t*)av_malloc(size);
    avpicture_fill((AVPicture*)picture,picture_buf,pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height);
    
    // add metadata info
    av_dict_set(&pFormatCtx->metadata, "comment", "<uploader>qizhang</uploader>,<city>shanghai</city>,<company>TVUNetworks</company>", 0);
    
    avformat_write_header(pFormatCtx, NULL);
    
    // create encoded frame
    AVPacket pkt;
    int y_size = pCodecCtx->width*pCodecCtx->height;
    av_new_packet(&pkt, size * 3);
    
    int i = 0;
    while (1) {
        
        //读入YUV
        if(fread(picture_buf,1,y_size*3/2,in_file)<0)
        {
            printf("read file fail!");
            break;
            return -1;
        }else if(feof(in_file))
            break;
        
        picture->data[0] = picture_buf;
        picture->data[1] = picture_buf+y_size;
        picture->data[2] = picture_buf+y_size*5/4;
        
        picture->pts = i;
        i++;
        
        //encoder
        int ret = avcodec_encode_video2(pCodecCtx,&pkt,picture,&got_picture);
        if(ret<0)
        {
            printf("encoder fail!\n");
            return -1;
        }
        
        if(got_picture == 1)
        {
            printf("encoder success!\n");
            // parpare packet for muxing
            pkt.stream_index = video_st->index;
            av_packet_rescale_ts(&pkt, pCodecCtx->time_base, video_st->time_base);
            pkt.pos = -1;
            ret = av_interleaved_write_frame(pFormatCtx,&pkt);
            av_free_packet(&pkt);
        }
    }
    
    // flush encoder
    int ret = flush_encoder(pFormatCtx,0);
    if(ret < 0)
    {
        printf("flushing encoder failed!\n");
        return -1;
    }
    
     av_write_trailer(pFormatCtx);
    

    //free memory
    if(video_st)
    {
        avcodec_close(video_st->codec);
        av_free(picture);
        av_free(picture_buf);
    }
    if(pFormatCtx)
    {
        avio_close(pFormatCtx->pb);
        avformat_free_context(pFormatCtx);
    }
    
    fclose(in_file);
    
    return 0;
}

int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index)
{
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
          CODEC_CAP_DELAY))
        return 0;
    while (1) {
        printf("Flushing stream #%u encoder\n", stream_index);
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                     NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame)
        {ret=0;break;}
        printf("success encoder 1 frame\n");
        
        // parpare packet for muxing
        enc_pkt.stream_index = stream_index;
        av_packet_rescale_ts(&enc_pkt,
                             fmt_ctx->streams[stream_index]->codec->time_base,
                             fmt_ctx->streams[stream_index]->time_base);
        ret = av_interleaved_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}


