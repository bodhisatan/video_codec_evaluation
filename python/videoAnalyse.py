# -*- coding: utf-8 -*-
#####################################################################
# 该脚本用户分析视频的各种属性，例如码率，帧率，分辨率，音量等
# 分析需要依赖：ffprobe, ffmpeg, httpie工具
# Usage：videoAnalyse.py videoFile douyin $(which ffprobe) $(which ffmpeg) $(which http)"
# videoFile为存放视频资源的文件，每行一个。可以是视频资源的本地路径，也可以是视频资源的url地址。
#####################################################################
import re
import os
import sys
import json
import time
import random
import hashlib
import requests
import platform
from threading import Thread

class VideoAnalyse(object):
	def __init__(self, file, product, ffprobe, ffmpeg, httpie):
		self.ffprobe        = ffprobe   # ffprobe路径
		self.ffmpeg         = ffmpeg    # ffmpeg路径
		self.httpie         = httpie    # httpie路径
		self.file           = file  	# 媒体文件，可以是url
		self.product        = product   # 产品

		self.rawResult      = {}    	# 原始信息
		self.result         = {}		# 分析后结果
		self.videoInfo      = []		# 视频信息
		self.audioInfo      = []        # 音频信息
		self.videoInfoKey   = []        # 需要获取的视频信息key
		self.audioInfoKey   = []        # 需要获取的音频信息key
		self.streamNum      = 0;        # 媒体文件中流的数目
		self.videoStreamNum = 0;        # 视频流数目
		self.audioStreamNum = 0;        # 音频流数目
		self.tmpFile        = ""
		
		self.__initVideoInfoKey()
		self.__initAudioInfoKey()
		self.__initDir()
		self.__initCMD()

	def __initVideoInfoKey(self):
		self.videoInfoKey = ['codec_name',
							 'profile',
							 'level',
							 'codec_time_base',
							 'time_base',
							 'width',
							 'height',
							 'pix_fmt',
							 'r_frame_rate',
							 'avg_frame_rate',
							 'bit_rate',
							 'duration',
							 'color_range',
							 'color_space']

	def __initAudioInfoKey(self):
		self.audioInfoKey = ['codec_name',
							 'profile',
							 'codec_tag_string',
							 'sample_fmt',
							 'sample_rate',
							 'channels',
							 'channel_layout',
							 'bit_rate',
							 'duration']

	def __initDir(self):
		if os.path.exists('tmp') and os.path.isdir('tmp'):
			pass
		else:
			os.mkdir('tmp')

		self.tmpFile = 'tmp/%s' % (self.__getMD5())
	
	def __getMD5(self):
		m2 = hashlib.md5()   
		m2.update(self.file.encode('utf-8'))
		return m2.hexdigest()

	def __initCMD(self):
		if re.match('.*\.mp4$', self.file): # 如果是MP4文件，则不需要http进行跳转分析.
			self.ffmpegCMD = '-y -vn -i %s -af "[0:a] volumedetect" -f null /dev/null > %s 2>&1' % \
		                      (self.file, self.tmpFile)
		else: 
			fileCMD = "%s -h %s | grep Location | cut -d ' ' -f2" % (self.httpie, self.file)
			cmdF    = os.popen(fileCMD)
			res     = cmdF.read()
			res     = res.split('\n')
			location = ""

			for h in res:
				if re.match('Location', h):
					location = h.split(' ')[1]

			if location == "":
				self.ffmpegCMD = '-y -vn -i %s -af "[0:a] volumedetect" -f null /dev/null > %s 2>&1' % \
			                      (self.file, self.tmpFile)
			else:
				self.ffmpegCMD = '-y -vn -i %s -af "[0:a] volumedetect" -f null /dev/null > %s 2>&1' % \
			                      (location, self.tmpFile)

		self.ffprobeCMD    = "-v quiet -show_format -show_streams -of json"                    

	def getResult(self):
		self.__getRawResult()

		if len(self.rawResult) == 0:
			print("The result is {}, pleas check")
			return self.result

		self.__analyseVideoInfo()
		self.__analyseAudioInfo()
		self.__analyseFormat()
		self.__analyseVolume()

		self.result['video'] = self.videoInfo
		self.result['audio'] = self.audioInfo
		self.result['file']  = self.file

		return self.result

	def __getRawResult(self):
		cmd     = "%s %s %s" % (self.ffprobe, self.ffprobeCMD, self.file)
		cmdF    = os.popen(cmd)
		res     = cmdF.read()
		resDict = json.loads(res)
		if 'streams' in resDict and 'format' in resDict:
			self.rawResult = resDict
		else:
			self.rawResult = {}

	def __analyseVideoInfo(self): 
		for stream in self.rawResult['streams']:
			if stream['codec_type'] == 'video':
				self.videoStreamNum += 1
				tmp = {}
				for key in self.videoInfoKey:
					tmp[key] = (stream[key] if (key in stream) else "")

				self.videoInfo.append(tmp)

	def __analyseAudioInfo(self):
		for stream in self.rawResult['streams']:
			if stream['codec_type'] == 'audio':
				self.audioStreamNum += 1
				tmp = {}
				for key in self.audioInfoKey:
					tmp[key] = (stream[key] if (key in stream) else "")

				self.audioInfo.append(tmp)

	def __analyseFormat(self):
		self.streamNum = self.rawResult['format']['nb_streams']
		self.result['format'] = {}
		self.result['format']['nb_streams'] = self.rawResult['format']['nb_streams']

	def __analyseVolume(self):
		cmd = "%s %s" % (self.ffmpeg, self.ffmpegCMD)
		os.system(cmd)

		cmdGrep = ""
		pf  = platform.system()
		if pf == "Darwin":
			cmdGrep = "grep -oE '[^ ]+_volume: *-[0-9.]+' %s" % (self.tmpFile)
		else:
			cmdGrep = "grep -oP '[^ ]+_volume: *-[0-9.]+' %s" % (self.tmpFile)

		cmdF    = os.popen(cmdGrep)
		res     = cmdF.read()
		res     = res.split('\n')

		self.result['volume'] = {}
		for v in res:
			if len(v) == 0:
				continue
			vi = v.split(':')
			if (len(vi)) != 2:
				continue

			self.result['volume'][vi[0]] = vi[1].strip()
			self.result['volume']['unit'] = 'dB'

		os.remove(self.tmpFile)


