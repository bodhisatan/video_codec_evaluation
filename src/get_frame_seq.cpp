#include <string>
#include <fstream>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <yaml-cpp/yaml.h>
#include <opencv2/opencv.hpp>
#include <boost/lexical_cast.hpp>

#include "cmdline.h"
#include "cmdlineutils.h"

YAML::Node initConfigure() {
	return YAML::LoadFile("conf/conf.yaml");
}

cv::Rect getSubFrameRect(YAML::Node &conf, cv::Mat &frame, int oriWidth, int oriHeight) {
	cv::Rect rect(0, 0, 0, 0);

	if (!conf["originalVideo"] || 
		!conf["originalVideo"]["flagArea"]) {
        return rect;
    }

	// int oriWidth    = conf["originalVideo"]["resolution"]["width"].as<int>();
	// int oriHeight   = conf["originalVideo"]["resolution"]["height"].as<int>();

	int oriFlagX    = conf["originalVideo"]["flagArea"]["x"].as<int>();
	int oriFlagY    = conf["originalVideo"]["flagArea"]["y"].as<int>();
	int oriFlagW    = conf["originalVideo"]["flagArea"]["width"].as<int>();
	int oriFlagH    = conf["originalVideo"]["flagArea"]["height"].as<int>();

	int frameWidth  = frame.cols;
	int frameHeight = frame.rows;

	float ratioX      = oriWidth * 1.0 / frameWidth;
	float ratioY      = oriHeight * 1.0 / frameHeight;

	std::cout << "ratioX: " << ratioX << ", ratioY:" << ratioY << std::endl;
	
	rect = cv::Rect(int(oriFlagX / ratioX), 
		            int(oriFlagY / ratioY), 
		            int(oriFlagW / ratioX),
		            int(oriFlagH / ratioY));

	return rect;
}

int main(int argc, char *argv[]) {
	cv::VideoCapture cap;
	std::string wndTitle;

	cmdline::parser cmdPara = initCmdLine(argc, argv);
	cmdPara.parse_check(argc, argv);

	std::string file = cmdPara.get<std::string>("input");
	int oriWidth     = cmdPara.get<int>("originWidth");
	int oriHeight    = cmdPara.get<int>("originHeight");
	std::string dir  = cmdPara.get<std::string>("dir");

	std::cout << "dir: " << dir << std::endl;
	
	if (!isDirExist(dir)) {
		mkdir(dir.c_str(), S_IRWXU);
	}

	wndTitle = file;
	cap.open(file);

	YAML::Node conf = initConfigure();

    cv::Mat frame;
    cap >> frame;
    int i = 0;

    cv::Rect subFrameRect = getSubFrameRect(conf, frame, oriWidth, oriHeight);

	while(!frame.empty()) {
		char c = (char)cv::waitKey(1);
    	if (c == 27) {
    		break;
    	}

    	cv::Mat subFrame;
		subFrame = frame(subFrameRect);

    	cv::Mat greyFrame;
    	if (frame.channels() == 1) {
    		greyFrame = subFrame;
    	} else {
    		cv::cvtColor(subFrame, greyFrame, cv::COLOR_RGB2GRAY);
    	}
    
		cv::imwrite(dir + "/" + boost::lexical_cast<std::string>(i) + ".png", greyFrame);
		//cv::imshow(wndTitle, subFrame);
		++i;
		cap >> frame;
  	}

	return 0;
}