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

#define CURSOR_W 50
#define CURSOR_H 50

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

int main(int argc,char** argv)
{
    const char *devfile = "/dev/fb0";
    const char *curfile1 = "cursor_r_50x50_argb1555.raw";
    const char *curfile2 = "cursor_g_50x50_argb1555.raw";
    struct fb_var_screeninfo vinfo;
    MI_FB_CursorAttr_t stCursorAttr;
    unsigned int osdWidth = 1920;
    unsigned int osdHeight = 1080;
    int curfd1 = -1;
    int curfd2 = -1;
    char curbuf1[CURSOR_W * CURSOR_H * 4] = {0};
    char curbuf2[CURSOR_W * CURSOR_H * 4] = {0};

    memset(&stCursorAttr, 0, sizeof(MI_FB_CursorAttr_t));

    curfd1 = open(curfile1, O_RDONLY);
    if (curfd1 == -1)
    {
        printf("cursor file %s not found\n", curfile1);
        return -1;
    }
    curfd2 = open(curfile2, O_RDONLY);
    if (curfd2 == -1)
    {
        printf("cursor file %s not found\n", curfile2);
        return -1;
    }
    read(curfd1, curbuf1, sizeof(curbuf1));
    read(curfd2, curbuf2, sizeof(curbuf2));
    close(curfd1);
    close(curfd2);


    fb_Tc_Init(devfile, 0);

    fb_Tc_Get_Var_Info(&vinfo);
    osdWidth = vinfo.xres;
    osdHeight = vinfo.yres;

    stCursorAttr.stCursorImageInfo.data = curbuf1;
    stCursorAttr.stCursorImageInfo.eColorFmt = E_MI_FB_COLOR_FMT_ARGB1555;
    stCursorAttr.stCursorImageInfo.u32Width  = CURSOR_W;
    stCursorAttr.stCursorImageInfo.u32Height = CURSOR_H;
    stCursorAttr.stCursorImageInfo.u32Pitch  = 50;
    stCursorAttr.bShown = TRUE;
    stCursorAttr.u32HotSpotX = 15;
    stCursorAttr.u32HotSpotY = 6;
    stCursorAttr.u32XPos = 0;
    stCursorAttr.u32YPos = 0;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_ICON | E_MI_FB_CURSOR_ATTR_MASK_SHOW
        | E_MI_FB_CURSOR_ATTR_MASK_POS;

    printf("---> Set icon is red cursor, and show it at (0, 0)\n");
    fb_Tc_Set_CursorAttr(&stCursorAttr);
    Wait(0);

    printf("---> Hide cursor.\n");
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_HIDE;
    fb_Tc_Set_CursorAttr(&stCursorAttr);
    Wait(0);
    printf("---> Show cursor.\n");
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_SHOW;
    fb_Tc_Set_CursorAttr(&stCursorAttr);
    Wait(0);

    printf("---> Disable colorkey to show black block.\n");
    stCursorAttr.stColorKey.bKeyEnable = TRUE;
    stCursorAttr.stColorKey.u8Red = 0xff;
    stCursorAttr.stColorKey.u8Green = 0xff;
    stCursorAttr.stColorKey.u8Blue = 0xff;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_COLORKEY;
    fb_Tc_Set_CursorAttr(&stCursorAttr);
    Wait(0);
    printf("---> Enable colorkey to hide black block.\n");
    stCursorAttr.stColorKey.bKeyEnable = TRUE;
    stCursorAttr.stColorKey.u8Red = 0x00;
    stCursorAttr.stColorKey.u8Green = 0x00;
    stCursorAttr.stColorKey.u8Blue = 0x00;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_COLORKEY;
    fb_Tc_Set_CursorAttr(&stCursorAttr);
    Wait(0);

    printf("---> Cursor will change to green.\n");
    stCursorAttr.stCursorImageInfo.data = curbuf2;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_ICON;
    fb_Tc_Set_CursorAttr(&stCursorAttr);
    Wait(0);

    printf("---> Cursor will move on the border of screen.\n");
    int x, y;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_POS;
    for (x = 0; x < osdWidth; x+=CURSOR_W)
    {
        stCursorAttr.u32XPos = x;
        fb_Tc_Set_CursorAttr(&stCursorAttr);
        usleep(300 * 1000);
    }
    for (y = 0; y < osdHeight; y+=CURSOR_H)
    {
        stCursorAttr.u32YPos = y;
        fb_Tc_Set_CursorAttr(&stCursorAttr);
        usleep(300 * 1000);
    }
    x -= CURSOR_W;
    y -= CURSOR_H;
    for (; x >= 0; x-=CURSOR_W)
    {
        stCursorAttr.u32XPos = x;
        fb_Tc_Set_CursorAttr(&stCursorAttr);
        usleep(300 * 1000);
    }
    for (; y >= 0; y-=CURSOR_H)
    {
        stCursorAttr.u32YPos = y;
        fb_Tc_Set_CursorAttr(&stCursorAttr);
        usleep(300 * 1000);
    }

    Wait(0);

    fb_Tc_Deinit();
}
