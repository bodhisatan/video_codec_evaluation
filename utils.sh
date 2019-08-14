#!/bin/bash
#####################################################################
# brief: 工具集
#####################################################################
FFMPEG='/usr/local/bin/ffmpeg'
PYTHON='/usr/local/bin/python'
FFPROBE='/usr/local/bin/ffprobe'
ADB='/Users/wangwei/Library/Android/sdk/platform-tools/adb'
BIN='/Users/wangwei/Documents/Project/github/video_codec_evaluation/get_frame_seq'
VIDEO_PATH='/sdcard/相机'
VIDEO_AFTER_PATH='videoAfter'
VIDEO_DB_PATH='videoDB'

###
#@brief: 获取视频的宽，高，帧数.
#
#@param file:string，待分析的文件名
#@echo string "width height nb_frames"
function getVideoInfo() {
	local file=$1
	res=$($FFPROBE -v error -count_frames -select_streams v:0  -show_entries stream=width:stream=height:stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 $file)
	echo $res
}

###
#@brief: 判断是否有设备和电脑连接
#
#@return 0: 没有挂载设备，1：挂载设备 
function isDeviceConnection() {
	local devices=$($ADB devices -l | grep -v "List of devices attached")
	
	if [ -z "$devices" ]
	then
		echo "没有挂载Android设备，请挂载..."
		return 0
	fi

	return 1
}

###
#@brief: 获取设备的屏幕分辨率
#
#@return 0: 成功，1：失败 
#
#@echo string:"width height"
function getDeviceResolution() {
	isDeviceConnection

	if [ $? -ne 1 ]
	then
		return 1
	fi

	local size=$($ADB shell wm size | grep -oE '\d+.\d+' | tr -s 'x' ' ')
	
	echo $size
	return 0
}

###
# 清空设备上的视频的图片，先删除，然后再广播，否则可能会导致删除后文件依然可用
function cleanDevice() {
	isDeviceConnection

	if [ $? -ne 1 ]
	then
		return 1
	fi

	local files=$($ADB shell ls $VIDEO_PATH)
	
	echo "情况设备上所有的视频..."
	for file in $files
	do
		echo "删除${file}"
		$ADB shell rm -f "${VIDEO_PATH}/${file}"
		$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH}/${file}"
	done

	return 0
}

###
# 将文件push到Android设备,每次推送之前先执行清空操作，保证每次只有一个视频。
#
# @param file:string 文件名
function pushVideo2Device() {
	cleanDevice
	if [ $? -ne 0 ]
	then
		return 1
	fi

	local file=$1
	local filename=${file##*/}

	echo "将视频文件${filename}推送到手机相册下"

	$ADB push $file "${VIDEO_PATH}/"
	$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH}/${filename}"
 
	return 0
}

###
# 获取应用处理后，保存到本地的视频，该视频需要拉到本地处理. 
function getFileToBePull() {
	local file=$($ADB shell ls $VIDEO_PATH | grep -v 'anchor_')
	while [ -z "$file" ]
	do
		sleep 2
		file=$($ADB shell ls $VIDEO_PATH | grep -v 'anchor_')
	done

	echo $file
}

### 
# @brief: 将该文件上传到抖音
# 
# @param file:string: 此次处理的视频的文件名 
handleDouYin() {
	local handleFileName=$1
	handleFileName=${handleFileName##*/}

	# 1. 打开抖音
	echo "打开抖音，开始上传视频..."
	$ADB shell am start -n com.ss.android.ugc.aweme/.main.MainActivity

	# 2. 等待5秒，方便打开主界面
	sleep 5

	# 3. 点击+,进入视频拍摄页面
	$ADB shell input tap 540 1840
	sleep 1

	# 4. 点击上传，进入相册选择页面
	$ADB shell input tap 890 1650
	sleep 1

	# 5. 选择视频
	$ADB shell input tap 130 500
	sleep 1

	# 6. 点击<下一步>
	$ADB shell input tap 920 220
	sleep 15

	# 7. 点击<下一步>
	$ADB shell input tap 920 1760
	sleep 15

	# 8. 点击<发布>
	$ADB shell input tap 800 1800
	sleep 5

	# 9. 有同步到头条的选项就点击取消
	$ADB shell input tap 540 1540
	sleep 1

	# 10. 获取文件
	echo "上传视频成功，获取保存到本地的视频..."
	local file=$(getFileToBePull)

	# 11. 将文件拉到指定的目录
	echo "将视频拉取到电脑，准备分析..."
	$ADB pull "${VIDEO_PATH}/${file}" "${VIDEO_AFTER_PATH}/douyin_android_${handleFileName}"
	sleep 1

	# 12. 删除手机的上文件并发送广播通知
	echo "拉取成功后，删除手机上的视频..."
	$ADB shell rm -f "${VIDEO_PATH}/${file}"
	$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH}/${file}"
}

###
# @brief: 分析videoAfter目录下的视频的丢帧情况
function analyseFrameDrop() {
	local files=$(ls $VIDEO_AFTER_PATH)
	local tmp=""
	local originalVideoFile=""
	local videoInfo=""

	echo "开始分析丢帧的情况..."
	for file in $files
	do
		tmp=${file%%.*}
		originalVideoFile="${VIDEO_DB_PATH}/${file##douyin_android_}"
		
		echo $originalVideoFile
		echo $tmp
		
		echo "分析视频信息，得到分辨率和帧数..."
		videoInfo=$(getVideoInfo $originalVideoFile)
		videoInfo=($videoInfo)
		echo $videoInfo
		
		mkdir $tmp
		$BIN -i "$VIDEO_AFTER_PATH/$file" -w ${videoInfo[0]} -h ${videoInfo[1]} -d $tmp
		
		$PYTHON ocr.py $tmp ${videoInfo[2]}
		rm -rf $tmp
	done
}

file='videoDB/anchor_video_20190814_190000.mp4'
pushVideo2Device $file
sleep 2
handleDouYin $file
sleep 2
analyseFrameDrop
sleep 2