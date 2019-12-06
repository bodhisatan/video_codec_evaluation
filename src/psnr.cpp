#include "psnr.h"

unsigned pow2(unsigned base) {
    return base*base;
}

double getPsnr(double mse, int max) {
    return 10.0 * log10(pow2(max) / (mse));
}

double computeImagesMse(const unsigned char *main_data, const unsigned char *ref_data, 
    const EPixFormat format, const int w, const int h, const int plane) {
    
    int size  = 0; 
    int begin = 0;
    int end   = 0;
    int deno  = 0;

    if (format == YUV420) {size = (w * h * 3) >> 1;}

    if (plane == 0) {begin = 0; end = w * h; deno = w * h; } 
    if (plane == 1) {begin = w * h; end = (w * h) + ((w * h) >> 2); deno = (w * h) >> 2;}
    if (plane == 2) {begin = (w * h) + ((w * h) >> 2); end = size; deno = (w * h) >> 2;}
    if (plane == 3) {begin = 0; end = size; deno = size;}
    
    double mse = 0.0;

    for (int i = begin; i < end; ++i) {
        // std::cout << "(" << (int)main_data[i] << ", " << (int)ref_data[i] << ")" << std::endl;
        mse += pow2((int)main_data[i] - (int)ref_data[i]);
    }


    return mse / (double) (deno);
}

uchar ssdNormal(int ssd, int min_ssd, int max_ssd, int level, int normal) {
    int oriInterval = (max_ssd - min_ssd + 1) / (int)level;
    int normalInterval = (normal + 1) / (int)level;

    std::vector<int> oriList;
    std::vector<int> normalList;

    for (int i = 0; i <= level; ++i) {
        oriList.emplace_back(min_ssd + oriInterval * i);
        normalList.emplace_back(normalInterval * i);
    }
    
    int res = 0;
    for (int i = 1; i <= level; ++i) {
        if (ssd <= oriList[i]) {
            res = normalList[i - 1];
            break;
        } 
    }

    return (uchar)res;
}

void computeSsdImage(const unsigned char *main_data, const unsigned char *ref_data, 
    const EPixFormat format, const int w, const int h, const int plane, int frame_no, cv::VideoWriter &writer) {
    
    int size    = 0;
    int begin   = 0;
    int end     = 0;
    
    int min_ssd = 0;
    int max_ssd = 0;

    int w1 = 0;
    int h1 = 0;

    if (format == YUV420) {size = (w * h * 3) >> 1;}

    if (plane == 0) {begin = 0; end = w * h; w1 = w; h1 = h;} 
    if (plane == 1) {begin = w * h; end = (w * h) + ((w * h) >> 2); w1 = w >> 1; h1 = h >> 1;}
    if (plane == 2) {begin = (w * h) + ((w * h) >> 2); end = size; w1 = w >> 1; h1 = h >> 1;}
    
    int ssd = 0;

    min_ssd = max_ssd = pow2((int)main_data[begin] - (int)ref_data[begin]);

    for (int i = begin + 1; i < end; ++i) {
        ssd = pow2((int)main_data[i] - (int)ref_data[i]);
        if (ssd < min_ssd) {min_ssd = ssd;}
        if (ssd > max_ssd) {max_ssd = ssd;}
    }
    
    if (max_ssd > 255) {max_ssd = 255;}

    {
        min_ssd = 0;
        max_ssd = 255;
    }
    
    cv::Mat greyFrame = cv::Mat(h1, w1, CV_8UC1, cv::Scalar(255));

    for (int i = begin; i < end; ++i) {
        ssd = pow2((int)main_data[i] - (int)ref_data[i]);
        uchar ssd_nor = ssdNormal(ssd, min_ssd, max_ssd, 8, 255);
        int j = i - begin;
        greyFrame.at<uchar>((int)(j / w1), (int)(j % w1)) = ssd_nor;
    }

    writer.write(greyFrame);
}

