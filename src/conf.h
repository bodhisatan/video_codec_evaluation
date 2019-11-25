#ifndef CONF_H_
#define CONF_H_

#include <yaml-cpp/yaml.h>

YAML::Node initConfigure();

YAML::Node initOcrConf();

YAML::Node initPsnrConf();
#endif // CONF_H_