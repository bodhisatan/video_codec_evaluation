#include "cmdlineutils.h"

cmdline::parser initCmdLine(int argc, char *argv[]) {
	cmdline::parser cmdPara;

	cmdPara.add<std::string>("input", 'i', "the video after codec operator that needed to process", true, "");
	cmdPara.add<int>("originWidth", 'w', "the origin video width before codec operator", true, 0);
	cmdPara.add<int>("originHeight", 'h', "the origin video height before codec operator", true, 0);
	cmdPara.add<std::string>("dir", 'd', "the path that the frame number area extract from the video", false, "data");

	return cmdPara;
}

cmdline::parser vpsnrCmdLine(int argc, char *argv[]) {
	cmdline::parser cmdPara;

	cmdPara.add<std::string>("refVideo", 'r', "含有噪声视频的参考视频", true, "");
	cmdPara.add<std::string>("mainVideo", 'm', "含有噪声的视频", true, "");
	cmdPara.add<int>("blockSize", 'b', "计算分块psnr的块大小", false, 1);
	cmdPara.add<bool>("dropFrameDetect", 'd', "是否执行丢帧检测", false, false);
	cmdPara.add<std::string>("dropFrameInfoFile", 'f', "手动指定丢帧信息文件", false, "");

	return cmdPara;
}

bool isDirExist(std::string dir) {
	if (dir == "") {
		return false;
	}

	if (opendir(dir.c_str()) == NULL) {
		return false;
	}

	return true;
}

bool isFileExist(const std::string &file) {
	std::ifstream fin(file);
    if (!fin) {
        std::cout << "the file: " << file << " does not exist." << std::endl;
        return false;
    }

    return true;    
}

cmdline::parser checkDropFrameCmdLine(int argc, char *argv[]) {
	cmdline::parser cmdPara;

	cmdPara.add<std::string>("mainVideo", 'm', "待检测丢帧的视频",  true, "");
	cmdPara.add<std::string>("refVideo",   'r', "原视频", true, "");
	

	return cmdPara;
}

bool GetFiles(const std::string &path, std::vector<std::string> &files, const std::string &exten) {
    files.clear();
 
    DIR* dp = nullptr;
    struct dirent* dirp = nullptr;
    if ((dp = opendir(path.c_str())) == nullptr) {
        return false;
    }
 
    while ((dirp = readdir(dp)) != nullptr) {
        if (dirp->d_type == DT_REG) {
            if (exten.compare("*") == 0) {
                files.emplace_back(static_cast<std::string>(dirp->d_name));
            } else{
                if (std::string(dirp->d_name).find(exten) != std::string::npos) {
                    files.emplace_back(static_cast<std::string>(dirp->d_name));
                }
            }
        }
    }
 
    closedir(dp);
 
    return true;
}

bool DeleteFiles(const std::string &path) {
	if (!isDirExist(path)) {
		return false;
	}

	std::vector<std::string> files;
	GetFiles(path, files);

	for (auto &i : files) {
		std::string f = path + "/" + i;
    	remove(f.c_str());
    }

    return true;
}
