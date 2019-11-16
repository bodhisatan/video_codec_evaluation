#ifndef CMDLINEUTILS_H_
#define CMDLINEUTILS_H_

#include <string>
#include <unistd.h>
#include <dirent.h>

#include "cmdline.h"

cmdline::parser initCmdLine(int argc, char *argv[]);
cmdline::parser checkDropFrameCmdLine(int argc, char *argv[]);
bool isDirExist(std::string dir);

#endif // CMDLINEUTILS_H_