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
            psnr = 60;
        } else {
            mse = ssd / (double)(pow2(step));
            psnr = (int)getPsnr(mse, 255);
        }
        
        // std::cout << psnr << " ";
        // if (min_psnr > psnr) {min_psnr = psnr;}
        // if (max_psnr < psnr) {max_psnr = psnr;}
        psnr *= 1;
        if (psnr > 60) {psnr = 60;}

        greyFrame.at<uchar>((int)(i / w_t), (int)(i % w_t)) = (uchar)(psnr);
    }

    writer.write(greyFrame);
}

bool psnrAndVisualize(const std::string &main_video, const std::string &ref_video,
                      const EPixFormat format, const int frame_count,
                      const int width, const int height,  
                      const std::vector<int> &frame_drop_info,
                      const int step) {
    std::cout << "开始计算psnr" << std::endl;

    if (!isFileExist(main_video) || !isFileExist(ref_video)) {
        std::cout << main_video << "||" << ref_video << " 不存在!" << std::endl; 
        return false;
    }

    YAML::Node conf = initPsnrConf();
    if (!conf["psnr"] || !conf["psnr"]["ocrSrcDir"] || !conf["psnr"]["resDir"]) {
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
    /* 因为u,v分量的折损较小，所以不用可视化，如果需要，可以打开注释的代码.
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
    */
    cv::VideoWriter writer_y = cv::VideoWriter(resDir + "/y.avi", 
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

        // 如果存在丢帧，则参考视频需要跳过
        if (frame_drop_info[current_frame] == 0) {
            #ifdef DEBUG
            std::cout << std::endl << "...mainVideo存在丢帧帧，跳过refVideo的对应帧." << std::endl;
            #endif

            if (1 != fread(b2, frame_bytes, 1, f2)) {
                std::cout << std::endl << "...refVideo file pointer get the end!" << std::endl;
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
        
        // 如果存在重复帧，则噪声视频需要跳过.
        int repeat = (frame_drop_info[current_frame] - 1);
        while (repeat > 0) {
            #ifdef DEBUG
            std::cout << std::endl << "...mainVideo存在重复帧，跳过mainVideo的对应帧." << std::endl;
            #endif
            
            if (1 != fread(b1, frame_bytes, 1, f1)) {
                std::cout << std::endl << "...mainVideo file pointer get the end!" << std::endl;
                break; 
            } else {
                --repeat;
                std::memset(b1, 0, frame_bytes);
            }
        }
        
        // 对齐之后进行计算.
        if (1 != fread(b1, frame_bytes, 1, f1) || 1 != fread(b2, frame_bytes, 1, f2)) {
            std::cout << std::endl << "...at least one file pointer get the end!" << std::endl;
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

        /* 因为u,v分量的折损较小，所以不用可视化，如果需要，可以打开注释的代码.
        computeSsdImage(b1, b2, YUV420, width, height, 0, current_frame, writer_y);
        computeSsdImage(b1, b2, YUV420, width, height, 1, current_frame, writer_u);
        computeSsdImage(b1, b2, YUV420, width, height, 2, current_frame, writer_v); 
        */ 
        computeBlockPsnrImage(b1, b2, YUV420, width, height, step, 0, current_frame, writer_y);

        std::cout << "\r\033[k"; // 清空命令行.      
    }

    // 关闭资源
    fclose(f1);
    fclose(f2);
    free(b1);
    free(b2);
    psnr_log_f.close();

    return true;
} 

bool mp42yuv(const std::string &mp4, std::string &yuv) {
    YAML::Node conf = initPsnrConf();
    if (!conf["psnr"] || !conf["psnr"]["ocrSrcDir"] || !conf["psnr"]["resDir"]) {
        std::cout << "解析psnr.yaml文件失败!" << std::endl;
        return false;
    }

    std::string resDir = conf["psnr"]["resDir"].as<std::string>();
    if (!isDirExist(resDir)) {
        mkdir(resDir.c_str(), S_IRWXU);
    }

    std::string file_name = mp4.substr(mp4.find_last_of('/') + 1);
    std::string file_yuv  = resDir + "/" + 
                            file_name.substr(0, file_name.find_last_of('.')) + ".yuv";
    yuv = file_yuv;
    std::cout << "mp4 file: " << mp4 << " <===> yuv file: " << yuv << std::endl;
    FILE *yuv_file = fopen(file_yuv.c_str(), "wb");
    if (!yuv_file)
        return false;

    // 1. register all codecs, demux and protocols
    avdevice_register_all();

    //2. 得到一个ffmpeg的上下文（上下文里面封装了视频的比特率，分辨率等等信息...非常重要）
    AVFormatContext* pContext = avformat_alloc_context();
    if (!pContext) {
        std::cout << "could not allocate context." << std::endl;
        return false;
    }

    //3. 打开视频
    if (avformat_open_input(&pContext, mp4.c_str(), NULL, NULL) < 0) {
        std::cout << "open failed." << std::endl;
        return false;
    }

    //4. 获取视频信息，视频信息封装在上下文中
    if (avformat_find_stream_info(pContext, NULL) < 0) {
        std::cout << "get the information failed." << std::endl;
        return false;
    }

    //5. 用来记住视频流的索引
    AVStream *st = NULL;
    int video_stream_idx = -1;
    //从上下文中寻找找到视频流
    for (int i = 0; i < pContext->nb_streams; ++i) {
        //codec：每一个流 对应的解码上下文
        //codec_type：流的类型
        st = pContext->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    if (video_stream_idx == -1) {
        std::cout << "can not find video stream." << std::endl;
        return false;
    }

    int rotate = getRotateAngle(st);
    std::cout << "...视频转成yuv格式时需要旋转 " << rotate << "度" << std::endl;

    av_dump_format(pContext, 0, mp4.c_str(), 0);

    //6. 获取编码器上下文和编码器
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(NULL);
    if (!pCodecCtx) {
        std::cout << "get codec context failed." << std::endl;
        return false;
    }

    if (avcodec_parameters_to_context(pCodecCtx, pContext->streams[video_stream_idx]->codecpar) < 0) {
        std::cout << "get codec parameters failed." << std::endl;
        return false;
    }

    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    
    //7. 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        std::cout << "decode the video stream failed." << std::endl;
        return false;
    }

    AVPacket *pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(pkt);
    

    int f = -1;
    int f1 = -1;
    int f2 = -1;

    while (true) {
        if (av_read_frame(pContext, pkt) < 0) {
            fclose(yuv_file);
            break;
        }

        if (pkt->stream_index != video_stream_idx) {
            continue;
        }
        
        int got_picture = 0;
        int ret = 0;
        AVFrame *pFrame  = av_frame_alloc();
        AVFrame *pFrameR = av_frame_alloc();

        ret = avcodec_decode_video2(pCodecCtx, pFrameR, &got_picture, pkt);
        ++f1;

        if (ret < 0) {
            break;
        }

        if (rotate == 90) {
            uint8_t* buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1));
            av_image_fill_arrays(pFrame->data, pFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1);
            Rotate90(pFrameR, pFrame);
        } else if (rotate == 270) {
            uint8_t* buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1));
            av_image_fill_arrays(pFrame->data, pFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->height, pCodecCtx->width, 1);
            Rotate270(pFrameR, pFrame);
        } else {
            pFrame = pFrameR;
        }

        ++f2;

        // std::cout << "got_picture: " << got_picture << std::endl;
        if (got_picture) {
            ++f;
            char* buf = new char[pCodecCtx->height * pCodecCtx->width * 3 / 2];
            memset(buf, 0, pCodecCtx->height * pCodecCtx->width * 3 / 2);
            
            int width, height;
            if (rotate == 90 || rotate == 270) {
                height = pCodecCtx->width;
                width  = pCodecCtx->height;
            } else {
                width = pCodecCtx->width;
                height = pCodecCtx->height;
            }

            int a = 0, i;
            for (i = 0; i<height; i++) {
                memcpy(buf + a, pFrame->data[0] + i * pFrame->linesize[0], width);
                a += width;
            }
            for (i = 0; i<height / 2; i++) {
                memcpy(buf + a, pFrame->data[1] + i * pFrame->linesize[1], width / 2);
                a += width / 2;
            }
            for (i = 0; i<height / 2; i++) {
                memcpy(buf + a, pFrame->data[2] + i * pFrame->linesize[2], width / 2);
                a += width / 2;
            }
            fwrite(buf, 1, pCodecCtx->height * pCodecCtx->width * 3 / 2, yuv_file);
            
            delete[] buf;
            buf = NULL;
        }
        av_frame_free(&pFrame);
    }

    // std::cout << "the f is : " << f << ", f1 is : " << f1 << ", f2 is : " << f2 << std::endl;

    fclose(yuv_file);
    avcodec_close(pCodecCtx);
    avformat_free_context(pContext);

    return true;
}

