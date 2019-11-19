#ifndef OCR_H_
#define OCR_H_

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

#include <boost/algorithm/hex.hpp>
#include <boost/uuid/detail/md5.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/lexical_cast.hpp>

#include "http_request.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8>> Base64EncodeIterator;

bool Base64Encode(const std::string &input, std::string &output);

bool FileBase64Encode(const std::string &path, std::string &output);

bool GetMd5(std::string &str_md5, const char  * const buffer, size_t buffer_size);

int64_t GetCurrentStamp64();

std::string GetOcrToken(YAML::Node &conf);

std::string GetOcrResult(std::string image_path);

// 在应用中，识别的数字全部为正整数。
// 因此，如果没有识别，则返回-1，如果识别了，则返回对应的整数。
int ParseOcrNumber(std::string &ocr_result);

int GetOcrNumber(std::string &path);

#endif // OCR_H_