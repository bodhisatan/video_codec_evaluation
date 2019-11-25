#ifndef FRAME_DROP_DETECT_H_
#define FRAME_DROP_DETECT_H_

#include <string>
#include <fstream>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <boost/lexical_cast.hpp>

#include <yaml-cpp/yaml.h>

#include "conf.h"
#include "cmdlineutils.h"
#include "matrixutils.h"
#include "ocr.h"

enum EPixFormat {
	YUV444     = 1,
	YUV422     = 2,
	YUV420     = 3,
};

cv::Rect GetSubFrameRect(YAML::Node &conf, cv::Mat &frame, int oriWidth, int oriHeight, EVideoType vt); 
cv::Mat GetLabel(cv::Mat &frame, cv::Rect rect, EVideoType vt);
void GetFrameLabel(const std::string &file, const int &oriWidth, const int &oriHeight, const EVideoType &vt, const std::string &dir);
int GetVideoFrameNumber(const std::string &video);
cv::Size GetVideoResolution(const std::string &path, EVideoType vt = CAMERA_OTHERS);
bool CheckFrameDrop(const std::string &path, int videoFrameNumber, std::vector<int> &res);

#endif // FRAME_DROP_DETECT_H_