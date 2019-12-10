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
    std::string f1 = "videoDB/t42.mp4";
    std::string f2 = "videoDB/t43.mp4";
    EVideoType t   = CAMERA_OTHERS;
    bool drop_frame_detect = false;

    YAML::Node conf = initPsnrConf();
    if (!conf["psnr"] || !conf["psnr"]["srcDir"] || !conf["psnr"]["resDir"]) {
        std::cout << "解析psnr.yaml文件失败!" << std::endl;
        return 0;
    }

    std::string srcDir = conf["psnr"]["srcDir"].as<std::string>();
    std::string resDir = conf["psnr"]["resDir"].as<std::string>();

    // 获取分辨率信息和帧数信息
    cv::Size r1 = GetVideoResolution(f1, t);
    cv::Size r2 = GetVideoResolution(f2);

    std::cout << "(" << r1.width << "," << r1.height << ")" << std::endl;
    std::cout << "(" << r2.width << "," << r2.height << ")" << std::endl;

    int oriWidth     = r1.width;
	int oriHeight    = r1.height;
	int frameNumber  = GetVideoFrameNumber(f1);
	
    std::vector<int> v;
    v.clear();

    if (drop_frame_detect) {
        // 清空目录
        DeleteFiles(srcDir);
        // 获取帧号图像，存储在dir目录下
        GetFrameLabel(f2, oriWidth, oriHeight, t, srcDir);
        // ocr检测, 丢帧信息存放在v中
        CheckFrameDrop(srcDir, frameNumber, v);

        std::cout << "丢帧信息如下:" << std::endl;
        for (auto &i : v) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    } else {
        for (int i = 0; i < frameNumber; ++i) {
            v.emplace_back(1);
        }    
    }
    
    std::string yuv1 = "";
    std::string yuv2 = "";
    if (!mp42yuv(f1, CAMERA_OTHERS, yuv1) || !mp42yuv(f2, CAMERA_OTHERS, yuv2)) {
        std::cout << "生成yuv文件错误." << std::endl;
        return 0;
    }

    psnrAndVisualize(yuv1, yuv2, YUV420, frameNumber, oriWidth, oriHeight, v, 1);

    std::cout << std::endl;
    std::cout << "计算psnr完毕" << std::endl;
	return 0;
}

