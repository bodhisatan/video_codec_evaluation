#include "conf.h"

YAML::Node initConfigure() {
	return YAML::LoadFile("conf/conf.yaml");
}

YAML::Node initOcrConf() {
	return YAML::LoadFile("conf/ocr.yaml");
}