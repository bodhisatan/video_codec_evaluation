/*
 * Copyright (c) 2003-2013 Loren Merritt
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110 USA
 */
/*
 * tiny_ssim.c
 * Computes the Structural Similarity Metric between two rawYV12 video files.
 * original algorithm:
 * Z. Wang, A. C. Bovik, H. R. Sheikh and E. P. Simoncelli,
 *   "Image quality assessment: From error visibility to structural similarity,"
 *   IEEE Transactions on Image Processing, vol. 13, no. 4, pp. 600-612, Apr. 2004.
 *
 * To improve speed, this implementation uses the standard approximation of
 * overlapped 8x8 block sums, rather than the original gaussian weights.
 */

/*
 * 上文注释里的重点：
 * 输入格式：两个YV12格式视频
 * 为了提升速度，没有用论文里的高斯卷积核作加权平均，而是用了8x8有重叠的像素块求和的方法，也就是代码里的ssim_4x4x2_core()和ssim_end4()
 */

// #include "config.h"
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define FFSWAP(type, a, b) \
    do                     \
    {                      \
        type SWAP_tmp = b; \
        b = a;             \
        a = SWAP_tmp;      \
    } while (0)
#define FFMIN(a, b) ((a) > (b) ? (b) : (a))

#define BIT_DEPTH 8                      // 位深 使用几位来定位一个像素点 8位的话 像素值范围就是0-255
#define PIXEL_MAX ((1 << BIT_DEPTH) - 1) // 像素值最大值 公式里的L
typedef uint8_t pixel;                   // 8位无符号 表示像素值

const double WEIGHT[] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
const int scale = 5;

typedef struct
{
    double L;
    double C_S;
} ssim_value;

/****************************************************************************
 * structural similarity metric
 ****************************************************************************/
static void ssim_4x4x2_core(const pixel *pix1, intptr_t stride1,
                            const pixel *pix2, intptr_t stride2,
                            int sums[2][4])
{
    // s1 表示 sigma(sigma(a(i, j)))
    // s2 表示 sigma(sigma(b(i, j)))
    // ss 表示 sigma(sigma[a(i, j)^2 + b(i, j)^2])
    // s12 表示 sigma(sigma(a(i, j) * b(i, j)))

    // 函数中计算了一行中两个4x4块，分别将结果存储在sums[0][n]、sums[1][n]中
    int x, y, z;

    for (z = 0; z < 2; z++)
    {
        uint32_t s1 = 0, s2 = 0, ss = 0, s12 = 0;
        for (y = 0; y < 4; y++)
            for (x = 0; x < 4; x++)
            {
                int a = pix1[x + y * stride1];
                int b = pix2[x + y * stride2];
                s1 += a;
                s2 += b;
                ss += a * a;
                ss += b * b;
                s12 += a * b;
            }
        sums[z][0] = s1;
        sums[z][1] = s2;
        sums[z][2] = ss;
        sums[z][3] = s12;
        pix1 += 4;
        pix2 += 4;
    }
}

/*
 * s1：原始像素之和
 * s2：重建像素之和
 * ss：原始像素平方之和+重建像素平方之和
 * s12：原始像素*重建像素的值的和
 */
static ssim_value ssim_end1(int s1, int s2, int ss, int s12)
{
    ssim_value value;
/* Maximum value for 10-bit is: ss*64 = (2^10-1)^2*16*4*64 = 4286582784, which will overflow in some cases.
 * s1*s1, s2*s2, and s1*s2 also obtain this value for edge cases: ((2^10-1)*16*4)^2 = 4286582784.
 * Maximum value for 9-bit is: ss*64 = (2^9-1)^2*16*4*64 = 1069551616, which will not overflow. */
#if BIT_DEPTH > 9
    typedef double type;
    static const double ssim_c1 = .01 * .01 * PIXEL_MAX * PIXEL_MAX * 64;
    static const double ssim_c2 = .03 * .03 * PIXEL_MAX * PIXEL_MAX * 64 * 63;
#else
    typedef int type;
    // k1=0.01, k2=0.03
    static const int ssim_c1 = (int)(.01 * .01 * PIXEL_MAX * PIXEL_MAX * 64 * 64 + .5);
    static const int ssim_c2 = (int)(.03 * .03 * PIXEL_MAX * PIXEL_MAX * 64 * 63 + .5);
#endif
    type fs1 = s1;
    type fs2 = s2;
    type fss = ss;
    type fs12 = s12;
    type vars = fss * 64 - fs1 * fs1 - fs2 * fs2;
    type covar = fs12 * 64 - fs1 * fs2;
    value.L = (double)(2 * fs1 * fs2 + ssim_c1) / (double)(fs1 * fs1 + fs2 * fs2 + ssim_c1);
    value.C_S = (double)(2 * covar + ssim_c2) / (double)(vars + ssim_c2);
    return value;
}

