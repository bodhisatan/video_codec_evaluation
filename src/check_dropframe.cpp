#include "cmdlineutils.h"
#include "matrixutils.h"
#include "frame_drop_detect.h"

int main(int argc, char *argv[]) {
	cmdline::parser cmdPara = checkDropFrameCmdLine(argc, argv);
	cmdPara.parse_check(argc, argv);

	std::string file = cmdPara.get<std::string>("input");
	int oriWidth     = cmdPara.get<int>("originWidth");
	int oriHeight    = cmdPara.get<int>("originHeight");
	int oriType      = cmdPara.get<int>("originVideoType");
	std::string dir  = cmdPara.get<std::string>("dir");
	
	GetFrameLabel(file, oriWidth, oriHeight, EVideoType(oriType), dir);
	
	return 0;
}

