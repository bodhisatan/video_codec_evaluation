#ifndef CMDLINEUTILS_H_
#define CMDLINEUTILS_H_

#include <string>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <dirent.h>

#include "cmdline.h"

cmdline::parser initCmdLine(int argc, char *argv[]);
cmdline::parser checkDropFrameCmdLine(int argc, char *argv[]);
bool isDirExist(std::string dir);
bool GetFiles(const std::string &path, std::vector<std::string> &files, const std::string &exten = "*");

#endif // CMDLINEUTILS_H_