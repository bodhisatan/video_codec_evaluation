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
#include <opencv2/opencv.hpp>
#include <boost/lexical_cast.hpp>

#include "../frame_drop_detect.h"

static inline unsigned pow_2(unsigned base) {
    return base*base;
}

static inline double get_psnr(double mse, int max) {
    return 10.0 * log10(pow_2(max) / (mse));
}

// plane=0,1,2,3分别对应y,u,v,avg各分量，默认为y分量。
static double compute_images_mse(const unsigned char *main_data, const unsigned char *ref_data, 
    const int w, const int h, const int size, const int plane = 0) {
    
    int begin = 0;
    int end   = 0;
    int deno  = 0;

    if (plane == 0) {begin = 0; end = w * h; deno = w * h; } 
    if (plane == 1) {begin = w * h; end = (w * h) + ((w * h) >> 2); deno = (w * h) >> 2;}
    if (plane == 2) {begin = (w * h) + ((w * h) >> 2); end = size; deno = (w * h) >> 2;}
    if (plane == 3) {begin = 0; end = size; deno = size;}
    
    double mse = 0.0;

    for (int i = begin; i < end; ++i) {
        // std::cout << "(" << (int)main_data[i] << ", " << (int)ref_data[i] << ")" << std::endl;
        mse += pow_2((int)main_data[i] - (int)ref_data[i]);
    }


    return mse / (double) (deno);
}

// 归一化mse,将[min_mse, max_min]区间内的mse归一化到[0, normal]的level级别区间中。
// 例如将[0, 10]中的5归一化到[0, 100]的5级区间中，则1=>20， 2=>20，3=>40...
uchar mse_normal(int mse, int min_mse, int max_mse, int level, int normal) {
    int oriInterval = (max_mse - min_mse + 1) / (int)level;
    int normalInterval = (normal + 1) / (int)level;

    std::vector<int> oriList;
    std::vector<int> normalList;

    for (int i = 0; i <= level; ++i) {
        oriList.emplace_back(min_mse + oriInterval * i);
        normalList.emplace_back(normalInterval * i);
    }
    
    int res = 0;
    for (int i = 1; i <= level; ++i) {
        if (mse <= oriList[i]) {
            res = normalList[i - 1];
            break;
        } 
    }

    return (uchar)res;
}

// plane=0,1,2分别对应y,u,v各分量，默认为y分量。
static void compute_mse_image(const unsigned char *main_data, const unsigned char *ref_data, 
    const int w, const int h, const int size, const int plane, int frame_no, cv::VideoWriter &writer) {
    
    int begin   = 0;
    int end     = 0;
    
    int min_mse = 0;
    int max_mse = 0;

    int w1 = 0;
    int h1 = 0;

    if (plane == 0) {begin = 0; end = w * h; w1 = w; h1 = h;} 
    if (plane == 1) {begin = w * h; end = (w * h) + ((w * h) >> 2); w1 = w >> 1; h1 = h >> 1;}
    if (plane == 2) {begin = (w * h) + ((w * h) >> 2); end = size; w1 = w >> 1; h1 = h >> 1;}
    
    int mse = 0;

    min_mse = max_mse = pow_2((int)main_data[begin] - (int)ref_data[begin]);

    for (int i = begin + 1; i < end; ++i) {
        mse = pow_2((int)main_data[i] - (int)ref_data[i]);
        if (mse < min_mse) {min_mse = mse;}
        if (mse > max_mse) {max_mse = mse;}
    }
    
    if (max_mse > 255) {max_mse = 255;}

    cv::Mat greyFrame = cv::Mat(h1, w1, CV_8UC1, cv::Scalar(255));

    for (int i = begin; i < end; ++i) {
        mse = pow_2((int)main_data[i] - (int)ref_data[i]);
        uchar mse_nor = mse_normal(mse, min_mse, max_mse, 8, 255);
        int j = i - begin;
        greyFrame.at<uchar>((int)(j / w1), (int)(j % w1)) = mse_nor;
    }

    writer.write(greyFrame);
}

