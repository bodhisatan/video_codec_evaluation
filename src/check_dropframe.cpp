#include <string>
#include <fstream>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <opencv2/opencv.hpp>
#include <boost/lexical_cast.hpp>

#include "conf.h"
#include "cmdlineutils.h"
#include "matrixutils.h"

static cv::Rect getSubFrameRect(YAML::Node &conf, cv::Mat &frame, int oriWidth, int oriHeight, EVideoType vt); 
static cv::Mat getLabel(cv::Mat &frame, cv::Rect rect, EVideoType vt);

int main(int argc, char *argv[]) {
	cv::VideoCapture cap;
	std::string wndTitle;

	cmdline::parser cmdPara = checkDropFrameCmdLine(argc, argv);
	cmdPara.parse_check(argc, argv);

	std::string file = cmdPara.get<std::string>("input");
	int oriWidth     = cmdPara.get<int>("originWidth");
	int oriHeight    = cmdPara.get<int>("originHeight");
	int oriType      = cmdPara.get<int>("originVideoType");
	std::string dir  = cmdPara.get<std::string>("dir");
	
	if (!isDirExist(dir)) {
		mkdir(dir.c_str(), S_IRWXU);
	}

	wndTitle = file;
	cap.open(file);

	YAML::Node conf = initConfigure();

    cv::Mat frame;
    cap >> frame;
    int i = 0;

    cv::Rect subFrameRect = getSubFrameRect(conf, frame, oriWidth, oriHeight, EVideoType(oriType));

	while(!frame.empty()) {
		char c = (char)cv::waitKey(1);
    	if (c == 27) {
    		break;
    	}

    	cv::Mat subFrame;
		subFrame = getLabel(frame, subFrameRect, EVideoType(oriType));

    	// cv::Mat greyFrame;
    	// if (frame.channels() == 1) {
    	// 	greyFrame = subFrame;
    	// } else {
    	// 	cv::cvtColor(subFrame, greyFrame, cv::COLOR_RGB2GRAY);
    	// }
    
		cv::imwrite(dir + "/" + boost::lexical_cast<std::string>(i) + ".png", subFrame);
		//cv::imshow(wndTitle, subFrame);
		++i;
		cap >> frame;
  	}

	return 0;
}

cv::Rect getSubFrameRect(YAML::Node &conf, cv::Mat &frame, int oriWidth, int oriHeight, EVideoType vt) {
	cv::Rect out(0, 0, 0, 0);

	if (!conf["originalVideo"] || 
		!conf["originalVideo"]["flagArea"]) {
        return out;
    }

	int oriFlagX    = conf["originalVideo"]["flagArea"]["x"].as<int>();
	int oriFlagY    = conf["originalVideo"]["flagArea"]["y"].as<int>();
	int oriFlagW    = conf["originalVideo"]["flagArea"]["width"].as<int>();
	int oriFlagH    = conf["originalVideo"]["flagArea"]["height"].as<int>();

	cv::Rect in = cv::Rect(oriFlagX, oriFlagY, oriFlagW, oriFlagH);
	Resolution vr = {.width = oriWidth, .height = oriHeight};
	
	// 修正旋转
	if (vt == CAMERA_FACING_FRONT) {
		std::cout << "旋转270度" << std::endl;
		out = rotate270(in, vr);
	} else if (vt == CAMERA_FACING_BACK) {
		std::cout << "旋转90度" << std::endl;
		out = rotate90(in, vr);
	}

	int frameWidth  = frame.cols;
	int frameHeight = frame.rows;

	float ratioX      = oriWidth * 1.0 / frameWidth;
	float ratioY      = oriHeight * 1.0 / frameHeight;

	std::cout << "ratioX: " << ratioX << ", ratioY:" << ratioY << std::endl;
	
	out.x      = int(out.x / ratioX);
	out.y      = int(out.y / ratioY);
	out.width  = int(out.width / ratioX);
	out.height = int(out.height / ratioY);
	std::cout << "x:" << out.x << ", y:" << out.y << ", width:" << out.width << ",height:" << out.height << std::endl; 

	return out;
}

cv::Mat getLabel(cv::Mat &frame, cv::Rect rect, EVideoType vt) {
	cv::Mat subFrame;

	subFrame = frame(rect);

	if (vt == CAMERA_FACING_FRONT) {
		subFrame = imgRotate(subFrame, 90, false);
	} else if (vt == CAMERA_FACING_BACK) {
		subFrame = imgRotate(subFrame, 90, true);
	}

	return subFrame;
}