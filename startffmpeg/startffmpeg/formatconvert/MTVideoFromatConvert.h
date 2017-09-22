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
    
    
    
    /// \brief add metadata to a video
    /// \param filepath the video's path
    /// \param metadata metadata info
    /// \return return 0 is success otherwise is failed
    int addVideoMetadata(const char* filepath,const char* metadata);
    const char* getVideoMetadata(const char* filepath);
    int test();
private:
    static MTVideoFromatConvert *m_instance;
    
};
