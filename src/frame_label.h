#ifndef FRAME_LABEL_H_
#define FRAME_LABEL_H_

#include <string>
#include <fstream>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <opencv2/opencv.hpp>
#include <boost/lexical_cast.hpp>

#include <yaml-cpp/yaml.h>

#include "conf.h"
#include "cmdlineutils.h"
#include "matrixutils.h"

cv::Rect GetSubFrameRect(YAML::Node &conf, cv::Mat &frame, int oriWidth, int oriHeight, EVideoType vt); 
cv::Mat GetLabel(cv::Mat &frame, cv::Rect rect, EVideoType vt);
void GetFrameLabel(const std::string &file, const int &oriWidth, const int &oriHeight, const EVideoType &vt, const std::string &dir);
int GetVideoFrameNumber(const std::string &video);

#endif // FRAME_LABEL_H_