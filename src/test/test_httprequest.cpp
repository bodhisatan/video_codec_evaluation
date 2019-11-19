#include <iostream>

#include "../ocr.h"

int main(int argc, char *argv[]) {
    std::string image = "../../psnr/data/3.png";
    std::cout << GetOcrNumber(image) << std::endl;
    
	return 0;
}

