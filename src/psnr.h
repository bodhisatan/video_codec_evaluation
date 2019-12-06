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
#endif // PSNR_H_