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

int MTVideoFromatConvert::addVideoMetadata(const char* filepath,const char* metadata)
{
    if (filepath == NULL || strlen(filepath) <= 4) {
        printf("filepath error..\n");
        return -1;
    }
    if (metadata == NULL || strlen(metadata) <= 0) {
        printf("metadata info is error..\n");
        return -1;
    }
    
    char outputfilepath[1024];
    strcpy(outputfilepath, filepath);
    size_t length = strlen(filepath);
    
    /* filepath=2017-09-17.mp4   outputpath=2017-09-17_tvu.mp4(is a temp file)  */
    
    outputfilepath[length-4] = '_';
    outputfilepath[length-3] = 't';
    outputfilepath[length-2] = 'v';
    outputfilepath[length-1] = 'u';
    outputfilepath[length] = '.';
    outputfilepath[length+1] = 'm';
    outputfilepath[length+2] = 'p';
    outputfilepath[length+3] = '4';
    
    
    
    av_register_all();
    avformat_network_init();
    
    AVFormatContext * outputContext = avformat_alloc_context();
    AVOutputFormat * outputFormat = av_guess_format(NULL, outputfilepath, NULL);
    if(outputFormat == NULL)
    {
        printf("av_guess_format error..\n");
        return -1;
    }
    outputContext->oformat = outputFormat;
    av_dict_set(&outputContext->metadata, "comment", metadata, 0);
    av_strlcpy(outputContext->filename, outputfilepath, sizeof(outputContext->filename));
    outputContext->nb_streams = 0;
    
    AVStream * outvideostream =  avformat_new_stream(outputContext, NULL);
    AVStream * outaudiostream = avformat_new_stream(outputContext, NULL);
    
    AVFormatContext * pFormatCtx = NULL;
    int ret = avformat_open_input(&pFormatCtx,filepath,NULL,NULL);
    if(ret < 0)
    {
        printf("open input file failed..\n");
        return -1;
    }
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0)
    {
        av_dump_format(pFormatCtx, 0, pFormatCtx->filename, 0);
        avformat_close_input(&pFormatCtx);
        printf("find stream failed..\n");
        return -2;
    }
    int video_stream_index = -1;
    int audio_stream_index = -1;
    for(int i =0 ;i<(pFormatCtx->nb_streams) ;i++)
    {
        AVStream * stream = pFormatCtx->streams[i];
        AVCodecContext * pCodecCtx = stream->codec;
        AVCodec  *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec == NULL)
        {
            printf("find decoder failed..\n");
            return -1;
        }
        AVMediaType  type = pCodecCtx->codec_type;
        if(type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            
            AVCodecContext *c = outvideostream->codec;
            outputContext->video_codec_id = stream->codec->codec_id;
            outvideostream->start_time = stream->start_time;
            outvideostream->time_base.den = stream->time_base.den;
            outvideostream->time_base.num = stream->time_base.num;
            c->width = stream->codec->width;
            c->height = stream->codec->height;
            c->bits_per_raw_sample = stream->codec->bits_per_raw_sample;
            c->chroma_sample_location = stream->codec->chroma_sample_location;
            c->codec_id = stream->codec->codec_id;
            c->codec_type = stream->codec->codec_type;
            
            c->bit_rate = stream->codec->bit_rate;
            c->rc_max_rate    = stream->codec->rc_max_rate;
            c->rc_buffer_size = stream->codec->rc_buffer_size;
            
            uint64_t extra_size = (uint64_t)stream->codec->extradata_size;
            c->extradata = (uint8_t *)av_mallocz((int)extra_size);
            memcpy(c->extradata, stream->codec->extradata, stream->codec->extradata_size);
            c->extradata_size= stream->codec->extradata_size;
            
            
            c->pix_fmt = stream->codec->pix_fmt;
            c->has_b_frames = stream->codec->has_b_frames;
            c->time_base.den = stream->time_base.den;
            c->time_base.num = stream->time_base.num;
            c->gop_size = stream->codec->gop_size;
            outputFormat->video_codec = stream->codec->codec_id;
            c->flags |= CODEC_FLAG_GLOBAL_HEADER;
            printf("set output stream succ..\n");
        }
        
        if(type == AVMEDIA_TYPE_AUDIO)
        {
            audio_stream_index = i;
            AVCodecContext *c = outaudiostream->codec;
            outputContext->audio_codec_id = stream->codec->codec_id;
            outaudiostream->start_time = stream->start_time;
            outaudiostream->time_base.den = stream->time_base.den;
            outaudiostream->time_base.num = stream->time_base.num;
            c->bits_per_raw_sample = stream->codec->bits_per_raw_sample;
            c->chroma_sample_location = stream->codec->chroma_sample_location;
            c->codec_id = stream->codec->codec_id;
            c->codec_type = stream->codec->codec_type;
            
            c->bit_rate = stream->codec->bit_rate;
            c->rc_max_rate    = stream->codec->rc_max_rate;
            c->rc_buffer_size = stream->codec->rc_buffer_size;
            
            c->channel_layout =  stream->codec->channel_layout;
            c->sample_rate = stream->codec->sample_rate;
            c->channels = stream->codec->channels;
            c->frame_size = stream->codec->frame_size;
            c->block_align= stream->codec->block_align;
            /* put sample parameters */
            c->sample_fmt = stream->codec->sample_fmt;
            outputFormat->audio_codec = stream->codec->codec_id;
            
            uint64_t extra_size = (uint64_t)stream->codec->extradata_size;
            c->extradata = (uint8_t *)av_mallocz((int)extra_size);
            memcpy(c->extradata, stream->codec->extradata, stream->codec->extradata_size);
            c->extradata_size= stream->codec->extradata_size;
            c->flags |= CODEC_FLAG_GLOBAL_HEADER;
            printf("set audio stream succ..\n");
        }
    }
    
    outputContext->max_delay = (int)(0.7*AV_TIME_BASE);
    
    int err = -1;
    if ((err = avio_open(&outputContext->pb, outputfilepath, AVIO_FLAG_WRITE)) < 0)
    {
        avformat_close_input(&pFormatCtx);
        printf("open output file failed..\n");
        return -2;
    }
    
    if (avformat_write_header(outputContext,NULL) < 0) {
        av_free(outvideostream);
        av_free(outaudiostream);
        avformat_close_input(&pFormatCtx);
        av_free(outputContext);
        printf("av_write_header error..\n");
        return -2;
    }
    
    AVPacket packet;
    int64_t starttime = -1;
    bool foundKframe = false;
    bool foundAudio = false;
    
    for(;;)
    {
        av_init_packet(&packet);
        packet.data = NULL;
        packet.size = 0;
        int ret = av_read_frame(pFormatCtx,&packet);
        if(ret >= 0)
        {
            if(foundAudio == false && packet.stream_index == 1)
            {
                foundAudio = true;
            }
            
            if (foundKframe == false && packet.flags != 0 && packet.stream_index == 0 && foundAudio == true)
            {
                foundKframe = true;
                starttime = packet.dts;
            }
            
            if(foundKframe == true &&  foundAudio == true)
            {
                packet.dts = packet.dts - starttime;
                packet.pts = packet.dts;
                
                if(packet.stream_index == 1)
                {
                    packet.pts = packet.dts;
                    packet.duration = 1024;
                }
                else
                {
                    if(packet.pts != AV_NOPTS_VALUE)
                        packet.pts =  packet.dts;
                }
                
                
                av_write_frame(outputContext, &packet);
            }
            av_free_packet(&packet);
            
        }
        else
        {
            if (ret == AVERROR_EOF || avio_feof(pFormatCtx->pb))
                break;
        }
        
    }
    av_write_trailer(outputContext);
    av_free(outvideostream);
    av_free(outaudiostream);
    av_free(outputContext);
    avformat_close_input(&pFormatCtx);
    
    // file operation
    int res = remove(filepath);
    if (res == -1) {
        printf("delete inputfile failed..\n");
        return  -1;
    }else{
        printf("delete inputfile succ..\n");
    }
    if (!rename(outputfilepath, filepath)) {
        printf("rename outputfilepath succ..\n");
    }else{
        printf("rename outputfilepath failed..\n");
        return -1;
    }
    
    return 0;
}

const char* MTVideoFromatConvert::getVideoMetadata(const char* filepath)
{
    if (filepath == NULL || strlen(filepath) <= 4) {
        printf("filepath error..\n");
        return NULL;
    }
    av_register_all();
    AVFormatContext *pFormatCtx = NULL;
    int ret = avformat_open_input(&pFormatCtx, filepath, NULL, NULL);
    if (ret < 0) {
        printf("open input file failed..\n");
        return NULL;
    }
    
    AVDictionaryEntry *tag = av_dict_get(pFormatCtx->metadata, "comment", NULL, 0);
    if (!tag) {
        avformat_close_input(&pFormatCtx);
        printf("have not metadata info..\n");
        return NULL;
    }
    
    printf("succeed get metadata info: %s\n",tag->value);
    return tag->value;
}
