#include "matrixutils.h"

MES rotateMatrix270(MES in, Resolution vr) {
	MES out = {.x = 0, .y= 0};

	out.x = vr.height - 1 - in.y;
	out.y = in.x;

	return out;
}