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
#include "../psnr.h"

int main(int argc, char *argv[]) {
    cv::Size r1 = GetVideoResolution("videoDB/t12.mp4", CAMERA_FACING_BACK);
    cv::Size r2 = GetVideoResolution("VideoDB/t13.mp4");
    std::cout << "(" << r1.width << "," << r1.height << ")" << std::endl;
    std::cout << "(" << r2.width << "," << r2.height << ")" << std::endl;

    std::string file1  = "videoDB/t12.yuv";
	std::string file2   = "videoDB/t13.yuv";

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

    psnrAndVisualize(file1, file2, YUV420, frameNumber, oriWidth, oriHeight, v);

    std::cout << std::endl;
    std::cout << "计算psnr完毕" << std::endl;
	return 0;
}

