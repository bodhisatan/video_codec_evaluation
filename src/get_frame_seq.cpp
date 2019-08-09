#include <opencv2/opencv.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <boost/lexical_cast.hpp>

YAML::Node initConfigure() {
	return YAML::LoadFile("conf/conf.yaml");
}

cv::Rect getSubFrameRect(YAML::Node &conf, cv::Mat &frame) {
	cv::Rect rect(0, 0, 0, 0);

	if (!conf["originalVideo"] || 
		!conf["originalVideo"]["resolution"] ||
		!conf["originalVideo"]["flagArea"]) {
        return rect;
    }

	int oriWidth    = conf["originalVideo"]["resolution"]["width"].as<int>();
	int oriHeight   = conf["originalVideo"]["resolution"]["height"].as<int>();

	int oriFlagX    = conf["originalVideo"]["flagArea"]["x"].as<int>();
	int oriFlagY    = conf["originalVideo"]["flagArea"]["y"].as<int>();
	int oriFlagW    = conf["originalVideo"]["flagArea"]["width"].as<int>();
	int oriFlagH    = conf["originalVideo"]["flagArea"]["height"].as<int>();

	int frameWidth  = frame.cols;
	int frameHeight = frame.rows;

	int ratioX      = oriWidth  / frameWidth;
	int ratioY      = oriHeight / frameHeight;

	std::cout << "ratioX: " << ratioX << ", ratioY:" << ratioY << std::endl;
	
	rect = cv::Rect(oriFlagX / ratioX, 
		            oriFlagY / ratioY, 
		            oriFlagW / ratioX,
		            oriFlagH / ratioY);

	return rect;
}

int main(int argc, char *argv[]) {
	cv::VideoCapture cap;
	std::string wndTitle;

	if (argc == 1) {
		std::cout << "the parameters error." << std::endl;
		std::cout << "Usage: " << argv[0] << " file" << std::endl;
		return 0;
	} else {
		wndTitle = argv[1];
		cap.open(std::string(argv[1]));
	}

	YAML::Node conf = initConfigure();

    cv::Mat frame;
    cap >> frame;
    int i = 0;
    cv::Rect subFrameRect = getSubFrameRect(conf, frame);

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
    
		cv::imwrite("data/" + boost::lexical_cast<std::string>(i) + ".png", greyFrame);
		//cv::imshow(wndTitle, subFrame);
		++i;
		cap >> frame;
  	}

	return 0;
}