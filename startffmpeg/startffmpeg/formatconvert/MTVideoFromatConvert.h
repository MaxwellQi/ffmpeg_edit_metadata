//
//  MTVideoFromatConvert.hpp
//  MediaTools
//
//  Created by zhangqi on 20/9/2017.
//  Copyright © 2017 zhangqi. All rights reserved.
//

#ifndef MTVideoFromatConvert_h
#define MTVideoFromatConvert_h

#include <stdio.h>

#endif /* MTVideoFromatConvert_h */

#ifdef __cplusplus
extern "C"
{
#endif
    
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avstring.h"
#include "libavutil/error.h"
#include "libswscale/swscale.h"
#include <libavutil/dict.h>
#include <time.h>
#include <stdio.h>
#ifdef __cplusplus
}
#endif

class MTVideoFromatConvert
{
public:
    static MTVideoFromatConvert * getInstance();
    MTVideoFromatConvert();
    ~MTVideoFromatConvert();
    int decoder();
    void setInputFilePath(const char* inputFile);
    
    // input file
    const  char *inputFilePath;
    
    // out put file
    const char  *output_file_h264;
    const char  *output_file_yuv;
private:
    static MTVideoFromatConvert *m_instance;
    
    // about ffmpeg
    AVFormatContext *pFormatCtx;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrame,*pFrameYUV;
    uint8_t         *out_buffer;
    AVPacket        *packet;
    int             y_size;
    int             got_picture;
    struct SwsContext *img_convert_ctx;
    int             frame_cnt;
    int             videoindex;
    
    // input and output file path
   
    char *outputFilePath;
    
};
