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

#include "../frame_label.h"

static inline unsigned pow_2(unsigned base) {
    return base*base;
}

static inline double get_psnr(double mse, int max) {
    return 10.0 * log10(pow_2(max) / (mse));
}

// plane=0,1,2分别对应y,u,v各分量，默认为y分量。
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

int main(int argc, char *argv[]) {
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
    }

    // 关闭资源
    fclose(f1);
    fclose(f2);
    free(b1);
    free(b2);

    std::cout << "psnr" << std::endl;
	return 0;
}

