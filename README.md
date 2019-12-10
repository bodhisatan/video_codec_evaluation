# video_codec_evaluation
用于对音视频编解码器进行分析，例如丢帧，卡顿等。

## 编译依赖
该项目依赖如下的项目：

* ffmpeg
* opencv
* boost
* yaml-cpp
* rapidjson
* http_request

其中rapidjson和http_request是头文件，已经包含在该项目中，无需处理，但是其它的依赖需要自行解决。

## videoLabelProcess.sh
在视频的指定区域给视频增加帧号信息。

## vpsnr
与传统的psnr相比，vpsn有如下的优势：

* vpsnr用于计算两个视频之间的psnr，并且对每帧视频的psnr进行空间维度的可视化展示，从而避免psnr的单一的平均结果无法描述视频中的不同区域的折损的情况。
* vpsnr还会考虑手机等移动设备在拍摄视频过程中存在的视频旋转的情况，vpsnr内部会自动对视频进行旋转处理。
* vpsnr还增加了计算视频丢帧信息的功能，避免丢帧对psnr的结果带来的不准确性。如果需要开启丢帧检测，请使用`videoLabelProcess.sh`对视频t1.mp4增加标记生成t2.mp4，然后再对处理之后的视频进行各种转码操作并得到t3.mp4，然后利用vpsnr计算t2.mp4和t3.mpr的psnr。

vpsnr的使用如下所示：
```shell
$ vpsnr
usage: vpsnr --refVideo=string --mainVideo=string [options] ...
options:
  -r, --refVideo           含有噪声视频的参考视频 (string)
  -m, --mainVideo          含有噪声的视频 (string)
  -b, --blockSize          计算分块psnr的块大小 (int [=1])
  -d, --dropFrameDetect    是否执行丢帧检测 (bool [=0])
  -?, --help               print this message

$ vpsnr -r videoDB/t12.mp4 -m videoDB/t13.mp4 -b 4
```

vpsn对每帧视频的psnr的可视化结果如下所示：

![](imgs/vpsnr.gif)

如图所示：psnr越大的区域，灰度值越高；psnr越小的区域，灰度值则越低。通过vpsnr的可视化之后，就能评估出视频的不同区域的psnr的范围。

## check_dropframe
该命令用于检测旋转视频被处理之后的丢帧信息。

如果用手机拍摄的原始视频为t1.mp4，则对于t1的存储数据是存在某个旋转角度的，一般后置摄像头会旋转270度，而前置摄像头会旋转90度。如果需要得到正确的预览视频，则需要对t1.mp4进行一定的旋转，后置摄像头需要旋转90度，而前置摄像头需要旋转270度。

利用`videoLabelProcess.sh`对t1.mp4进行无旋转的预处理，在视频的指定区域增加每一帧的帧号信息，得到视频t2.mp4。

视频处理工具（例如`ffmpeg`，`拍摄器`等）对t2.mp4进行处理得到t3.mp4，会自动进行旋转处理，以得到正确的视频。但是在旋转的过程中也对原始的标签区域进行了旋转，因此需要做对应的处理才能提取到原视频中的帧号信息。利用`check_dropframe`对t3.mp4进行处理，得到帧号数据，并存储在指定目录下。

```shell
$ check_dropframe --help

usage: check_dropframe --input=string --originWidth=int --originHeight=int --originVideoType=int [options] ...
options:
  -i, --input              the video need to process (string)
  -w, --originWidth        the video width before codec operator (int)
  -h, --originHeight       the video height before codec operator (int)
  -t, --originVideoType    the origin video source type, 1: front, 2: back, or 3: normal (int)
  -d, --dir                the path that the frame number area extract from the video (string [=psnr/data])
  -?, --help               print this message

$ check_dropframe -i videoDB/t3.mp4 -w 720 -h 1280 -t 1
```


 
