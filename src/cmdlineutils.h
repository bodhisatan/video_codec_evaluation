#ifndef CMDLINEUTILS_H_
#define CMDLINEUTILS_H_

#include <string>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <fstream>
#include  <cstdio>
#include <iostream>
#include <stdio.h>
#include "../third_party/cmdline.h"

cmdline::parser initCmdLine(int argc, char *argv[]);
cmdline::parser checkDropFrameCmdLine(int argc, char *argv[]);
cmdline::parser vpsnrCmdLine(int argc, char *argv[]);

bool isDirExist(std::string dir);
bool GetFiles(const std::string &path, std::vector<std::string> &files, const std::string &exten = "*");
bool isFileExist(const std::string &file);
bool DeleteFiles(const std::string &path);

#endif // CMDLINEUTILS_H_