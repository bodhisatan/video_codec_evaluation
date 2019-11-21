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


 
