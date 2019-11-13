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