static ssim_value ssim_end4(int sum0[5][4], int sum1[5][4], int width)
{
    ssim_value ssim_v;
    ssim_v.L = 0.0;
    ssim_v.C_S = 0.0;
    int i;
    ssim_value value;
    for (i = 0; i < width; i++)
    {
        value = ssim_end1(sum0[i][0] + sum0[i + 1][0] + sum1[i][0] + sum1[i + 1][0],
                          sum0[i][1] + sum0[i + 1][1] + sum1[i][1] + sum1[i + 1][1],
                          sum0[i][2] + sum0[i + 1][2] + sum1[i][2] + sum1[i + 1][2],
                          sum0[i][3] + sum0[i + 1][3] + sum1[i][3] + sum1[i + 1][3]);
        ssim_v.L += value.L;
        ssim_v.C_S += value.C_S;
    }

    return ssim_v;
}

ssim_value ssim_plane(
    pixel *pix1, intptr_t stride1,
    pixel *pix2, intptr_t stride2,
    int width, int height, void *buf, int *cnt)
{
    int z = 0;
    int x, y;
    ssim_value ssim;
    ssim.L = 0.0;
    ssim.C_S = 0.0;
    /*
     * 按照4x4的块对像素进行处理的。使用sum1保存上一行块的“信息”，sum0保存当前一行块的“信息”
     * sum0是一个数组指针，其中存储了一个4元素数组的地址
     * 换句话说，sum0中每一个元素对应一个4x4块的信息（该信息包含4个元素）。
     *
     * 4个元素中：
     * [0]原始像素之和
     * [1]重建像素之和
     * [2]原始像素平方之和+重建像素平方之和
     * [3]原始像素*重建像素的值的和
     *
     */
    int(*sum0)[4] = (int(*)[4])buf; // 指向长度为4的数组指针
    int(*sum1)[4] = sum0 + (width >> 2) + 3;
    width >>= 2;
    height >>= 2; // 除以4 因为SSIM计算以4*4为基本单位
    for (y = 1; y < height; y++)
    {
        // 下面这个循环，只有在第一次执行的时候执行2次，处理第1行和第2行的块
        // 后面的都只会执行一次
        for (; z <= y; z++)
        {
            // FFSWAP( (int (*)[4]), sum0, sum1 );
            int(*tmp)[4] = sum0;
            sum0 = sum1;
            sum1 = tmp;

            // 获取4x4块的信息(4个值存于长度为4的一维数组)（这里并没有代入公式计算SSIM结果）
            // 结果存储在sum0中。从左到右每个4x4的块依次存储在sum0[0]，sum0[1]，sum0[2]...
            // 每次前进2个块，通过ssim_4x4x2_core()计算2个4x4块,两个4×4有一半重叠部分
            for (x = 0; x < width; x += 2)
                ssim_4x4x2_core(&pix1[4 * (x + z * stride1)], stride1, &pix2[4 * (x + z * stride2)], stride2, &sum0[x]);
        }
        // sum1是储存上一行的信息，sum0是储存本行的信息，ssim_end4是进行2（line）×4×4×2 2行每行2个4×4的块的单元进行处理
        ssim_value tmp_value;
        for (x = 0; x < width - 1; x += 4)
        {
            tmp_value = ssim_end4(sum0 + x, sum1 + x, FFMIN(4, width - x - 1));
            ssim.L += tmp_value.L;
            ssim.C_S += tmp_value.C_S;
        }
    }
    ssim.L /= (height - 1) * (width - 1);
    ssim.C_S /= (height - 1) * (width - 1);
    return ssim;
}

static void print_results(double ms_ssim[3], int frames, int w, int h)
{
    printf("MSSSIM Y:%.5f U:%.5f V:%.5f All:%.5f\n",
           ms_ssim[0] / frames,
           ms_ssim[1] / frames,
           ms_ssim[2] / frames,
           (ms_ssim[0] * 4 + ms_ssim[1] + ms_ssim[2]) / (frames * 6));
}