void computeBlockPsnrImage(const unsigned char *main_data, const unsigned char *ref_data, 
    const EPixFormat format, const int w, const int h, const int step, const int plane, int frame_no, cv::VideoWriter &writer) {
    
    int size    = 0;
    int begin   = 0;
    int end     = 0;
    
    int w1 = 0;
    int h1 = 0;

    if (format == YUV420) {size = (w * h * 3) >> 1;}

    if (plane == 0) {begin = 0; end = w * h; w1 = w; h1 = h;} 
    if (plane == 1) {begin = w * h; end = (w * h) + ((w * h) >> 2); w1 = w >> 1; h1 = h >> 1;}
    if (plane == 2) {begin = (w * h) + ((w * h) >> 2); end = size; w1 = w >> 1; h1 = h >> 1;}
    
    int w_t = w1 / step;
    int h_t = h1 / step;
    int pixel_count_t = w_t * h_t;

    cv::Mat greyFrame = cv::Mat(h_t, w_t, CV_8UC1, cv::Scalar(255));
    
    for (int i = 0; i < pixel_count_t; ++i) {
        // 对目标的每一个块求psnr.
        int ssd = 0;
        for (int j = 0; j < step; ++j) {
            for (int k = 0; k < step; ++k) {
                int pixel_index = ((i / w_t) * step + j) * w1 + ((i % w_t) * step + k) + begin;
                ssd += pow2((int)main_data[pixel_index] - (int)ref_data[pixel_index]);
            }
        }

        double mse  = 0.0;
        int psnr = 0;
        
        if (ssd == 0) {
            psnr = 100;
        } else {
            mse = ssd / (double)(pow2(step));
            psnr = (int)getPsnr(mse, 255);
        }
        
        // if (min_psnr > psnr) {min_psnr = psnr;}
        // if (max_psnr < psnr) {max_psnr = psnr;}
        psnr *= 1;
        if (psnr > 255) {psnr = 255;}

        greyFrame.at<uchar>((int)(i / w_t), (int)(i % w_t)) = (uchar)(psnr);
    }

    writer.write(greyFrame);
}

