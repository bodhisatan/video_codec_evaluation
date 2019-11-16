#ifndef CONF_H_
#define CONF_H_

#include <yaml-cpp/yaml.h>

YAML::Node initConfigure() {
	return YAML::LoadFile("conf/conf.yaml");
}

#endif // CONF_H_