def getProcessInfo(mediaFile, processNum):
	if processNum < 1:
		processNum = 50
	
	file           = open(mediaFile)
	mediaResources = file.readlines()
	mediaNum       = len(mediaResources)
	mediaNumPerProcess = 1
	mediaResourcesNew  = []

	if mediaNum < processNum :
		mediaNumPerProcess = 1
	else :
		mediaNumPerProcess = int(mediaNum / processNum)

	for i in range(mediaNum):
		mediaResources[i] = mediaResources[i].strip()

	for i in range(0, mediaNum, mediaNumPerProcess):
		mediaResourcesNew.append(mediaResources[i:i+mediaNumPerProcess])

	return mediaResourcesNew


def work(medias, i, product, ffprobe, ffmpeg, httpie):
	print("%d starting...." % (i))
	m = medias[i]

	for media in m:
		time.sleep(1)
		print("[process]:%d [resource]:%s" % (i, media))
		v = VideoAnalyse(media, product, ffprobe, ffmpeg, httpie)
		res = v.getResult()

	print("%d stop...." % (i))


if __name__ == '__main__': 
    print("***      Test VideoAnalyse      ***")
    print("CMD:     videoAnalyse.py videoFile douyin $(which ffprobe) $(which ffmpeg) $(which http)")
    if len(sys.argv) != 6:
    	print("FATAL:   parameters error, please check!")
    	exit(1)
    
    mediaFile = sys.argv[1]
    product   = sys.argv[2]
    ffprobe   = sys.argv[3]
    ffmpeg    = sys.argv[4]
    httpie    = sys.argv[5]

    if not os.path.exists(ffprobe):
    	print("FATAL:   the ffprobe can not find in this machine, please check!")
    	exit(2)

    if not os.path.exists(ffmpeg):
    	print("FATAL:   the ffmpeg can not find in this machine, please check!")
    	exit(3)

    if not os.path.exists(httpie):
    	print("FATAL:   the httpie can not find in this machine, please check!")
    	exit(4)

    ## 确定需要处理的文件大小
    processNum = 20
    medias = getProcessInfo(mediaFile, processNum)
    processNum = len(medias)
    
    ## 创建线程
    process = []
    for i in range(0, processNum):
    	process.append(Thread(target=work, args=(medias, i, product, ffprobe, ffmpeg, httpie)))

    ## 启动线程并执行
    for t in process:
    	t.start()

    ## 等待结果
    for t in process:
    	t.join()

    print("all process done...")
