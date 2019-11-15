#include "matrixutils.h"

MES rotateMatrix270(MES in, Resolution vr) {
	MES out = {.x = 0, .y= 0};

	out.x = vr.height - 1 - in.y;
	out.y = in.x;

	return out;
}

MES rotateMatrix90(MES in, Resolution vr) {
	MES out = {.x = 0, .y = 0};

	out.x = in.y;
	out.y = vr.width - 1 - in.x;

	return out;
}

cv::Rect rotate90(cv::Rect in, Resolution vr) {
	cv::Rect out = {0, 0, 0, 0};

	out.x      = vr.width - in.y;
	out.y      = in.x;
	out.width  = in.height;
	out.height = in.width;
	out.x      = out.x - out.width; // 修正y坐标，因为旋转90度后，左下角的点变为了左上角。

	return out;
}