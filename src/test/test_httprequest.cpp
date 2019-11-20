#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>

#include "../ocr.h"
#include "../cmdlineutils.h"
#include "../matrixutils.h"
#include "../frame_label.h"

int main(int argc, char *argv[]) {
	std::string file = "videoDB/t33.mp4";
	int oriWidth     = 720;
	int oriHeight    = 1280;
	int oriType      = 1;
	std::string dir  = "psnr/data";
	
	// 获取帧号图像
	GetFrameLabel(file, oriWidth, oriHeight, EVideoType(oriType), dir);

	// ocr检测
    std::string image = "psnr/data/3.png";
    int res = GetOcrNumber(image);
    std::cout <<  res << std::endl;
    
    // 计算丢帧
    // ...

    // 计算psnr
    // ...
    
    std::cout << "s" << std::endl;
    std::vector<std::string> files;
    GetFiles("psnr/data", files);
    std::cout << "files size: " << files.size() << std::endl;
    for (auto &i : files) {
    	std::cout << "psnr/data/" << i << std::endl;
    }

	return 0;
}

