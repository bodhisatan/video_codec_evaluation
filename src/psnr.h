#ifndef PSNR_H_
#define PSNR_H_

#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <cstring>
#include <cmath>
#include <complex>
#include <iomanip>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <boost/lexical_cast.hpp>

#include "cmdlineutils.h"
#include "conf.h"
#include "frame_drop_detect.h"
#include "matrixutils.h"

extern "C" {
  #include "libavcodec/avcodec.h"
  #include "libavformat/avformat.h"
  #include "libavdevice/avdevice.h"
  #include "libavutil/imgutils.h"
}

#define CONFIG_AVDEVICE 1

/**
 * 计算并返回base * base.
 */
unsigned pow2(unsigned base);

/**
 * 计算并返回psnr = log10((max * max) / mse)
 */
double getPsnr(double mse, int max);

/**
 * 计算两幅图像的MSE.
 * @param main_data: 含有噪声的图像.
 * @param ref_data: 参考图像.
 * @param w: 图像的宽度.
 * @param h: 图像的高度.
 * @param size: 每帧图像的字节数.
 * @param plane {0, 1, 2, 3}: 需要计算的图像的通道,
 *        plane=0,1,2,3分别对应y,u,v,avg各分量，默认为y分量.
 */
double computeImagesMse(const unsigned char *main_data, 
	                      const unsigned char *ref_data,
                        const EPixFormat format,  
                        const int w, 
                        const int h,   
                        const int plane = 0);

/**
 * 计算两幅图像的每个像素点的误差，并且将误差以灰度图的格式压制成视频文件.
 * @param main_data: 含有噪声的图像.
 * @param ref_data: 参考图像.
 * @param w: 图像的宽度.
 * @param h: 图像的高度.
 * @param size: 每帧图像的字节数.
 * @param plane {0, 1, 2}: 需要计算的图像的通道,
 *        plane=0,1,2分别对应y,u,v各分量，默认为y分量.
 * @param frame_no: 当前计算的图像伪视频的第几帧.
 * @param writer: 将像素点的误差写入writer对应的视频文件.
 */
void computeMseImage(const unsigned char *main_data, 
	                   const unsigned char *ref_data, 
                     const EPixFormat format, 
    				         const int w, 
    				         const int h, 
    				         const int plane, 
    				         int frame_no, 
    				         cv::VideoWriter &writer);

/**
 * 计算两幅图像的以step*step为块大小的各子块的psnr，并将对应子块的psnr以灰度图的格式压制成视频文件.
 * @param main_data: 含有噪声的图像.
 * @param ref_data: 参考图像.
 * @param w: 图像的宽度.
 * @param h: 图像的高度.
 * @param step: 计算子块的大小，目前仅支持方形块，也就是块的大小为step*step.
 * @param size: 每帧图像的字节数.
 * @param plane {0, 1, 2}: 需要计算的图像的通道,
 *        plane=0,1,2分别对应y,u,v各分量，默认为y分量.
 * @param frame_no: 当前计算的图像伪视频的第几帧.
 * @param writer: 将像素点的误差写入writer对应的视频文件.
 */
void computeBlockPsnrImage(const unsigned char *main_data, 
                           const unsigned char *ref_data, 
                           const EPixFormat format, 
                           const int w, 
                           const int h, 
                           const int step, 
                           const int plane, 
                           int frame_no, 
                           cv::VideoWriter &writer);

/**
 * 归一化mse. 
 * 将[min_mse, max_min]区间内的mse归一化到[0, normal]的level级别区间中。
 * 例如将[0, 10]中的5归一化到[0, 100]的5级区间中，则1=>20， 2=>20，3=>40...
 */
uchar mseNormal(int mse, int min_mse, int max_mse, int level, int normal);

/**
 * 计算yuv格式的main_video和ref_video之间的psnr并将其可视化.
 * @param main_video: 含有噪声的视频数据, yuv格式.
 * @param ref_video: 参数视频, yuv格式.
 * @param width: 视频宽度.
 * @param height: 视频高度.
 * @param frame_count: 视频的帧数.
 * @param frame_drop_info: 视频main_video相对于ref_video的丢帧信息. 
 * @param step: 按照step*step的块计算psnr.
 */
bool psnrAndVisualize(const std::string &main_video, 
                      const std::string &ref_video,
                      const EPixFormat format, 
                      const int frame_count,
                      const int width,
                      const int height,
                      const std::vector<int> &frame_drop_info,
                      const int step = 1);

/**
 * 将MP4文件转为对应的yuv原始数据文件, 
 * 并存储在conf/psnr.yaml中配置的conf["psnr"]["resDir"]目录中。
 * @param mp4: 待转为yuv格式的MP4文件的路径.
 * @param yuv: yuv文件的存放地址.
 * @return: 成功返回true，失败返回false.
 * @TODO: 目前没有对t参数做处理，因此只能处理没有旋转的视频的mp4转yuv.
 */
bool mp42yuv(const std::string &mp4, std::string &yuv);

/**
 * 获取视频的旋转角度.
 * @param avStream: 指定视频流.
 */
int getRotateAngle(const AVStream *avStream);

/**
 * 对src 旋转90度，然后存储在dst中.
 */
void Rotate90(const AVFrame* src, AVFrame* dst);

/**
 * 对src 旋转270度，然后存储在dst中.
 */
void Rotate270(const AVFrame* src, AVFrame* dst);

#endif // PSNR_H_