pixel *down_sample(pixel *input, int height, int width, int scale)
{
    pixel *output = new pixel[height * width / 4];
    int cur = 0;
    for (int i = 0; i < height && cur < height * width / 4; i += 2)
    {
        for (int j = 0; j < width && cur < height * width / 4; j += 2)
        {
            output[cur++] = (input[i * width + j] + input[(i + 1) * width + j] + input[i * width + j + 1] + input[(i + 1) * width + j + 1]) / 4;
        }
    }
    return output;
}

double ms_ssim_plane(pixel *pix1, pixel *pix2, int width, int height)
{
    int *temp;
    ssim_value value;
    double result = 1.0;
    int scale = 0;
    double light_value[5];
    int w = width;
    int h = height;
    pixel *tmp_img1;
    pixel *tmp_img2;

    for (; scale < 5 && pow(2, scale) < width && pow(2, scale) < height; scale++)
    {
        if (scale == 0)
        {
            tmp_img1 = new pixel[width * height];
            tmp_img2 = new pixel[width * height];
            for (int i = 0; i < width * height; i++)
            {
                tmp_img1[i] = pix1[i];
                tmp_img2[i] = pix2[i];
            }
        }
        else
        {
            tmp_img1 = down_sample(tmp_img1, h, w, scale);
            tmp_img2 = down_sample(tmp_img2, h, w, scale);
            h /= 2;
            w /= 2;
        }

        temp = (int *)malloc((2 * w + 12) * sizeof(*temp));
        value = ssim_plane(tmp_img1, w, tmp_img2, w, w, h, temp, NULL);
        result *= pow(value.C_S, WEIGHT[scale]);
        light_value[scale] = value.L;
    }

    result *= pow(light_value[scale - 1], WEIGHT[scale - 1]);
    return result;
}

int main(int argc, char *argv[])
{
    FILE *f[2];
    uint8_t *buf[2], *plane[2][3];
    int *temp;
    double ms_ssim[3] = {0, 0, 0};
    int frame_size, w, h;
    int frames, seek;
    int i;

    // 输入格式
    if (argc < 4 || 2 != sscanf(argv[3], "%dx%d", &w, &h))
    {
        printf("ms-ssim <file1.yuv> <file2.yuv> <width>x<height> [<seek>]\n");
        return -1;
    }

    // 读入两个文件 长x宽
    f[0] = fopen(argv[1], "rb");
    f[1] = fopen(argv[2], "rb");
    sscanf(argv[3], "%dx%d", &w, &h);

    if (w <= 0 || h <= 0 || w * (int64_t)h >= INT_MAX / 3 || 2LL * w + 12 >= INT_MAX / sizeof(*temp))
    {
        fprintf(stderr, "Dimensions are too large, or invalid\n");
        return -2;
    }

    // 一帧的内存大小
    // yuv420格式：先w*h个Y，然后1/4*w*h个U，再然后1/4*w*h个
    frame_size = w * h * 3LL / 2;

    // plane[i][0] Y分量信息
    // plane[i][1] U分量信息
    // plane[i][2] V分量信息
    for (i = 0; i < 2; i++)
    {
        buf[i] = (uint8_t *)malloc(frame_size);
        plane[i][0] = buf[i]; // plane[i][0] = buf[i]
        plane[i][1] = plane[i][0] + w * h;
        plane[i][2] = plane[i][1] + w * h / 4;
    }
    temp = (int *)malloc((2 * w + 12) * sizeof(*temp));
    seek = argc < 5 ? 0 : atoi(argv[4]);
    fseek(f[seek < 0], seek < 0 ? -seek : seek, SEEK_SET);

    // 逐帧计算
    for (frames = 0;; frames++)
    {
        double ms_ssim_one[3]; // Y U V 三个向量一帧ms-ssim的结果
        // 分别读入这一帧Y向量的地址，随之也获得了UV向量的起始地址
        if (fread(buf[0], frame_size, 1, f[0]) != 1)
            break;
        if (fread(buf[1], frame_size, 1, f[1]) != 1)
            break;
        for (int i = 0; i < 3; i++)
        {
            ms_ssim_one[i] = ms_ssim_plane(plane[0][i], plane[1][i], w >> !!i, h >> !!i);
            ms_ssim[i] += ms_ssim_one[i];
        }

        printf("Frame %d | ", frames);
        print_results(ms_ssim_one, 1, w, h);
        printf("                \r");
        fflush(stdout);
    }

    if (!frames)
        return 0;

    printf("Total %d frames | ", frames);
    print_results(ms_ssim, frames, w, h);
    printf("\n");

    return 0;
}