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

void Wait(int val)
{
    if (val <= 0)
    {
        printf("Press 'Enter' to continue...");
        getchar();
    }
    else
    {
        printf("Action will continue after %ds\n", val);
        sleep(val);
    }
}

int main(int argc, char *argv[])
{
    char *devfile = "/dev/fb0";

    if (argc >= 2)
        devfile = argv[1];

    struct fb_var_screeninfo vinfo;
    int wait_time = 0;

    printf("--> Start\n");
    Wait(wait_time);

    fb_Tc_Init(devfile, 0);
    fb_Tc_Print_Fix_Info();
    fb_Tc_Print_Var_Info();
    fb_Tc_Get_Var_Info(&vinfo);

    printf("--> Three rectangle will show on center of screen.\n");
    //draw red rectangle
    fb_Tc_Fill_Rect(vinfo.xres / 8, vinfo.yres / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xf1ff0000);
    //draw DarkGoldenrod rectangle
    fb_Tc_Fill_Rect(vinfo.xres * 3 / 8, vinfo.yres * 3 / 8,
                vinfo.xres / 4, vinfo.yres / 4, 0x80b8860b);
    //draw blue rectanble
    fb_Tc_Fill_Rect(vinfo.xres * 5 / 8, vinfo.yres * 5 / 8,
             vinfo.xres / 4, vinfo.yres / 4, 0xff0000ff);

    Wait(wait_time);

    // Test Show
    printf("--> Three rectanble will hide.\n");
    MI_BOOL bShow;
    fb_Tc_Get_Show(&bShow);
    bShow = FALSE;
    fb_Tc_Set_Show(&bShow);
    Wait(wait_time);

    printf("--> Three rectanble will show.\n");
    bShow = TRUE;
    fb_Tc_Set_Show(&bShow);
    Wait(wait_time);

    // Test screen location
    printf("--> Screen lacation and size will change.\n");
    MI_FB_Rectangle_t rect;
    fb_Tc_Get_ScreenLocation(&rect);
    fb_Tc_Print_Rect(&rect);
    rect.u16Xpos = 100;
    rect.u16Ypos = 100;
    rect.u16Width = vinfo.xres;
    rect.u16Height = vinfo.yres;
    fb_Tc_Print_Rect(&rect);
    fb_Tc_Set_ScreenLocation(&rect);
    Wait(wait_time);

    // Test colorkey
    printf("--> The center rectangle will hide..\n");
    MI_FB_ColorKey_t key;
    fb_Tc_Get_ColorKey(&key);
    fb_Tc_Print_Color_Key(&key);
    key.bKeyEnable = TRUE;
    key.u8Red = 0xb8;
    key.u8Green = 0x86;
    key.u8Blue = 0x0b;
    fb_Tc_Print_Color_Key(&key);
    fb_Tc_Set_ColorKey(&key);
    Wait(wait_time);

    // Test Pixel Alpha
    printf("--> Pixel alpha mode, the left top rectangle will show alpha value.\n");
    MI_FB_GlobalAlpha_t alpha;
    fb_Tc_Get_GlobalAlpha(&alpha);
    printf("%d, %d, %d, %d, %d\n", alpha.bAlphaEnable, alpha.bAlphaChannel, alpha.u8Alpha0, alpha.u8Alpha1, alpha.u8GlobalAlpha);
    alpha.bAlphaEnable = TRUE;
    alpha.bAlphaChannel = TRUE;
    alpha.u8Alpha1 = 0x80;
    fb_Tc_Set_GlobalAlpha(&alpha);
    Wait(wait_time);

    // Test Global Alpha
    printf("--> Global alpha mode, all rectangle will show same alpha value.\n");
    alpha.bAlphaEnable = TRUE;
    alpha.bAlphaChannel = FALSE;
    alpha.u8GlobalAlpha = 0xff;
    fb_Tc_Set_GlobalAlpha(&alpha);
    Wait(wait_time);
    
    fb_Tc_Deinit();
    printf("--> End\n");
    return 0;
}

