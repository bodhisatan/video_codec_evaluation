#include <iostream>

#include "ocr.h"

int main(int argc, char *argv[]) {
	// http::Request request("http://www.baidu.com");
	// const http::Response getResponse = request.send("GET");
    // std::cout << std::string(getResponse.body.begin(), getResponse.body.end()) << std::endl;

    std::string image = "/Users/wangwei/Documents/Project/github/video_codec_evaluation/psnr/data/3.png";
    std::cout << GetOcrNumber(image) << std::endl;
    // std::string encode;
    // FileBase64Encode(image, encode);


    // std::cout << std::endl << encode << std::endl;
    // std::cout << "size: " << encode.size() << std::endl;

    // std::string t = "157407829818845996089338386BVNc7UiTCnMxPSzPDOKH5bcgpIrIb0G1";
    // std::string md5;
    // GetMd5(md5, t.c_str(), t.size());
    // std::cout << "md5: " << md5 << std::endl;

    // 0c957519a32d31b8f85fbb43b446ebc1 
    // 0C957519A32D31B8F85FBB43B446EBC1

    // int64_t ti = 0;
    // ti = GetCurrentStamp64();
    // std::cout << "ti: " << ti << std::endl;

	return 0;
}

