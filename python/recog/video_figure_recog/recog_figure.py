#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@author: weijingjing

"""

import cv2
import os

from recog.video_figure_recog import sift


res_path  = "Utils/res"


def check_result(image_path, alose_list, file_lose_list):
    """
    :param image_path: 切割出得数字标签区域保存位置
    :param alose_list: 丢失的帧+ocr识别中被漏掉的帧，设置alose_list的原因是为了减少SIFT对比计算量
    :param file_lose_list: ocr识别中被漏掉的帧
    :return: 真实丢失的帧alose_list - file_lose_list
    """
    for image in file_lose_list:
        image = os.path.join(image_path, image)
        for i in alose_list:
            search_icon = os.path.join(res_path, "%s.png" % str(i))
            try:
                w_h_range = sift.find_sift(cv2.imread(image), cv2.imread(search_icon), threshold=0.8, get_point=True)
            except RuntimeError:
                continue
            if w_h_range:
                alose_list.remove(i)
                break
    return alose_list
