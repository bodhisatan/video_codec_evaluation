#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <unistd.h>
#include <stdio.h>

#include "../ocr.h"
#include "../cmdlineutils.h"
#include "../matrixutils.h"
#include "../frame_label.h"

int main(int argc, char *argv[]) {
    std::string file1= "videoDB/t31.mp4";
	std::string file = "videoDB/t33.mp4";
	int oriWidth     = 720;
	int oriHeight    = 1280;
	int oriType      = 1;
	std::string dir  = "psnr/data";
	
    // 获取原始视频的帧数
    int frameNumber = GetVideoFrameNumber(file1);
    std::cout << "frameNumber: " << frameNumber << std::endl;

    // 清空目录
    DeleteFiles(dir);

	// 获取帧号图像
	GetFrameLabel(file, oriWidth, oriHeight, EVideoType(oriType), dir);

	// ocr检测
    std::vector<std::string> files;
    GetFiles(dir, files);
    std::cout << "files size: " << files.size() << std::endl;
    std::cout << "start process the ocr detect" << std::endl;

    int ii = 0;
    for (auto &i : files) {
        ++ii;
        std::string f = dir + "/" + i;
        int res = GetOcrNumber(f);

        std::cout << "...[" << ii << "/" 
                  << frameNumber << "] process the file: " 
                  << f << ", the res is: "<< res << std::setw(16); 
        fflush(stdout);
        usleep(100000);
        std::cout << "\r\033[k";
    }
    std::cout << std::endl;
    std::cout << "process the ocr detect stop" << std::endl;

    // 计算丢帧
    // ...

    // 计算psnr
    // ...
    
	return 0;
}

