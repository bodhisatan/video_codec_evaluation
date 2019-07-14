#include <opencv2/opencv.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include <boost/lexical_cast.hpp>

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

    cv::Mat frame;
    cap >> frame;
    int i = 0;
	while(!frame.empty()) {
		char c = (char)cv::waitKey(1);
    	if (c == 27) {
    		break;
    	}

    	cv::Mat subFrame;
		cv::Rect subFrameRect = cv::Rect(30, 80, 250, 130);
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