void Rotate90(const AVFrame* src, AVFrame* dst) {
    int half_width   = src->width >> 1;
    int half_height  = src->height >> 1;
    int size         = src->linesize[0] * src->height;
    int half_size    = size >> 2;
    
    for (int j = 0, n = 0; j < src->width; j++) {
        int pos = size;
        for (int i = src->height - 1; i >= 0; i--) {
            pos -= src->linesize[0];
            dst->data[0][n++] = src->data[0][pos + j];
        }
    }
    for (int j = 0, n = 0; j < half_width; j++) {
        int pos = half_size;
        for (int i = half_height - 1; i >= 0; i--) {
            pos -= src->linesize[1];
            dst->data[1][n] = src->data[1][pos + j];
            dst->data[2][n++] = src->data[2][pos + j];
        }
    }

    dst->height = src->width;
    dst->width  = src->height;
}

void Rotate270(const AVFrame* src, AVFrame* dst) {
    int half_width = src->linesize[0] >> 1;
    int half_height = src->height >> 1;

    for (int i = src->width - 1, n = 0; i >= 0; i--) {
        for (int j = 0, pos = 0; j < src->height; j++) {
            dst->data[0][n++] = src->data[0][pos + i];
            pos += src->linesize[0];
        }
    }

    for (int i = (src->width >> 1) - 1, n = 0; i >= 0; i--) {
        for (int j = 0, pos = 0; j < half_height; j++) {
            dst->data[1][n] = src->data[1][pos + i];
            dst->data[2][n++] = src->data[2][pos + i];
            pos += half_width;
        }
    }
    dst->width  = src->height;
    dst->height = src->width;
}

void getDPIFromFile(const std::string &dpi, std::vector<int> &v) {
    std::ifstream in(dpi);  
    if(!in) {
        std::cout <<"no such file" << std::endl;  
    }

    char c;
    v.clear();
    while (in.get(c)) {
        if (c == ' ' || c == '\n') {
            continue;
        }  

        try {
            v.emplace_back(boost::lexical_cast<int>(c));
        } catch(...) {}  
    }

    in.close();  
}