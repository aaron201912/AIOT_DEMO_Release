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
    MI_FB_DisplayLayerAttr_t dispattr;

    fb_Tc_Init("/dev/fb0", 0);

    fb_Tc_Get_DispAttr(&dispattr);
    fb_Tc_Print_Disp_Attr(&dispattr);
    fb_Tc_Fill_Rect(0, 0, dispattr.u32DisplayWidth / 1, dispattr.u32DisplayHeight / 1, 0xff00ffff);    
    fb_Tc_Fill_Rect(0, 0, dispattr.u32DisplayWidth / 2, dispattr.u32DisplayHeight / 2, 0xff00ff00);   
    fb_Tc_Fill_Rect(0, 0, dispattr.u32DisplayWidth / 4, dispattr.u32DisplayHeight / 4, 0xffffff00);
    fb_Tc_Fill_Rect(0, 0, dispattr.u32DisplayWidth / 8, dispattr.u32DisplayHeight / 8, 0xffff0000);

    Wait(0);


    dispattr.u32DisplayWidth = dispattr.u32DisplayWidth / 4;
    dispattr.u32DisplayHeight = dispattr.u32DisplayHeight / 4;
    dispattr.u32SetAttrMask = E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE;

    fb_Tc_Set_DispAttr(&dispattr);
    fb_Tc_Get_DispAttr(&dispattr);
    fb_Tc_Print_Disp_Attr(&dispattr);

    Wait(0);

    dispattr.u32DisplayWidth = dispattr.u32DisplayWidth * 2;
    dispattr.u32DisplayHeight = dispattr.u32DisplayHeight * 2;
    dispattr.u32SetAttrMask = E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE;

    fb_Tc_Set_DispAttr(&dispattr);
    fb_Tc_Get_DispAttr(&dispattr);
    fb_Tc_Print_Disp_Attr(&dispattr);

    fb_Tc_Deinit();
    return 0;
}