bool psnrAndVisualize(const std::string &main_video, const std::string &ref_video,
                      const EPixFormat format, const int frame_count,
                      const int width, const int height,  
                      const std::vector<int> &frame_drop_info,
                      const int step) {
    if (!isFileExist(main_video) || !isFileExist(ref_video)) {
        std::cout << main_video << "||" << ref_video << " 不存在!" << std::endl; 
        return false;
    }

    YAML::Node conf = initPsnrConf();
    if (!conf["psnr"] || !conf["psnr"]["srcDir"] || !conf["psnr"]["resDir"]) {
        std::cout << "解析psnr.yaml文件失败!" << std::endl;
        return false;
    }

    std::string resDir = conf["psnr"]["resDir"].as<std::string>();
    if (!isDirExist(resDir)) {
        mkdir(resDir.c_str(), S_IRWXU);
    }

    // 目前没有使用format参数，默认为YUV420格式，后续需要增加.

    // 每一帧的yuv数据所占的字节大小
    int frame_bytes = 0;
    if (format == YUV420) {frame_bytes = (width * height * 3) >> 1;}
    
    FILE          *f1, *f2;
    unsigned char *b1, *b2;

    // 当前处理的帧.
    int current_frame = -1;

    // 打开文件
    if ((f1 = fopen(main_video.c_str(), "rb")) == 0 || (f2 = fopen(ref_video.c_str(), "rb")) == 0) {
        std::cout << "open main_video || ref_video failed!" << std::endl;
        if (f1) {
            fclose(f1);
        }

        std::cout << "打开文件失败!" << std::endl;
        return false;
    }

    // 如果申请内存失败则返回。
    if (!(b1 = (unsigned char *)malloc(frame_bytes)) || !(b2 = (unsigned char *)malloc(frame_bytes))) {
        std::cout << "malloc memory for b1 || b2 failed!" << std::endl;
        if (b1) {
            free(b1);
        }

        std::cout << "申请内存失败!" << std::endl;
        return false;
    }

    // 时间维度的psnr信息.
    std::string psnr_log     = resDir + "/psnr.log";
    std::ofstream psnr_log_f(psnr_log, std::ios::out);
    if(!psnr_log_f) {
        std::cout << "打开文件: " << psnr_log << "失败!" << std::endl;
        return false;
    }

    // 空间psnr可视化, 分别将y/u/v各分量的差值写入视频文件.
    cv::VideoWriter writer_y = cv::VideoWriter(resDir + "/y.avi", 
                                               cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 
                                               30, 
                                               cv::Size(width, height), 
                                               false);
    cv::VideoWriter writer_u = cv::VideoWriter(resDir + "/u.avi", 
                                               cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 
                                               30, 
                                               cv::Size(width >> 1, height >> 1), 
                                               false);
    cv::VideoWriter writer_v = cv::VideoWriter(resDir + "/v.avi", 
                                               cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 
                                               30, 
                                               cv::Size(width >> 1, height >> 1), 
                                               false);
    cv::VideoWriter writer_y1 = cv::VideoWriter(resDir + "/y1.avi", 
                                               cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 
                                               30, 
                                               cv::Size(width / step, height / step), 
                                               false);
    // 打开文件开始按每一帧读取数据然后计算.
    while (1) {
        // 处理第current_frame帧，从0开始.
        ++current_frame;

        std::string msg = "";

        try {
            msg = " 计算第 " + boost::lexical_cast<std::string>(current_frame + 1) + " 帧的psnr";
        } catch(...){}

        std::cout << "...[" << current_frame + 1 << "/" 
                      << frame_count << "]" << std::setw(20) << msg; 
        fflush(stdout);

        std::memset(b1, 0, frame_bytes);
        std::memset(b2, 0, frame_bytes);

        // 如果存在丢帧，则跳过
        if (frame_drop_info[current_frame] == 0) {
            if (1 != fread(b1, frame_bytes, 1, f1) || 1 != fread(b2, frame_bytes, 1, f2)) {
                std::cout << "at least one file pointer get the end!" << std::endl;
                break; 
            } else {
                psnr_log_f << "n:" << current_frame + 1
                          << std::setiosflags(std::ios::fixed) << std::setprecision(2)
                          << " mse_avg:"  << 0  
                          << " mse_y:"    << 0 
                          << " mse_u:"    << 0 
                          << " mse_v:"    << 0
                          << " psnr_avg:" << 0
                          << " psnr_y:"   << 0
                          << " psnr_u:"   << 0
                          << " psnr_v:"   << 0  
                          << std::endl;

                std::cout << "\r\033[k"; // 清空命令行.
                continue;
            }
        }
        
        if (1 != fread(b1, frame_bytes, 1, f1) || 1 != fread(b2, frame_bytes, 1, f2)) {
            std::cout << "at least one file pointer get the end!" << std::endl;
            break; 
        }

        // 计算y分量psnr
        double mse_y   = computeImagesMse(b1, b2, YUV420, width, height);
        double mse_u   = computeImagesMse(b1, b2, YUV420, width, height, 1);
        double mse_v   = computeImagesMse(b1, b2, YUV420, width, height, 2);
        double mse_avg = computeImagesMse(b1, b2, YUV420, width, height, 3);

        psnr_log_f << "n:" << current_frame + 1
                  << std::setiosflags(std::ios::fixed) << std::setprecision(2)
                  << " mse_avg:"  << mse_avg  
                  << " mse_y:"    << mse_y 
                  << " mse_u:"    << mse_u 
                  << " mse_v:"    << mse_v
                  << " psnr_avg:" << getPsnr(mse_avg, 255)
                  << " psnr_y:"   << getPsnr(mse_y, 255)
                  << " psnr_u:"   << getPsnr(mse_u, 255)
                  << " psnr_v:"   << getPsnr(mse_v, 255)  
                  << std::endl;

        computeSsdImage(b1, b2, YUV420, width, height, 0, current_frame, writer_y);
        computeSsdImage(b1, b2, YUV420, width, height, 1, current_frame, writer_u);
        computeSsdImage(b1, b2, YUV420, width, height, 2, current_frame, writer_v);  
        computeBlockPsnrImage(b1, b2, YUV420, width, height, step, 0, current_frame, writer_y1);

        std::cout << "\r\033[k"; // 清空命令行.      

        break;
    }

    // 关闭资源
    fclose(f1);
    fclose(f2);
    free(b1);
    free(b2);
    psnr_log_f.close();

    return true;
} 