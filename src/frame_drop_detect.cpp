#include "cmdlineutils.h"
#include "frame_drop_detect.h"

void GetFrameLabel(const std::string &file, const int &oriWidth, const int &oriHeight, const EVideoType &vt, const std::string &dir) {
	cv::VideoCapture cap;

	if (!isDirExist(dir)) {
		mkdir(dir.c_str(), S_IRWXU);
	}

	cv::Mat frame;
	cap.open(file);
    cap >> frame;

    YAML::Node conf = initConfigure();
    cv::Rect subFrameRect = GetSubFrameRect(conf, frame, oriWidth, oriHeight, vt);

    int i = 0;
	while(!frame.empty()) {
		char c = (char)cv::waitKey(1);
    	if (c == 27) {
    		break;
    	}

    	cv::Mat subFrame;
		subFrame = GetLabel(frame, subFrameRect, vt);

    	// cv::Mat greyFrame;
    	// if (frame.channels() == 1) {
    	// 	greyFrame = subFrame;
    	// } else {
    	// 	cv::cvtColor(subFrame, greyFrame, cv::COLOR_RGB2GRAY);
    	// }
    
		cv::imwrite(dir + "/" + boost::lexical_cast<std::string>(i) + ".png", subFrame);
		++i;
		cap >> frame;
  	}
}

cv::Rect GetSubFrameRect(YAML::Node &conf, cv::Mat &frame, int oriWidth, int oriHeight, EVideoType vt) {
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

cv::Mat GetLabel(cv::Mat &frame, cv::Rect rect, EVideoType vt) {
	cv::Mat subFrame;

	subFrame = frame(rect);

	if (vt == CAMERA_FACING_FRONT) {
		subFrame = imgRotate(subFrame, 90, false);
	} else if (vt == CAMERA_FACING_BACK) {
		subFrame = imgRotate(subFrame, 90, true);
	}

	return subFrame;
}

int GetVideoFrameNumber(const std::string &video) {
	cv::VideoCapture cap;
	cv::Mat frame;

	int frameNumber = 0;
	if (!isFileExist(video)) {
		return frameNumber;
	}

	cap.open(video);
    cap >> frame;

	while(!frame.empty()) {
		++frameNumber;
		cap >> frame;
  	}

  	return frameNumber;
}

bool CheckFrameDrop(const std::string &path, int videoFrameNumber, std::vector<int> &res) {
	res.clear();

	if (!isDirExist(path) || videoFrameNumber < 0) {
		return false;
	}

	res.resize(videoFrameNumber);

	std::vector<std::string> files;
    GetFiles(path, files);
    std::cout << "files size: " << files.size() << std::endl;
    std::cout << ">>>begin process the ocr detect<<<" << std::endl;
    
    int ii = 0;
    for (auto &i : files) {
        ++ii;
        std::string f = path + "/" + i;
        int num = GetOcrNumber(f);

        if (num >= 0) {
        	++res[num];
        }

        try {
        	std::string msg = " process the file: " + f + ", the res is: " + 
                          boost::lexical_cast<std::string>(num);
	        std::cout << "...[" << ii << "/" 
	                  << videoFrameNumber << "]" << std::setw(57) << msg; 
	        fflush(stdout);
	        usleep(100000);
	        std::cout << "\r\033[k";
        } catch(...) {}
        
    }
    std::cout << std::endl;
    std::cout << ">>>end process the ocr detect<<<" << std::endl;

    return true;
}

cv::Size GetVideoResolution(const std::string &video, EVideoType vt) {
	cv::Size r = cv::Size(0, 0);
	cv::VideoCapture cap;
	cv::Mat frame;

	if (!isFileExist(video)) {
		return r;
	}

	cap.open(video);
    cap >> frame;

	if (frame.empty()) {
		return r;
  	}

  	if (vt == CAMERA_FACING_FRONT || vt == CAMERA_FACING_BACK) {
  		r = cv::Size(frame.rows, frame.cols);
  	} else {
  		r = cv::Size(frame.cols, frame.rows);
  	}
  	
  	return r;
}