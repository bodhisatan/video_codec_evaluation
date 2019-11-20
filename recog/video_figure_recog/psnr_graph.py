#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@author: zhangyu57

"""

import matplotlib.pyplot as plt
import os

def psnrFileAnalysis():
    """
    读取psnr文件，识别psnr&mse的值
    Args:
        psnrFile: psnr文件
    Returns:
        None
    """
    psnrFilePath = '/Users/zhangyu57/Downloads/psnr.log'
    if not os.path.exists(psnrFilePath):
        return
    pic_x_list = []
    mse_avg_list = []
    mse_y_list = []
    mse_u_list = []
    mse_v_list = []
    psnr_avg_list = []
    psnr_y_list = []
    psnr_u_list = []
    psnr_v_list = []


    with open(psnrFilePath, 'r') as f:
        for line in f.readlines():
            pic_x = line.split('n:')[1].split(" ")[0]
            pic_x_list.append(pic_x)

            mse_avg = line.split('mse_avg:')[1].split(' ')[0]
            mse_avg_list.append(mse_avg)

            mse_y = line.split('mse_y:')[1].split(' ')[0]
            mse_y_list.append(mse_y)

            mse_u = line.split('mse_u:')[1].split(' ')[0]
            mse_u_list.append(mse_u)

            mse_v = line.split('mse_v:')[1].split(' ')[0]
            mse_v_list.append(mse_v)

            psnr_avg = line.split('psnr_avg:')[1].split(' ')[0]
            psnr_avg_list.append(psnr_avg)

            psnr_y = line.split('psnr_y:')[1].split(' ')[0]
            psnr_y_list.append(psnr_y)

            psnr_u = line.split('psnr_u:')[1].split(' ')[0]
            psnr_u_list.append(psnr_u)

            psnr_v = line.split('psnr_v:')[1].split(' ')[0]
            psnr_v_list.append(psnr_v)

        return pic_x_list ,psnr_avg_list,psnr_y_list,psnr_u_list,psnr_v_list,mse_avg_list,mse_y_list,mse_u_list,mse_v_list


def psnr_graph(pic_x_list,psnr_avg_list,psnr_y_list,psnr_u_list,psnr_v_list):
    """
    每一帧图片及yuv每个通道的psnr值可视化图
    Args:
        pic_x_list,psnr_avg_list,psnr_y_list,psnr_u_list,psnr_v_list
    Returns:
        None
    """
    # 创建绘图对象
    plt.figure(figsize=(10,8))
    plt.plot(pic_x_list, psnr_avg_list, "r",linewidth=1,label='psnr_avg')
    plt.plot(pic_x_list, psnr_y_list, "g",linewidth=1,label='psnr_y')
    plt.plot(pic_x_list, psnr_u_list, "b",linewidth=1,label='psnr_u')
    plt.plot(pic_x_list, psnr_v_list,color='black',linewidth=1,label='psnr_v')

    plt.xlabel("Frame Number")
    plt.ylabel("Value")
    plt.title("psnr graph")
    plt.legend()
    plt.grid(True)
    # plt.show()
    plt.savefig('psnr.png')

def mse_graph(pic_x_list,mse_avg_list,mse_y_list,mse_u_list,mse_v_list):
    """
    每一帧图片及yuv每个通道的mse值可视化图
    Args:
        pic_x_list,mse_avg_list,mse_y_list,mse_u_list,mse_v_list
    Returns:
        None
    """
    # 创建绘图对象
    plt.figure(figsize=(10,8))
    plt.plot(pic_x_list, mse_avg_list, "r",linewidth=1,label='mse_avg')
    plt.plot(pic_x_list, mse_y_list, "g",linewidth=1,label='mse_y')
    plt.plot(pic_x_list, mse_u_list, "b",linewidth=1,label='mse_u')
    plt.plot(pic_x_list, mse_v_list,color='black',linewidth=1,label='mse_v')

    plt.xlabel("Frame Number")
    plt.ylabel("Value")
    plt.title("mse graph")
    plt.legend()
    plt.grid(True)
    # plt.show()
    plt.savefig('mse.png')


if __name__ == '__main__':
    res = psnrFileAnalysis()
    psnr_graph(res[0],res[1],res[2],res[3],res[4])
    mse_graph(res[0], res[5], res[6], res[7], res[8])

