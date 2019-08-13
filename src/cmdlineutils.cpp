#include "cmdlineutils.h"

cmdline::parser initCmdLine(int argc, char *argv[]) {
	cmdline::parser cmdPara;

	cmdPara.add<std::string>("input", 'i', "the video need to process", true, "");
	cmdPara.add<int>("originWidth", 'w', "the video width before codec operator", true, 0);
	cmdPara.add<int>("originHeight", 'h', "the video height before codec operator", true, 0);

	return cmdPara;
}