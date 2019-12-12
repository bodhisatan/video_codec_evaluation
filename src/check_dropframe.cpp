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
	cmdline::parser cmdPara = checkDropFrameCmdLine(argc, argv);
	cmdPara.parse_check(argc, argv);

	std::string refVideo = cmdPara.get<std::string>("refVideo");
	std::string mainVideo = cmdPara.get<std::string>("mainVideo");
	
	// 解析配置文件.
    YAML::Node conf = initPsnrConf();
    if (!conf["psnr"] || !conf["psnr"]["ocrSrcDir"] || !conf["psnr"]["resDir"]) {
        std::cout << "解析psnr.yaml文件失败!" << std::endl;
        return 0;
    }
    std::string ocrSrcDir = conf["psnr"]["ocrSrcDir"].as<std::string>();
    std::string resDir = conf["psnr"]["resDir"].as<std::string>();

    // 获取原视频旋转角度
    EVideoType t   = getRotateAngle(refVideo);

    #ifdef DEBUG
    std::cout << "原视频的EVideoType为: " << t << std::endl;
    #endif

    // 获取分辨率信息和帧数信息
    cv::Size r1 = GetVideoResolution(refVideo, t);
    
    #ifdef DEBUG
    std::cout << "原视频的分辨率为: " << r1.width << " x " << r1.height << std::endl;
    #endif

    int oriWidth     = r1.width;
	int oriHeight    = r1.height;
	int frameNumber  = GetVideoFrameNumber(refVideo);
	
	std::string file_name = mainVideo.substr(mainVideo.find_last_of('/') + 1);
    std::string file_res  = resDir + "/" + 
                            file_name.substr(0, file_name.find_last_of('.')) + ".dfi";

    std::vector<int> v;
    v.clear();

	std::cout << "开始丢帧检测" << std::endl;
    // 清空目录
    DeleteFiles(ocrSrcDir);
    // 获取帧号图像，存储在dir目录下
    GetFrameLabel(mainVideo, oriWidth, oriHeight, t, ocrSrcDir);
    // ocr检测, 丢帧信息存放在v中
    CheckFrameDrop(ocrSrcDir, frameNumber, v);

    std::ofstream dfi_file(file_res, std::ios::out);
    int j = 0;
    for (auto &i : v) {
    	dfi_file << i;
    	if ((++j) % 30 == 0) {
    		dfi_file << std::endl;
    	} else {
    		dfi_file << " ";
    	}
    }
    dfi_file.close();
	
	std::cout << "丢帧信息已经写入文件：" << file_res << std::endl;

	return 0;
}

