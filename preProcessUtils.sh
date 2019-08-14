#!/bin/bash
#####################################################################
# brief: 对视频进行各种预处理的工具集
#####################################################################

FFMPEG='/usr/local/bin/ffmpeg'

#######################################
# @param filename:string, 待处理的文件名，可以为绝对路径或者当前路径下的文件
# @param output:string, 输出的文件名，可以为绝对路径或者当前路径下的文件
# @return res:int, 0为成功，非0为失败
function drawFrameNumber() {
	if [ $# -ne 2 ]
	then
		echo "参数错误，请检查..."
		return 1
	fi

	local filename = $1
	local output   = $2
	
	$FFMPEG -i ${filename} -vf 'drawbox=40:150:160:60:Green:fill, drawtext=fontcolor=white:fontsize=50:fontfile=/Library/Fonts/Arial\ Bold.ttf:text=%{n}:x=50:y=160' -y ${output}

	return 0
}