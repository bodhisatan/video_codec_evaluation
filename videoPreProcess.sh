#!/bin/bash
#####################################################################
# brief: 从手机拉取一个视频，然后对该视频执行预处理，再将视频推送到手机
#####################################################################

### 如下是会用到的配置项，使用时需要根据具体的命令进行替换。
ADB='/Users/wangwei/Library/Android/sdk/platform-tools/adb'
FFMPEG='/usr/local/bin/ffmpeg'

if [ $# -ne 1 ]
then
	echo "参数错误，请检查..."
	echo "Usage: $0 /fullpath/videofile"
	exit 1
fi

devices=$($ADB devices -l | grep -v "List of devices attached")
if [ -z "$devices" ]
then
	echo "没有挂载Android设备，请挂载..."
	exit 2
fi

videofile=$1
filename=${videofile##*/}
filePath=${videofile%/*}
output="anchor_${filename}"

fileExist=$($ADB shell ls $videofile)
if [ -z "$fileExist" ]
then
	echo "文件:${fileExist}不存在，请检查..."
	exit 3
fi

if [ -e $filename ]
then
	echo "文件${filename}已经存在，删除..."
	rm -f $filename
	echo "删除文件成功..."
fi

$ADB pull $videofile .
echo "从Android设备复制文件成功..."

echo "开始给视频每一帧打上帧号..."
$FFMPEG -i ${filename} -vf 'drawbox=40:150:160:60:Green:fill, drawtext=fontcolor=white:fontsize=50:fontfile=/Library/Fonts/Arial\ Bold.ttf:text=%{n}:x=50:y=160' -y ${output}
echo "视频打帧号完成, 推送视频到手机..."

$ADB push $output $filePath/$output