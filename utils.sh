#!/bin/bash
#####################################################################
# brief: 工具集
#####################################################################
FFMPEG='/usr/local/bin/ffmpeg'
PYTHON='/usr/local/bin/python'
FFPROBE='/usr/local/bin/ffprobe'
ADB='/Users/wangwei/Library/Android/sdk/platform-tools/adb'
BIN='/Users/wangwei/Documents/Project/github/video_codec_evaluation/get_frame_seq'

# 因为并不是所有的应用保存的路径都一致，所以要分别处理
VIDEO_PATH='/sdcard/相机'
VIDEO_PATH_2='/sdcard/DCIM/Camera'

# ADB模拟点击的坐标均基于OWxOH的设备分辨率获取，因此需要针对不同机型进行缩放.
OW=1080
OH=1920

# 视频素材库路径和存放处理后视频的路径
VIDEO_DB_PATH='videoDB'
VIDEO_AFTER_PATH='videoAfter'

. appInfo.sh

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
		echo "...没有挂载Android设备，请挂载..."
		return 0
	fi

	return 1
}

###
###
#@brief: 获取设备相关信息
#
#@echo string "设备信息"
function getDeviceInfo() {
	isDeviceConnection

	if [ $? -ne 1 ]
	then
		echo ""
		return 1
	fi

	local brand=$($ADB -d shell getprop ro.product.brand | tr -s ' ' '_')
	local model=$($ADB -d shell getprop ro.product.model |  tr -s ' ' '_')
	local androidVersion=$($ADB shell getprop ro.build.version.release |  tr -s ' ' '_')
	local sdklevel=$($ADB shell getprop ro.build.version.sdk |  tr -s ' ' '_')

	local deviceInfo=("${brand}" "${model}" "${androidVersion}" "${sdklevel}")
	echo ${deviceInfo[@]}

	return 0
}

###
#@brief: 获取app的版本信息
#
#@param: app_package:string, APP包名
#@echo string "app版本号"
function getAppVersion() {
	isDeviceConnection

	if [ $? -ne 1 ]
	then
		return 1
	fi

	local p=$1
	version=$($ADB shell pm dump "$p" | grep 'versionName' | cut -d'=' -f2)
	echo $version

	return 0
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
	
	echo "...清空设备上${VIDEO_PATH}目录下的所有的视频..."
	for file in $files
	do
		echo "......删除${file}"
		$ADB shell rm -f "${VIDEO_PATH}/${file}"
		$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH}/${file}"
	done

	echo "...清空设备上${VIDEO_PATH_2}目录下的所有的视频..."
	files=$($ADB shell ls $VIDEO_PATH_2)
	for file in $files
	do
		echo "......删除${file}"
		$ADB shell rm -f "${VIDEO_PATH_2}/${file}"
		$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH_2}/${file}"
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

	echo "...将视频文件${filename}推送到手机相册下"

	$ADB push $file "${VIDEO_PATH}/"
	$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH}/${filename}"
 
	return 0
}