int main(int argc, char *argv[]) {
    cv::Size r1 = GetVideoResolution("videoDB/t12.mp4", CAMERA_FACING_BACK);
    cv::Size r2 = GetVideoResolution("VideoDB/t13.mp4");
    std::cout << "(" << r1.width << "," << r1.height << ")" << std::endl;
    std::cout << "(" << r2.width << "," << r2.height << ")" << std::endl;

    std::string file1  = "videoDB/t12.yuv";
	std::string file2   = "videoDB/t13.yuv";
	EPixFormat  format = YUV420;

    int oriWidth     = 720;
	int oriHeight    = 1280;
	int frameNumber  = 173;
	
    // 丢帧信息，丢前2帧.
    std::vector<int> v;
    for (int i = 0; i < 2; ++i) {
        v.emplace_back(0);
    }

    for (int i = 2; i < frameNumber; ++i) {
        v.emplace_back(1);
    }

    for (auto &i : v) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    // 每一帧的yuv数据所占的字节大小
    int frame_bytes      = (oriWidth * oriHeight * 3) >> 1;

    FILE          *f1, *f2;
    unsigned char *b1, *b2;

    // 当前处理的帧.
    int current_frame = -1;

    // 打开文件
    if ((f1 = fopen(file1.c_str(), "rb")) == 0 || (f2 = fopen(file2.c_str(), "rb")) == 0) {
        std::cout << "open file1 || file2 failed!" << std::endl;
        if (f1) {
            fclose(f1);
        }
    }

    // 如果申请内存失败则返回。
    if (!(b1 = (unsigned char *)malloc(frame_bytes)) || !(b2 = (unsigned char *)malloc(frame_bytes))) {
        std::cout << "malloc memory for b1 || b2 failed!" << std::endl;
        if (b1) {
            free(b1);
        }

        return -2;
    }

    cv::VideoWriter writer_y = cv::VideoWriter("y.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(720, 1280), false);
    cv::VideoWriter writer_u = cv::VideoWriter("u.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(360, 640), false);
    cv::VideoWriter writer_v = cv::VideoWriter("v.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(360, 640), false);

    // 打开文件开始按每一帧读取数据然后计算.
    while (1) {
        // 处理第current_frame帧，从0开始.
        ++current_frame;
        std::memset(b1, 0, frame_bytes);
        std::memset(b2, 0, frame_bytes);

        // 如果存在丢帧，则跳过
        if (v[current_frame] == 0) {
            if (1 != fread(b1, frame_bytes, 1, f1) || 1 != fread(b2, frame_bytes, 1, f2)) {
                std::cout << "at least one file pointer get the end!" << std::endl;
                break; 
            } else {
                std::cout << "n:" << current_frame + 1
                          << std::setiosflags(std::ios::fixed) << std::setprecision(2)
                          << " mse_avg:"  << 0  
                          << " mse_y:"    << 0 
                          << " mse_u:"    << 0 
                          << " mse_v:"    << 0
                          << " psnr_avg:" << 0
                          << " psnr_y:"   << 0
                          << " psnr_u:"   << 0
                          << " psnr_v:"   << 0  
                          << std::endl;
                continue;
            }
        }
        
        if (1 != fread(b1, frame_bytes, 1, f1) || 1 != fread(b2, frame_bytes, 1, f2)) {
            std::cout << "at least one file pointer get the end!" << std::endl;
            break; 
        }

        // 计算y分量psnr
        double mse_y   = compute_images_mse(b1, b2, oriWidth, oriHeight, frame_bytes);
        double mse_u   = compute_images_mse(b1, b2, oriWidth, oriHeight, frame_bytes, 1);
        double mse_v   = compute_images_mse(b1, b2, oriWidth, oriHeight, frame_bytes, 2);
        double mse_avg = compute_images_mse(b1, b2, oriWidth, oriHeight, frame_bytes, 3);

        std::cout << "n:" << current_frame + 1
                  << std::setiosflags(std::ios::fixed) << std::setprecision(2)
                  << " mse_avg:"  << mse_avg  
                  << " mse_y:"    << mse_y 
                  << " mse_u:"    << mse_u 
                  << " mse_v:"    << mse_v
                  << " psnr_avg:" << get_psnr(mse_avg, 255)
                  << " psnr_y:"   << get_psnr(mse_y, 255)
                  << " psnr_u:"   << get_psnr(mse_u, 255)
                  << " psnr_v:"   << get_psnr(mse_v, 255)  
                  << std::endl;

        compute_mse_image(b1, b2, oriWidth, oriHeight, frame_bytes, 0, current_frame, writer_y);
        compute_mse_image(b1, b2, oriWidth, oriHeight, frame_bytes, 1, current_frame, writer_u);
        compute_mse_image(b1, b2, oriWidth, oriHeight, frame_bytes, 2, current_frame, writer_v);
    }

    // 关闭资源
    fclose(f1);
    fclose(f2);
    free(b1);
    free(b2);

    std::cout << "psnr" << std::endl;
	return 0;
}

