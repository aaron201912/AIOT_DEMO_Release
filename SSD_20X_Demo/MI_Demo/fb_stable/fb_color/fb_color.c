/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.
  
  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "fb_common.h"

int main(int argc, char *argv[])
{
    char *devfile = "/dev/fb0";
    if (argc >= 2)
        devfile = argv[1];

    printf("This prog wiil show all color on the screen.\n");

    fb_Tc_Init(devfile, 1);

    struct fb_var_screeninfo vinfo;
    int x, y;
    int i, j, k;
    int color = 0xff000000;
    int width, height;
    int sz;

    fb_Tc_Get_Var_Info(&vinfo);
    width = vinfo.xres;
    height = vinfo.yres;
    sz = width > height ? height / 256 : width / 256;
printf("sz=%d\n",sz);
    for (i = 0; i < 256; ++i)
    {
        fb_Tc_Begin_Batch_Draw();
        for (j = 0; j < 256; ++j)
        {
            for (k = 0; k < 256; ++k)
            {
                fb_Tc_Fill_Rect(x, y, sz, sz, color);
                y += sz;
                color++;
            }
            y = 0;
            x += sz;
        }
        fb_Tc_End_Batch_Draw();
        x = 0;
        y = 0;
        usleep(30 * 1000);
    }
        
    fb_Tc_Deinit();
    return 0;
}
