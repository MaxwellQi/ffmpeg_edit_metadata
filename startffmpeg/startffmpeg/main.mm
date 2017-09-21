//
//  main.m
//  startffmpeg
//
//  Created by zhangqi on 20/9/2017.
//  Copyright © 2017 zhangqi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MTVideoFromatConvert.h"


// /Users/qizhang/Desktop/IMG_2362.mp4


int main(int argc, const char * argv[]) {
    
    MTVideoFromatConvert *videoFormatConvert = MTVideoFromatConvert::getInstance();
    
    
    /* decoder */
    
//    videoFormatConvert->inputFilePath = "/Users/qizhang/Desktop/ffmpeg_video/IMG_2362.mp4";
//    
//    // output
//    videoFormatConvert->output_file_h264 = "/Users/qizhang/Desktop/ffmpeg_video/IMG_2362.h264";
//    videoFormatConvert->output_file_yuv = "/Users/qizhang/Desktop/ffmpeg_video/IMG_2362.yuv";
//    
//    videoFormatConvert->decoder();
    
    
    /* encoder */
    videoFormatConvert->inputFilePath = "/Users/qizhang/Desktop/ffmpeg_video/IMG_2362.yuv";
    videoFormatConvert->outputFilePath = "/Users/qizhang/Desktop/ffmpeg_video/result.mp4";
    videoFormatConvert->encoder();
    
    return 0;
}
