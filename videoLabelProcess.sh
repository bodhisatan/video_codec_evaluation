#!/bin/bash
#####################################################################
# brief: 对给定的视频，在保证ffmpeg转码不会修改视频旋转的前提下给视频每一帧
#        增加对应的帧号。
#####################################################################

### 如下是会用到的配置项，使用时需要根据具体的命令进行替换。
FFMPEG='/usr/local/bin/ffmpeg'
FFPROBE='/usr/local/bin/ffprobe'

if [ $# -ne 1 ]
then
	echo "参数错误，请检查..."
	echo "Usage: $0 /fullpath/videofile"
	exit 1
fi

videofile=$1
filename=${videofile##*/}
output="anchor_${filename}"

video_bitrate=$($FFPROBE -v error -select_streams v:0  -show_entries stream=bit_rate -of default=nokey=1:noprint_wrappers=1 ${videofile})

echo "开始给视频每一帧打上帧号..."
vf="drawbox=40:190:160:60:Green:fill, drawtext=fontcolor=white:fontsize=50:fontfile=/Library/Fonts/Arial\ Bold.ttf:text=%{n}:x=50:y=200"
$FFMPEG -noautorotate -i ${videofile} -metadata:s:v rotate=0 -b:v ${video_bitrate} -vf "${vf}" -y ${output}
echo "给视频增加帧号处理完毕..."