/*
  Copyright (c) 2019, Wang Wei.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MATRIXUTILS_H_
#define MATRIXUTILS_H_

#include <cmath>
#include <complex>
#include <opencv2/opencv.hpp>

enum EVideoType
{
    CAMERA_FACING_FRONT = 1,
    CAMERA_FACING_BACK,
    OTHERS,    
};

typedef struct MatrixElementSubscript {
  int x;
  int y;
} MES;

typedef struct VideoResolution {
  int width;
  int height;
} Resolution;


// 对分辨率是vr的视频帧旋转270度, 然后计算原视频帧中的像素点{in}在新视频帧中的位置.
MES rotateMatrix270(MES in, Resolution vr);

// 对分辨率是vr的视频帧旋转90度, 然后计算原视频帧中的像素点{in}在新视频帧中的位置.
MES rotateMatrix90(MES in, Resolution vr);

// 对vr旋转90度，然后返回旋转后其中的矩形in的新坐标.
cv::Rect rotate90(cv::Rect in, Resolution vr);

// 对vr旋转270度，然后返回旋转后其中的矩形in的新坐标.
cv::Rect rotate270(cv::Rect in, Resolution vr);

// 对图像matSrc旋转angle角度，direction为旋转方向，true为逆时针旋转，false为顺时针旋转.
cv::Mat imgRotate(cv::Mat matSrc, float angle, bool direction);

#endif // MATRIXUTILS_H_