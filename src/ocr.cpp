#include "ocr.h"
#include "conf.h"

bool Base64Encode(const std::string &input, std::string &output) {
    std::stringstream result;

    try {
        std::copy(Base64EncodeIterator(input.begin()), Base64EncodeIterator(input.end()), std::ostream_iterator<char>(result));
    } catch ( ... ) {
        return false;
    }

    size_t equal_count = (3 - input.length() % 3) % 3;
    for (size_t i = 0; i < equal_count; i++) {
        result.put( '=' );
    }

    output = result.str();
    return output.empty() == false;
}

bool FileBase64Encode(const std::string &path, std::string &output) {
	std::ifstream file;
    file.open(path, std::ios::binary | std::ios::in);

    std::stringstream buffer;
    buffer << file.rdbuf();  
    std::string strBuffer = buffer.str();
    
    Base64Encode(strBuffer, output);
    
    file.close();

    return true;
}

bool GetMd5(std::string &str_md5, const char  * const buffer, size_t buffer_size) {
	if (buffer == nullptr) {
		return false;
	}

	boost::uuids::detail::md5 boost_md5;
	boost_md5.process_bytes(buffer, buffer_size);

	boost::uuids::detail::md5::digest_type digest;
	boost_md5.get_digest(digest);
	const auto char_digest = reinterpret_cast<const char*>(&digest);
	str_md5.clear();
	boost::algorithm::hex(char_digest, 
                          char_digest + sizeof(boost::uuids::detail::md5::digest_type), 
                          std::back_inserter(str_md5));
    std::transform(str_md5.begin(), 
                   str_md5.end(), 
                   str_md5.begin(), 
                   [](unsigned char c) -> unsigned char { return std::tolower(c);});
	
    return true;
}

int64_t GetCurrentStamp64() {
	boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
	boost::posix_time::time_duration time_from_epoch = boost::posix_time::second_clock::universal_time() - epoch;

	return time_from_epoch.total_seconds();
}

std::string GetOcrToken(YAML::Node &conf) {
	std::string url   = conf["ocr"]["url"].as<std::string>();
    std::string appid = conf["ocr"]["appid"].as<std::string>();
    std::string uid   = conf["ocr"]["uid"].as<std::string>();
    std::string sk    = conf["ocr"]["sk"].as<std::string>();
    std::string type  = conf["ocr"]["type"].as<std::string>();
    int64_t     time  = GetCurrentStamp64();

    std::string time_string = boost::lexical_cast<std::string>(time);
    std::string sign_string = time_string + uid + appid + sk;
    std::string sign;
    GetMd5(sign, sign_string.c_str(), sign_string.size());

    std::string token;
    token = type + "." + sign + "." + time_string + "." + uid + "-" + appid;

    return token;
}

std::string GetOcrResult(std::string image_path) {
    YAML::Node conf   = initOcrConf();
    std::string token = GetOcrToken(conf);

    std::string image_base64;
    FileBase64Encode(image_path, image_base64);

    const std::string url   = conf["ocr"]["url"].as<std::string>() + token;
    const std::map<std::string, std::string> data{{"image", image_base64}};

    const std::vector<std::string> headers = {
        "Content-Type: application/x-www-form-urlencoded",
        "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.97 Safari/537.36"
    };

    std::string result = "";

    try {
        http::Request request(url);
        const http::Response response = request.send("POST", data, headers);
        result = std::string(response.body.begin(), response.body.end());
    } catch (const std::exception& e) {
        std::cerr << "Request failed, error: " << e.what() << std::endl;
    }

    return result;
}

// 在应用中，识别的数字全部为正整数。
// 因此，如果没有识别，则返回-1，如果识别了，则返回对应的整数。
// {"log_id":3127307901,"words_result":[{"location":{"left":4,"top":7,"width":39,"height":44},"words":"2"}],"words_result_num":1}
int ParseOcrNumber(std::string &ocr_result) {
    int res = -1;

    rapidjson::Document doc;
    doc.Parse(ocr_result.c_str());

    if (!doc.IsObject()) {
        return res;
    }

    if (!doc.HasMember("words_result")) {
        return res;
    }
    
    if (!doc["words_result"].IsArray()) {
        return res;
    }

    for (auto &v : doc["words_result"].GetArray()) {
        if (v.IsObject() && v.HasMember("words")) {
            try {
                res = boost::lexical_cast<int>(v["words"].GetString());
            } catch (...) {
                std::cerr << "cast the result error." << std::endl;
            }
        }

        break;
    }

    return res;
}

int GetOcrNumber(std::string &path) {
    int ocr_number         = -2;

    std::ifstream fin(path);
    if (!fin) {
        std::cout << "the file: " << path << " does not exist." << std::endl;
        return ocr_number;
    }    

    std::string ocr_result = GetOcrResult(path);
    ocr_number             = ParseOcrNumber(ocr_result);

    return ocr_number;
}