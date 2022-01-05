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

#include "fb_common.h"

int main(int argc, char *argv[])
{
    char *devfile = "/dev/fb0";
    struct fb_var_screeninfo vinfo;

    if (argc >= 2)
        devfile = argv[1];

    fb_Tc_Init(devfile, 0);
    fb_Tc_Print_Fix_Info();
    fb_Tc_Print_Var_Info();
    fb_Tc_Get_Var_Info(&vinfo);

    fb_Tc_Fill_Rect(vinfo.xres / 2, vinfo.yres / 2,
            vinfo.xres / 4, vinfo.yres / 4, 0xff9933fa);

    MI_FB_ColorKey_t colorkey;
    colorkey.bKeyEnable = TRUE;
    colorkey.u8Red = 0x99;
    colorkey.u8Green = 0x33;
    colorkey.u8Blue = 0xfa;

    printf("After sleep 5s FBIOSET_COLORKEY enable colorkey the filter color[0x%x,0x%x,0x%x]\n",
            colorkey.u8Red, colorkey.u8Green, colorkey.u8Blue);
    sleep(5);

    fb_Tc_Set_ColorKey(&colorkey);

    fb_Tc_Deinit();

    return 0;
}

