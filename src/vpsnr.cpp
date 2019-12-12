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

#include "frame_drop_detect.h"
#include "matrixutils.h"
#include "psnr.h"

int main(int argc, char *argv[]) {
	// 解析命令行参数.
    cmdline::parser cmdPara = vpsnrCmdLine(argc, argv);
	cmdPara.parse_check(argc, argv);

	std::string f1         = cmdPara.get<std::string>("refVideo");
	std::string f2         = cmdPara.get<std::string>("mainVideo");
    int  blockSize         = cmdPara.get<int>("blockSize");
	bool drop_frame_detect = cmdPara.get<bool>("dropFrameDetect");
    std::string dpi        = cmdPara.get<std::string>("dropFrameInfoFile");

	// 解析配置文件.
    YAML::Node conf = initPsnrConf();
    if (!conf["psnr"] || !conf["psnr"]["ocrSrcDir"] || !conf["psnr"]["resDir"]) {
        std::cout << "解析psnr.yaml文件失败!" << std::endl;
        return 0;
    }
    std::string ocrSrcDir = conf["psnr"]["ocrSrcDir"].as<std::string>();
    std::string resDir = conf["psnr"]["resDir"].as<std::string>();

    // 获取原视频旋转角度
    EVideoType t   = getRotateAngle(f1);

    #ifdef DEBUG
    std::cout << "原视频的EVideoType为: " << t << std::endl;
    #endif

    // 获取分辨率信息和帧数信息
    cv::Size r1 = GetVideoResolution(f1, t);
    
    #ifdef DEBUG
    std::cout << "原视频的分辨率为: " << r1.width << " x " << r1.height << std::endl;
    #endif

    int oriWidth     = r1.width;
	int oriHeight    = r1.height;
	int frameNumber  = GetVideoFrameNumber(f1);
	
    std::vector<int> v;
    v.clear();

    if (drop_frame_detect) {
        if (dpi == "") { // 未指定丢帧信息文件，则自动检测.
            std::cout << "开始丢帧检测" << std::endl;
            // 清空目录
            DeleteFiles(ocrSrcDir);
            // 获取帧号图像，存储在dir目录下
            GetFrameLabel(f2, oriWidth, oriHeight, t, ocrSrcDir);
            // ocr检测, 丢帧信息存放在v中
            CheckFrameDrop(ocrSrcDir, frameNumber, v);

            #ifdef DEBUG
            std::cout << "丢帧信息如下:" << std::endl;
            for (auto &i : v) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
            #endif
        } else { // 否则，读取丢帧信息文件中的丢帧信息.
            getDPIFromFile(dpi, v);
            #ifdef DEBUG
            std::cout << "丢帧信息如下:" << std::endl;
            for (auto &i : v) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
            #endif
        }
    } else { // 无需丢帧检测.
        for (int i = 0; i < frameNumber; ++i) {
            v.emplace_back(1);
        }    
    }
    
    if (v.size() != frameNumber) {
        std::cout << "丢帧检测失败，请检查丢帧结果" << std::endl;
    }

    std::string yuv1 = "";
    std::string yuv2 = "";
    if (!mp42yuv(f1, yuv1) || !mp42yuv(f2, yuv2)) {
        std::cout << "生成yuv文件错误." << std::endl;
        return 0;
    }

    psnrAndVisualize(yuv2, yuv1, YUV420, frameNumber, oriWidth, oriHeight, v, blockSize);
    std::cout << "计算psnr完毕" << std::endl;

	return 0;
}