###
# @brief: 获取应用处理后，保存到本地的视频，该视频需要拉到本地处理. 
#
# @param path:string, 查找path目录下是否已经生成文件.
#
# @echo file:string, 输出需要处理的文件名
function getFileToBePull() {
	local path=$1
	local file=$($ADB shell ls $path | grep -v 'anchor_')
	while [ -z "$file" ]
	do
		sleep 2
		file=$($ADB shell ls $path | grep -v 'anchor_')
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

	local resolution=$(getDeviceResolution)
	resolution=($resolution)
	local w=${resolution[0]}
	local h=${resolution[1]}
	local rw=$(echo "scale=2; $OW/$w" | bc)
	local rh=$(echo "scale=2; $OH/$h" | bc)
	local x=0
	local y=0

	# 1. 打开抖音
	echo "...打开抖音，开始上传视频..."
	$ADB shell am start -n "$DOUYIN"

	# 2. 等待5秒，方便打开主界面
	sleep 5

	# 3. 点击+,进入视频拍摄页面
	x=$(echo "scale=0; 540/$rw" | bc)
	y=$(echo "scale=0; 1840/$rh" | bc)
	$ADB shell input tap $x $y
	sleep 5

	# 4. 点击上传，进入相册选择页面
	x=$(echo "scale=0; 890/$rw" | bc)
	y=$(echo "scale=0; 1650/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 5. 选择视频
	x=$(echo "scale=0; 130/$rw" | bc)
	y=$(echo "scale=0; 500/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 6. 点击<下一步>
	x=$(echo "scale=0; 920/$rw" | bc)
	y=$(echo "scale=0; 220/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 7. 点击<下一步>
	x=$(echo "scale=0; 920/$rw" | bc)
	y=$(echo "scale=0; 1760/$rh" | bc)
	$ADB shell input tap $x $y
	sleep 5

	# 8. 点击<发布>
	x=$(echo "scale=0; 800/$rw" | bc)
	y=$(echo "scale=0; 1800/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 9. 有同步到头条的选项就点击取消
	x=$(echo "scale=0; 540/$rw" | bc)
	y=$(echo "scale=0; 1540/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 1

	# 10. 获取文件
	echo "......上传视频成功，获取保存到本地的视频..."
	local file=$(getFileToBePull $VIDEO_PATH)

	# 11. 将文件拉到指定的目录
	echo ".........将视频拉取到电脑，准备分析..."
	$ADB pull "${VIDEO_PATH}/${file}" "${VIDEO_AFTER_PATH}/douyin_android_${handleFileName}"
	sleep 1

	# 12. 删除手机的上文件并发送广播通知
	echo "............拉取成功后，删除手机上的视频..."
	$ADB shell rm -f "${VIDEO_PATH}/${file}"
	$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH}/${file}"
}

### 
# @brief: 将该文件上传到抖音
# 
# @param file:string: 此次处理的视频的文件名 
handleMini() {
	local handleFileName=$1
	handleFileName=${handleFileName##*/}

	local resolution=$(getDeviceResolution)
	resolution=($resolution)
	local w=${resolution[0]}
	local h=${resolution[1]}
	local rw=$(echo "scale=2; $OW/$w" | bc)
	local rh=$(echo "scale=2; $OH/$h" | bc)
	local x=0
	local y=0

	# 1. 打开全民小视频
	echo "...打开全民小视频，开始上传视频..."
	$ADB shell am start -n "$MINI"

	# 2. 等待5秒，方便打开主界面
	sleep 5

	# 3. 点击+,进入视频拍摄页面
	x=$(echo "scale=0; 540/$rw" | bc)
	y=$(echo "scale=0; 1840/$rh" | bc)
	$ADB shell input tap $x $y
	sleep 5

	# 4. 点击上传，进入相册选择页面
	x=$(echo "scale=0; 890/$rw" | bc)
	y=$(echo "scale=0; 1650/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 5. 选择视频
	x=$(echo "scale=0; 130/$rw" | bc)
	y=$(echo "scale=0; 500/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 6. 点击<✔️>
	x=$(echo "scale=0; 967/$rw" | bc)
	y=$(echo "scale=0; 1832/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 7. 点击<下一步>
	x=$(echo "scale=0; 680/$rw" | bc)
	y=$(echo "scale=0; 1760/$rh" | bc)
	$ADB shell input tap $x $y 
	sleep 5

	# 8. 点击<发布>
	x=$(echo "scale=0; 680/$rw" | bc)
	y=$(echo "scale=0; 1730/$rh" | bc)
	$ADB shell input tap $x $y
	sleep 5

	# 9. 获取文件
	echo "......上传视频成功，获取保存到本地的视频..."
	local file=$(getFileToBePull $VIDEO_PATH_2)

	# 10. 将文件拉到指定的目录
	echo ".........将视频拉取到电脑，准备分析..."
	$ADB pull "${VIDEO_PATH_2}/${file}" "${VIDEO_AFTER_PATH}/mini_android_${handleFileName}"
	sleep 1

	# 11. 删除手机的上文件并发送广播通知
	echo "............拉取成功后，删除手机上的视频..."
	$ADB shell rm -f "${VIDEO_PATH_2}/${file}"
	$ADB shell am broadcast -a android.intent.action.MEDIA_SCANNER_SCAN_FILE -d "file://${VIDEO_PATH_2}/${file}"
}

###
# @brief: 分析videoAfter目录下的视频的丢帧情况
function analyseFrameDrop() {
	local files=$(ls $VIDEO_AFTER_PATH)
	local tmp=""
	local originalVideoFile=""
	local videoInfo=""

	echo "...开始分析丢帧的情况..."
	for file in $files
	do
		echo "...分析文件${file}...BEGIN........."
		tmp=${file%%.*}
		originalVideoFile="${VIDEO_DB_PATH}/${file##*_android_}"
		
		echo "......$originalVideoFile"
		echo "......$tmp"
		
		echo ".........分析视频信息，得到分辨率和帧数..."
		videoInfo=$(getVideoInfo $originalVideoFile)
		videoInfo=($videoInfo)
		echo ${videoInfo[@]}
		
		mkdir $tmp
		$BIN -i "$VIDEO_AFTER_PATH/$file" -w ${videoInfo[0]} -h ${videoInfo[1]} -d $tmp
		
		$PYTHON ocr.py $tmp ${videoInfo[2]}
		rm -rf $tmp
	done
}

s=$(getAppVersion $MINI_P)
echo $s
s=$(getAppVersion $DOUYIN_P)
echo $s
s=($(getDeviceInfo))
#echo $s
#s=(s)
echo ${#s[@]}
exit
rm -rf $VIDEO_AFTER_PATH/*
file='videoDB/anchor_video_20190814_190000.mp4'
pushVideo2Device $file
sleep 2
handleDouYin $file
handleMini $file
sleep 2
analyseFrameDrop
sleep 2