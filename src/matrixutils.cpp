#include "matrixutils.h"

MES rotateMatrix270(MES in, Resolution vr) {
	MES out = {.x = 0, .y= 0};

	out.x = vr.height - 1 - in.y;
	out.y = in.x;

	return out;
}

MES rotateMatrix90(MES in, Resolution vr) {
	MES out = {.x = 0, .y = 0};

	out.x = in.y;
	out.y = vr.width - 1 - in.x;

	return out;
}

cv::Rect rotate90(cv::Rect in, Resolution vr) {
	cv::Rect out = {0, 0, 0, 0};

	out.x      = vr.width - in.y;
	out.y      = in.x;
	out.width  = in.height;
	out.height = in.width;
	out.x      = out.x - out.width; // 修正y坐标，因为旋转90度后，左下角的点变为了左上角。

	return out;
}

cv::Rect rotate270(cv::Rect in, Resolution vr) {
	cv::Rect out = {0, 0, 0, 0};

	out.x      = in.y;
	out.y      = vr.height - in.x;
	out.width  = in.height;
	out.height = in.width;
	out.y      = out.y - out.height;

	return out;
} 

cv::Mat imgRotate(cv::Mat matSrc, float angle, bool direction) {
    float theta    = angle * CV_PI / 180.0;
    int   nRowsSrc = matSrc.rows;
    int   nColsSrc = matSrc.cols;

    // 如果是顺时针旋转
    if (!direction)
        theta = 2 * CV_PI - theta;

    // 全部以逆时针旋转来计算，逆时针旋转矩阵
    float matRotate[3][3] {
        {std::cos(theta), -std::sin(theta), 0},
        {std::sin(theta),  std::cos(theta), 0},
        {0, 0, 1}
    };

    float pt[3][2] {
        {float(0),        float(nRowsSrc)},
        {float(nColsSrc), float(nRowsSrc)},
        {float(nColsSrc), float(0)}
    };

    for (int i = 0; i < 3; i++) {
        float x  = pt[i][0] * matRotate[0][0] + pt[i][1] * matRotate[1][0];
        float y  = pt[i][0] * matRotate[0][1] + pt[i][1] * matRotate[1][1];
        pt[i][0] = x;
        pt[i][1] = y;
    }

    // 计算出旋转后图像的极值点和尺寸
    float fMin_x = std::min(std::min(std::min(pt[0][0], pt[1][0]), pt[2][0]), (float)0.0);
    float fMin_y = std::min(std::min(std::min(pt[0][1], pt[1][1]), pt[2][1]), (float)0.0);
    float fMax_x = std::max(std::max(std::max(pt[0][0], pt[1][0]), pt[2][0]), (float)0.0);
    float fMax_y = std::max(std::max(std::max(pt[0][1], pt[1][1]), pt[2][1]), (float)0.0);
    int   nRows  = cvRound(fMax_y - fMin_y + 0.5) + 1;
    int   nCols  = cvRound(fMax_x - fMin_x + 0.5) + 1;
    int   nMin_x = cvRound(fMin_x + 0.5);
    int   nMin_y = cvRound(fMin_y + 0.5);

    // 拷贝输出图像
    cv::Mat matRet(nRows, nCols, matSrc.type(), cv::Scalar(0));
    for (int j = 0; j < nRows; j++) {
        for (int i = 0; i < nCols; i++) {
            // 计算出输出图像在原图像中的对应点的坐标，然后复制该坐标的灰度值
            // 因为是逆时针转换，所以这里映射到原图像的时候可以看成是，输出图像
            // 到顺时针旋转到原图像的，而顺时针旋转矩阵刚好是逆时针旋转矩阵的转置
            // 同时还要考虑到要把旋转后的图像的左上角移动到坐标原点。
            int x = (i + nMin_x) * matRotate[0][0] + (j + nMin_y) * matRotate[0][1];
            int y = (i + nMin_x) * matRotate[1][0] + (j + nMin_y) * matRotate[1][1];
            if (x >= 0 && x < nColsSrc && y >= 0 && y < nRowsSrc) {
                matRet.at<cv::Vec3b>(j, i) = matSrc.at<cv::Vec3b>(y, x);
            }
        }
    }
    return matRet;
}

EVideoType getRotateAngle(const std::string &mp4) {
    EVideoType ret = CAMERA_OTHERS;

    // 1. register all codecs, demux and protocols
    avdevice_register_all();

    //2. 得到一个ffmpeg的上下文（上下文里面封装了视频的比特率，分辨率等等信息...非常重要）
    AVFormatContext* pContext = avformat_alloc_context();
    if (!pContext) {
        std::cout << "could not allocate context." << std::endl;
        return CAMERA_ERROR;
    }

    //3. 打开视频
    if (avformat_open_input(&pContext, mp4.c_str(), NULL, NULL) < 0) {
        std::cout << "open " << mp4 << " failed, please check "<< mp4 << " in right dir." << std::endl;
        return CAMERA_ERROR;
    }

    //4. 获取视频信息，视频信息封装在上下文中
    if (avformat_find_stream_info(pContext, NULL) < 0) {
        std::cout << "get the information failed." << std::endl;
        return CAMERA_ERROR;
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
        return CAMERA_ERROR;
    }

    int rotate = getRotateAngle(st);
    if (rotate == 90) {
        ret = CAMERA_FACING_BACK;
    } else if (rotate == 270) {
        ret = CAMERA_FACING_FRONT;
    } else {
        ret = CAMERA_OTHERS;
    }

    avformat_free_context(pContext);
    
    return ret;
}

int getRotateAngle(const AVStream *avStream) {
    AVDictionaryEntry *tag = NULL;
    int rotate = -1;
    tag = av_dict_get(avStream->metadata, "rotate", tag, 0);
    if (tag==NULL) {
        rotate = 0;
    } else {
        int angle = atoi(tag->value);
        angle %= 360;
        if (angle == 90) {
            rotate = 90;
        } else if (angle == 180) {
            rotate = 180;
        } else if (angle == 270) {
            rotate = 270;
        } else {
            rotate = 0;
        }
    }

    return rotate;
}