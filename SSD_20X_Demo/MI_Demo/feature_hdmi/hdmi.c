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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

#include "iniparser.h"
#include "dictionary.h"
#include "strlib.h"

#include "mi_sys.h"
#include "mi_hdmi.h"
#include "mi_disp.h"
#include "mi_ai.h"
#include "mi_ao.h"

#define INIFILE_PATH "default_hdmi_ut.ini"
#include <sys/mman.h>
#include <linux/fb.h>
#include <stdio.h>
#include "mstarFb.h"
#include <sys/ioctl.h>
void fb_black_lcd(int x, int y, int width, int height,int color);
#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

#define DISP_INPUT_PORT_MAX 16
#define DISP_LAYER_MAX 2
#define DISP_DEV_MAX 2

typedef struct _arg {
    MI_BOOL Inited;
    MI_BOOL hdmi_enable;
    MI_BOOL audio_enable;
    MI_BOOL disp_enable;
} Arg_t;

typedef struct _HdmiAttr {
    MI_BOOL bDevEnable;
    MI_HDMI_DeviceId_e eHdmi;
    MI_HDMI_Attr_t stAttr;
} HdmiAttr_t;

typedef struct stDispUtPort_s
{
    MI_BOOL bPortEnable;
    MI_DISP_LAYER DispLayer;
    MI_U32 u32PortId;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_SYS_ChnPort_t stSysChnPort;
    char FilePath[50];
    pthread_t task;
} StDispUtPort_t;

typedef struct _DispstAttr {
    MI_BOOL bDevEnable;
    MI_DISP_DEV DispDev;
    MI_DISP_LAYER DispLayer;
    MI_U8 u8PortNum;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    StDispUtPort_t stInPortAttr;
} DispstAttr_t;

typedef struct _AudioAttr {
    MI_BOOL bDevEnable;
    MI_AUDIO_DEV aoDevId;
    MI_AO_CHN aoChn;
    MI_AUDIO_Attr_t stAoAttr;
    char FilePath[50];
    pthread_t task;
} AudioAttr_t;

typedef struct DevAttr {
    DispstAttr_t stDisp;
    AudioAttr_t stAud;
    HdmiAttr_t stHdmi;
    Arg_t stEnable;
} DevAttr_t;

typedef struct stTimingArray_s
{
    char desc[50];
    MI_DISP_OutputTiming_e eOutputTiming;
    MI_HDMI_TimingType_e eHdmiTiming;
    MI_U16 u16Width;
    MI_U16 u16Height;
} TimingArray_t;

TimingArray_t astTimingArray[] = {
    {
        .desc = "user",
        .eOutputTiming = E_MI_DISP_OUTPUT_USER,
        .eHdmiTiming = E_MI_HDMI_TIMING_MAX,
    },
    {
        .desc = "480p60",
        .eOutputTiming = E_MI_DISP_OUTPUT_480P60,
        .eHdmiTiming = E_MI_HDMI_TIMING_480_60P,
        .u16Width = 720,.u16Height = 480
    },
    {
        .desc = "576p50",
        .eOutputTiming = E_MI_DISP_OUTPUT_576P50,
        .eHdmiTiming = E_MI_HDMI_TIMING_576_50P,
        .u16Width = 720,.u16Height = 576
    },
    {
        .desc = "720p50",
        .eOutputTiming = E_MI_DISP_OUTPUT_720P50,
        .eHdmiTiming = E_MI_HDMI_TIMING_720_50P,
            .u16Width = 1280,.u16Height = 720
    },
    {
        .desc = "720p60",
        .eOutputTiming = E_MI_DISP_OUTPUT_720P60,
        .eHdmiTiming = E_MI_HDMI_TIMING_720_60P,
            .u16Width = 1280,.u16Height = 720
    },
    {
        .desc = "1024x768_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1024x768_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1024x768_60P,
            .u16Width = 1024,.u16Height = 768
    },
    {
        .desc = "1080p24",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P24,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_24P,
            .u16Width = 1920,.u16Height = 1080
    },
    {
        .desc = "1080p25",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P25,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_25P,
            .u16Width = 1920,.u16Height = 1080
    },
    {
        .desc = "1080p30",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P30,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_30P,
            .u16Width = 1920,.u16Height = 1080
    },
    {
        .desc = "1080p50",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P50,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_50P,
            .u16Width = 1920,.u16Height = 1080
    },
    {
        .desc = "1080p60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1080_60P,
            .u16Width = 1920,.u16Height = 1080
    },
    {
        .desc = "1280x800_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1280x800_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1280x800_60P,
            .u16Width = 1280,.u16Height = 800
    },
    {
        .desc = "1280x1024_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1280x1024_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1280x1024_60P,
            .u16Width = 1280,.u16Height = 1024
    },
    {
        .desc = "1366x768_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1366x768_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1366x768_60P,
            .u16Width = 1366,.u16Height = 768
    },
    {
        .desc = "1440x900_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1440x900_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1440x900_60P,
            .u16Width = 1440,.u16Height = 900
    },
    {
        .desc = "1680x1050_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1680x1050_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1680x1050_60P,
            .u16Width = 1680,.u16Height = 1050
    },
    {
        .desc = "1600x1200_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1600x1200_60,
        .eHdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P,
            .u16Width = 1600,.u16Height = 1200
    },
    {
        .desc = "2560x1440_30",
        .eOutputTiming = E_MI_DISP_OUTPUT_2560x1440_30,
         .eHdmiTiming = E_MI_HDMI_TIMING_1440_30P,
            .u16Width = 2560,.u16Height = 1440
    },
    {
        .desc = "3840x2160_30",
        .eOutputTiming = E_MI_DISP_OUTPUT_3840x2160_30,
        .eHdmiTiming = E_MI_HDMI_TIMING_4K2K_30P,
            .u16Width = 3840,.u16Height = 2160
    },
};

static const TimingArray_t *_MI_HDMI_GetTimingMapFromVic(MI_HDMI_TimingType_e E_MI_HDMI_TIMING)
{
    MI_U32 u32u32Index = 0, U32TableSize = sizeof(astTimingArray)/sizeof(TimingArray_t);

    for(u32u32Index = 0; u32u32Index < U32TableSize;u32u32Index++)
    {
        if (astTimingArray[u32u32Index].eHdmiTiming == E_MI_HDMI_TIMING)
            return &astTimingArray[u32u32Index];
    }
    return &astTimingArray[U32TableSize-1];
}

#define ExecFunc(_func_, _ret_) \
    if (_func_ != _ret_)\
    {\
        printf("Test [%d] exec function failed\n", __LINE__);\
        return 1;\
    }\
    else\
    {\
        printf("Test [%d] exec function pass\n", __LINE__);\
    }

MI_S32 hdmi_stop();
MI_S32 disp_stop();
MI_S32 audio_stop();

MI_BOOL g_exit_app = false;

static MI_S32 Hdmi_callback_impl(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            printf("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            printf("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            printf("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}

static MI_HDMI_TimingType_e Input_Support_Timing()
{
    MI_U8 input[100];
    MI_S32 index = 0, choice;

    printf("Support Timing List:");
    for (index = 0; index < sizeof(astTimingArray)/sizeof(TimingArray_t);index++) 
        printf("index %d: %s \n", index, astTimingArray[index].desc);

    fgets(input,sizeof(input),stdin);
    choice = strtoul(input,NULL,10);

    printf("Input index :%d \n", choice);

    if (choice < sizeof(astTimingArray)/sizeof(TimingArray_t))
        return astTimingArray[choice].eHdmiTiming;
    else
        return E_MI_HDMI_TIMING_MAX;
}

static MI_S32 HdmiTim_to_Format(MI_HDMI_TimingType_e timing, MI_S32 *width, MI_S32 *height)
{
    assert(height);
    assert(width);

    TimingArray_t *Array = NULL;
    MI_S32 i, count = sizeof(astTimingArray)/sizeof(TimingArray_t);
    for(i = 0; i < count; i++)
    {
        Array = &astTimingArray[i];
        if (timing == Array->eHdmiTiming)
        {
            *width = Array->u16Width;
            *height = Array->u16Height;
            break;
        }
    }

    if(i == count)
    {
       printf("Hdmi No Support Such Format：%d\n", timing);
       return -EINVAL;
    }
    return 0;

}

static MI_AUDIO_BitWidth_e HdmiBitDepth_to_AudioBitwidth(MI_HDMI_BitDepth_e eBitDepth)
{
    switch(eBitDepth)
    {
    case E_MI_HDMI_BIT_DEPTH_16:
        return E_MI_AUDIO_BIT_WIDTH_16;
    case E_MI_HDMI_BIT_DEPTH_24:
        return E_MI_AUDIO_BIT_WIDTH_24;
    default:
        printf("Audio No Support Such BitDepth: %d\n", eBitDepth);
        return E_MI_AUDIO_BIT_WIDTH_MAX;
    }
}

static MI_AUDIO_SampleRate_e HdmiSampleRate_to_AudioSampleRate(MI_HDMI_SampleRate_e eRate)
{
   switch(eRate)
   {
   case  E_MI_HDMI_AUDIO_SAMPLERATE_32K:
       return E_MI_AUDIO_SAMPLE_RATE_32000;
   case E_MI_HDMI_AUDIO_SAMPLERATE_48K:
       return E_MI_AUDIO_SAMPLE_RATE_48000;
   default:
       printf("Audio No Support Such Sample Rate: %d\n", eRate);
       return E_MI_HDMI_AUDIO_SAMPLERATE_UNKNOWN;
   }
}

static MI_DISP_OutputTiming_e HdmiTim_to_DispTim(MI_HDMI_TimingType_e eTimingType)
{
    TimingArray_t *Array = NULL;
    MI_S32 i, count = sizeof(astTimingArray)/sizeof(TimingArray_t);
    for(i = 0; i < count; i++)
    {
        Array = &astTimingArray[i];
        if (eTimingType == Array->eHdmiTiming)
        {
            return Array->eOutputTiming;
        }
    }

    return E_MI_DISP_OUTPUT_MAX;
}

MI_S32 hdmi_start(DevAttr_t *pstAttr)
{
    HdmiAttr_t *pstHdmiAttr = &pstAttr->stHdmi;

    if(!pstAttr->stEnable.hdmi_enable || pstHdmiAttr->bDevEnable)
        return 0;

    printf("\nEnter Hdmi Start\n");

    MI_HDMI_DeviceId_e eHdmi = pstHdmiAttr->eHdmi;
    MI_HDMI_Attr_t stAttr = pstHdmiAttr->stAttr;
    MI_HDMI_InitParam_t stInitParam;

    /* Base Module Init */
    stInitParam.pCallBackArgs = NULL;
    stInitParam.pfnHdmiEventCallback = Hdmi_callback_impl;

    ExecFunc(MI_HDMI_Init(&stInitParam), MI_SUCCESS);
    ExecFunc(MI_HDMI_Open(eHdmi), MI_SUCCESS);

    ExecFunc(MI_HDMI_SetAttr(eHdmi, &stAttr), MI_SUCCESS);
    //jackson 测试csc by pass   YUV---->CSC (RGB)--->mix---->disp out---->HDMI--->monitor
 /*   if (1){
         MI_HDMI_Attr_t stAttr2;
         ExecFunc(MI_HDMI_GetAttr(eHdmi, &stAttr2), MI_SUCCESS);
         printf("jacksonA:stAttr2.stVideoAttr.eInColorType %d\n",stAttr2.stVideoAttr.eInColorType);
         stAttr2.stVideoAttr.eInColorType = E_MI_HDMI_COLOR_TYPE_YCBCR444;
         printf("jacksonB:stAttr2.stVideoAttr.eInColorType %d\n",stAttr2.stVideoAttr.eInColorType);
         ExecFunc(MI_HDMI_SetAttr(eHdmi, &stAttr2), MI_SUCCESS);
    }
    */
    ExecFunc(MI_HDMI_Start(eHdmi), MI_SUCCESS);
    //ExecFunc(MI_HDMI_SetAvMute(eHdmi, true), MI_SUCCESS);// first no audio
    ExecFunc(MI_HDMI_SetAvMute(eHdmi, false), MI_SUCCESS);

    pstHdmiAttr->bDevEnable = true;
    return MI_SUCCESS;
}

MI_S32 hdmi_stop(DevAttr_t *pstAttr)
{
    HdmiAttr_t *pstHdmiAttr = &pstAttr->stHdmi;

    if (!pstAttr->stEnable.hdmi_enable || !pstHdmiAttr->bDevEnable)
       return 0;

    printf("\nEnter Hdmi Stop\n");

    MI_HDMI_DeviceId_e eHdmi = pstHdmiAttr->eHdmi;

    //ExecFunc(MI_HDMI_SetAvMute(eHdmi, true), MI_SUCCESS);//jackson
    ExecFunc(MI_HDMI_Stop(eHdmi), MI_SUCCESS);

    pstHdmiAttr->bDevEnable = false;
    return 0;
}

static MI_BOOL disp_GetInputFrameDataYuv(MI_S32 srcFd, MI_SYS_BufInfo_t* pstBufInfo)
{
    MI_BOOL bRet = false;
    MI_U32 u32ReadSize = 0;
    MI_U32 u32LineNum = 0;
    MI_U32 u32BytesPerLine = 0;
    MI_U32 u32Index = 0;
    MI_U32 u32FrameDataSize = 0;
    MI_U32 u32YSize = 0;
    MI_U32 u32UVSize = 0;

    if(E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == pstBufInfo->stFrameData.ePixelFormat)
    {
        u32LineNum = pstBufInfo->stFrameData.u16Height;
        u32BytesPerLine = pstBufInfo->stFrameData.u16Width * 2;
        u32FrameDataSize = u32BytesPerLine * u32LineNum;

        for (u32Index = 0; u32Index < u32LineNum; u32Index ++)
        {
            u32ReadSize += read(srcFd, pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], u32BytesPerLine);
        }
        if(u32ReadSize == u32FrameDataSize)
        {
            bRet = true;
        }
        else if(u32ReadSize < u32FrameDataSize)
        {
            lseek(srcFd, 0, SEEK_SET);
            u32ReadSize = 0;

            for (u32Index = 0; u32Index < u32LineNum; u32Index ++)
            {
                u32ReadSize += read(srcFd, pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], u32BytesPerLine);
            }

            if(u32ReadSize == u32FrameDataSize)
            {
                bRet = true;
            }
            else
            {
                printf("read file error. u32ReadSize = %u. \n", u32ReadSize);
                bRet = false;
            }
        }

    }
    else if(E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == pstBufInfo->stFrameData.ePixelFormat)
    {
        u32LineNum = pstBufInfo->stFrameData.u16Height * 3 / 2;
        u32BytesPerLine = pstBufInfo->stFrameData.u16Width;
        u32FrameDataSize = u32BytesPerLine * u32LineNum;
        for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height; u32Index ++)
        {
            u32YSize += read(srcFd, pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], u32BytesPerLine);
        }

        for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height / 2; u32Index ++)
        {
            u32UVSize += read(srcFd, pstBufInfo->stFrameData.pVirAddr[1] + u32Index * pstBufInfo->stFrameData.u32Stride[1], u32BytesPerLine);
        }

        if(u32YSize + u32UVSize == u32FrameDataSize)
        {
            bRet = true;
        }
        else if(u32YSize + u32UVSize < u32FrameDataSize)
        {
            lseek(srcFd, 0, SEEK_SET);
            u32YSize = 0;
            u32UVSize = 0;

            for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height; u32Index ++)
            {
                u32YSize += read(srcFd, pstBufInfo->stFrameData.pVirAddr[0] + u32Index * pstBufInfo->stFrameData.u32Stride[0], u32BytesPerLine);
            }

            for (u32Index = 0; u32Index < pstBufInfo->stFrameData.u16Height / 2; u32Index ++)
            {
                u32UVSize += read(srcFd, pstBufInfo->stFrameData.pVirAddr[1] + u32Index * pstBufInfo->stFrameData.u32Stride[1], u32BytesPerLine);
            }

            if(u32YSize + u32UVSize == u32FrameDataSize)
            {
                bRet = true;
            }
            else
            {
                printf("read file error. u32YSize = %u, u32UVSize = %u. \n", u32YSize, u32UVSize);
                bRet = false;
            }
        }
        else
        {
            bRet = false;
            printf("[%s][%d][y_size:%d][uv_size:%d][frame_size:%d]\n",__func__, __LINE__, u32YSize, u32UVSize, u32FrameDataSize);
        }
    }

    return bRet;
}

static void *disp_PutBuffer (void *pData)
{
    MI_S32 srcFd = 0;
    MI_DISP_LAYER DispLayer = 0;
    MI_U8 u8PortId = 0;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BufConf_t stBufConf;
    MI_U32 u32BuffSize;
    MI_SYS_BUF_HANDLE hHandle;
    struct timeval stTv;
    struct timeval stGetBuffer, stReadFile, stFlushData, stPutBuffer, stRelease;
    time_t stTime = 0;

    DevAttr_t * pstAttr = (DevAttr_t*)pData;
    StDispUtPort_t * pstInPortAttr = &pstAttr->stDisp.stInPortAttr;
    MI_SYS_ChnPort_t *pstSysChnPort = &pstInPortAttr->stSysChnPort;
    MI_DISP_InputPortAttr_t *pstInputPortAttr = &pstInPortAttr->stInputPortAttr;
    MI_SYS_PixelFormat_e ePixelFormat = pstAttr->stDisp.stLayerAttr.ePixFormat;

    u8PortId = pstInPortAttr->u32PortId;
    DispLayer = pstInPortAttr->DispLayer;

    const char *filePath = pstInPortAttr->FilePath;

    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));
    memset(&stTv, 0, sizeof(stTv));

    srcFd = open(filePath, O_RDONLY);
    if (srcFd < 0)
    {
        printf("src_file: %s.\n", filePath);
        perror("open");
        return NULL;
    }

    while(pstInPortAttr->bPortEnable)
    {
        gettimeofday(&stTv, NULL);
        stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.u64TargetPts = stTv.tv_sec*1000000 + stTv.tv_usec;
        stBufConf.stFrameCfg.u16Width = pstInPortAttr->stInputPortAttr.u16SrcWidth;
        stBufConf.stFrameCfg.u16Height = pstInPortAttr->stInputPortAttr.u16SrcHeight;
        stBufConf.stFrameCfg.eFormat = ePixelFormat;
        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        gettimeofday(&stGetBuffer, NULL);

        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(pstSysChnPort,&stBufConf,&stBufInfo,&hHandle, -1))
        {
            stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
            stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
            stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
            stBufInfo.bEndOfStream = false;

            u32BuffSize = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            gettimeofday(&stReadFile, NULL);
            if(disp_GetInputFrameDataYuv(srcFd, &stBufInfo) == true)
            {
                MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , false);
            }
            else
            {
                printf("disp_test getframe failed\n");
                MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , true);
            }
        }
        else
        {
            printf("disp_test sys get buf fail\n");
        }
        usleep(200*1000);
    }
    close(srcFd);

    return MI_DISP_SUCCESS;
}

MI_S32 disp_start(DevAttr_t *pstAttr)
{
    DispstAttr_t *pstDispstAttr = &pstAttr->stDisp;

    if(!pstAttr->stEnable.disp_enable || pstDispstAttr->bDevEnable)
        return 0;

    MI_U8 u8PortNum = pstDispstAttr->u8PortNum, i;
    MI_DISP_DEV DispDev = pstDispstAttr->DispDev;
    MI_DISP_LAYER DispLayer = pstDispstAttr->DispLayer;
    MI_DISP_PubAttr_t stPubAttr = pstDispstAttr->stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr = pstDispstAttr->stLayerAttr;
    MI_SYS_ChnPort_t stSysChnPort;

    printf("\nEnter Disp Start\n");

    /* Set Disp Pub */
    if(0)//get PubAttr 看参数是否有发生变化
    {MI_DISP_PubAttr_t mypubattr;
        MI_DISP_GetPubAttr (DispDev, &mypubattr);
        printf("u32BgColor:0x%x\n",mypubattr.u32BgColor);
        printf("eIntfType:0x%x\n",mypubattr.eIntfType);
        printf("eIntfSync:0x%x\n",mypubattr.eIntfSync);
    }
    ExecFunc(MI_DISP_SetPubAttr(DispDev,  &stPubAttr), MI_SUCCESS);
    ExecFunc(MI_DISP_Enable(DispDev), MI_SUCCESS);//假如开机show logo,那么就要求再运行app之前,先执行 echo 1 > /sys/class/mstar/mdisp/bootlogo
    //getchar();
    //printf("\nfb_black_lcd\n");
    //fb_black_lcd(0,0,800,480,0xFF0A0A0A); //这里可以填充UI,盖住input的video数据显示.

    /* Disp Layer Releated */
    MI_DISP_BindVideoLayer(DispLayer,DispDev);
    MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr);
    MI_DISP_EnableVideoLayer(DispLayer);
    
 //jackson 测试csc by pass   YUV---->CSC (RGB)--->mix---->disp out---->HDMI--->monitor   
//jackson 这段代码测试bypass csc
/*if(1){
    MI_DISP_HdmiParam_t stHdmiParam;
    MI_DISP_GetHdmiParam(DispDev,&stHdmiParam);
    stHdmiParam.stCsc.eCscMatrix = E_MI_DISP_CSC_MATRIX_BYPASS;
    MI_DISP_SetHdmiParam(DispDev,&stHdmiParam);
}
*/
printf("goto set input port\n");
//getchar();
    /* Set Input Port */
    {
        MI_DISP_InputPortAttr_t *stInputPortAttr = &pstDispstAttr->stInPortAttr.stInputPortAttr;
        //MI_DISP_SetInputPortAttr(DispLayer, 0, stInputPortAttr);
        MI_DISP_EnableInputPort(DispLayer, 0);
        //getchar();
        MI_DISP_ClearInputPortBuffer(DispLayer,0,1);//清空input 缓存数据.(显示黑色)
        //getchar();
        //-----------------------------------------------------
        
        MI_DISP_DisableInputPort(DispLayer, 0);//先disable
        MI_DISP_SetInputPortAttr(DispLayer, 0, stInputPortAttr);//如果input是size发生变化,会造成画面显示异常
        MI_DISP_EnableInputPort(DispLayer, 0);//再enable
        
        //-----------------------------------------------------
        MI_DISP_SetInputPortSyncMode(DispLayer, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
        pstDispstAttr->stInPortAttr.u32PortId = 0;
        pstDispstAttr->stInPortAttr.bPortEnable = true;
    }
//getchar();
    //create put buff thread
    {
        StDispUtPort_t * pstInPortAttr = &pstDispstAttr->stInPortAttr;

        stSysChnPort.eModId = E_MI_MODULE_ID_DISP;
        stSysChnPort.u32DevId = DispDev;
        stSysChnPort.u32ChnId = 0;
        stSysChnPort.u32PortId = 0;
        pstInPortAttr->stSysChnPort = stSysChnPort;

        pthread_create(&pstInPortAttr->task, NULL, disp_PutBuffer, pstAttr);
    }
    pstDispstAttr->bDevEnable= true;
    //getchar();
    return 0;
}

MI_S32 disp_stop(DevAttr_t *pstAttr)
{
    DispstAttr_t *pstDispstAttr = &pstAttr->stDisp;

    if(!pstAttr->stEnable.disp_enable || !pstDispstAttr->bDevEnable)
       return 0;

    MI_S32 i;
    MI_DISP_DEV DispDev = pstDispstAttr->DispDev;
    MI_DISP_LAYER DispLayer = pstDispstAttr->DispLayer;
    MI_U8 u8PortNum = pstDispstAttr->u8PortNum;

    printf("\nEnter Disp Stop\n");

    //destroy put buff thread
    {
        StDispUtPort_t * pstInPortAttr = &pstDispstAttr->stInPortAttr;
        if(pstInPortAttr->bPortEnable)
        {
            pstInPortAttr->bPortEnable = false;
            pthread_join(pstInPortAttr->task, NULL);
            ExecFunc(MI_DISP_DisableInputPort(DispLayer, 0), MI_SUCCESS);
        }
    }
    ExecFunc(MI_DISP_DisableVideoLayer(DispLayer), MI_SUCCESS);
    ExecFunc(MI_DISP_UnBindVideoLayer(DispLayer, DispDev), MI_SUCCESS);
    ExecFunc(MI_DISP_Disable(DispDev), MI_SUCCESS);
    pstDispstAttr->bDevEnable = false;
    return 0;
}

static MI_S32 audio_GetFrameFromeFile(MI_S32 fd, void *bufferaddr, MI_S32 size)
{
    MI_S32 readsize;

    readsize = read(fd, bufferaddr, size);
    if(readsize < size)
        lseek(fd, 0, SEEK_SET);

    return readsize;
}

static void *audio_PutBuffer (void *pData)
{
    DevAttr_t *pstAttr = (DevAttr_t*)pData;
    AudioAttr_t * audioAttr = &pstAttr->stAud;

    MI_AUDIO_DEV aoDevId = audioAttr->aoDevId;
    MI_AO_CHN aoChn = audioAttr->aoChn;
    MI_AUDIO_Frame_t aoFrame;
    MI_AUDIO_Attr_t stAoAttr = audioAttr->stAoAttr;

    MI_S32 bitwidth = stAoAttr.eBitwidth == E_MI_AUDIO_BIT_WIDTH_16?16:24;
    MI_S32 sample_per_frame = stAoAttr.u32PtNumPerFrm;
    MI_S32 buffersize = sample_per_frame * bitwidth / 8;

    MI_S32 srcFd, ret;
    char *filePath = audioAttr->FilePath;
    char *buffer = NULL;

    buffer = (char*)malloc(buffersize * sizeof(char));
    srcFd = open(filePath, O_RDONLY);
    if (srcFd < 0)
    {
        printf("src_file: %s.\n", filePath);
        perror("open");
        free(buffer);
        return NULL;
    }
    aoFrame.apVirAddr[0] = buffer;
    while(audioAttr->bDevEnable)
    {
        //aoFrame.u32Len[0] =
        aoFrame.u32Len =
            audio_GetFrameFromeFile(srcFd, aoFrame.apVirAddr[0], buffersize);
retry:
        ret = MI_AO_SendFrame(aoDevId, aoChn, &aoFrame, 1);
        if (ret != 0)
            goto retry;
    }
    close(srcFd);
    free(buffer);
    return NULL;
}

MI_S32 audio_start(DevAttr_t *pstAttr)
{
    AudioAttr_t *pstAudioAttr = &pstAttr->stAud;

    if(!pstAttr->stEnable.audio_enable || pstAudioAttr->bDevEnable)
        return 0;

    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV aoDevId = pstAudioAttr->aoDevId;
    MI_AO_CHN aoChn = pstAudioAttr->aoChn;
    MI_AUDIO_Attr_t stAoAttr = pstAudioAttr->stAoAttr;

    ExecFunc(MI_AO_SetPubAttr(aoDevId, &stAoAttr), MI_SUCCESS);
    ExecFunc(MI_AO_GetPubAttr(aoDevId, &stAoAttr), MI_SUCCESS);
    ExecFunc(MI_AO_Enable(aoDevId), MI_SUCCESS);
    ExecFunc(MI_AO_EnableChn(aoDevId, aoChn), MI_SUCCESS);

    pthread_create(&pstAudioAttr->task, NULL, audio_PutBuffer, pstAttr);

    pstAudioAttr->bDevEnable = true;
    return 0;
}

MI_S32 audio_stop(DevAttr_t *pstAttr)
{
    AudioAttr_t *pstAudioAttr = &pstAttr->stAud;

    if(!pstAttr->stEnable.audio_enable || !pstAudioAttr->bDevEnable)
       return 0;

    printf("\nEnter Audio Stop\n");

    MI_AUDIO_DEV aoDevId = pstAudioAttr->aoDevId;
    MI_AO_CHN aoChn = pstAudioAttr->aoChn;

    pstAudioAttr->bDevEnable = false;
    pthread_join(pstAudioAttr->task, NULL);
    ExecFunc(MI_AO_DisableChn(aoDevId, aoChn), MI_SUCCESS);
    ExecFunc(MI_AO_Disable(aoDevId), MI_SUCCESS);
    pstAudioAttr->bDevEnable = false;
    return 0;
}

void help_message(MI_S32 argc, char **argv)
{
    printf("\n");
    printf(" %s params\n", argv[0]);
    printf(" params: \n");
    printf("   -i param_ini file, default: %s\n", INIFILE_PATH);
}

MI_HDMI_SampleRate_e parsing_hdmiAudSampleRate(MI_S32 rate)
{
    MI_HDMI_SampleRate_e samplerate;

    switch (rate)
    {
        case 32000:
            samplerate = E_MI_HDMI_AUDIO_SAMPLERATE_32K;
            break;
        case 48000:
            samplerate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
            break;
        default:
            samplerate = E_MI_HDMI_AUDIO_SAMPLERATE_UNKNOWN;
            break;
    }
    return samplerate;
}

MI_HDMI_AudioCodeType_e parsing_hdmiAudCodeType(const char *str)
{
    MI_HDMI_AudioCodeType_e codetype = E_MI_HDMI_ACODE_MAX;

    if (!strcmp(str, "pcm"))
        codetype = E_MI_HDMI_ACODE_PCM;
    else
        codetype = E_MI_HDMI_ACODE_NON_PCM;
    return codetype;
}
MI_HDMI_BitDepth_e parsing_hdmiAudBitDepth(const MI_S32 depth)
{
    MI_HDMI_BitDepth_e bitdepth = 0;

    switch (depth)
    {
        case 8:
            bitdepth = E_MI_HDMI_BIT_DEPTH_8;
            break;
        case 16:
            bitdepth = E_MI_HDMI_BIT_DEPTH_16;
            break;
        case 18:
            bitdepth = E_MI_HDMI_BIT_DEPTH_18;
            break;
        case 20:
            bitdepth = E_MI_HDMI_BIT_DEPTH_20;
            break;
        case 24:
            bitdepth = E_MI_HDMI_BIT_DEPTH_24;
            break;
        case 32:
            bitdepth = E_MI_HDMI_BIT_DEPTH_32;
            break;
        default:
            bitdepth = E_MI_HDMI_BIT_DEPTH_MAX;
            break;
    }
    return bitdepth;
}

MI_HDMI_OutputMode_e parsing_hdmiOutMode(const char* str)
{
    MI_HDMI_OutputMode_e mode;

    if (!strcmp(str, "hdmi"))
        mode = E_MI_HDMI_OUTPUT_MODE_HDMI;
    else
    if (!strcmp(str, "hdmi-hdcp"))
        mode = E_MI_HDMI_OUTPUT_MODE_HDMI_HDCP;
    else
    if (!strcmp(str, "dvi"))
        mode = E_MI_HDMI_OUTPUT_MODE_DVI;
    else
    if (!strcmp(str, "hdmi-hdcp"))
        mode = E_MI_HDMI_OUTPUT_MODE_DVI_HDCP;

    return mode;
}

MI_HDMI_ColorType_e parsing_hdmiColorType(const char* str)
{
    MI_HDMI_ColorType_e colortype = 0;

    if (!strcmp(str, "rgb444"))
        colortype = E_MI_HDMI_COLOR_TYPE_RGB444;
    else
    if (!strcmp(str, "ycbcr420"))
        colortype = E_MI_HDMI_COLOR_TYPE_YCBCR420;
    else
    if (!strcmp(str, "ycbcr422"))
        colortype = E_MI_HDMI_COLOR_TYPE_YCBCR422;
    else
    if (!strcmp(str, "ycbcr444"))
        colortype = E_MI_HDMI_COLOR_TYPE_YCBCR444;
    else
        colortype = E_MI_HDMI_DEEP_COLOR_MAX;

    return colortype;
}

MI_HDMI_TimingType_e parsing_hdmiDeepColor(const MI_S32 type)
{
    MI_HDMI_DeepColor_e deepcolor = 0;

    switch (type)
    {
        case 24:
            deepcolor = E_MI_HDMI_DEEP_COLOR_24BIT;
            break;
        case 30:
            deepcolor = E_MI_HDMI_DEEP_COLOR_30BIT;
            break;
        case 36:
            deepcolor = E_MI_HDMI_DEEP_COLOR_36BIT;
            break;
        case 48:
            deepcolor = E_MI_HDMI_DEEP_COLOR_48BIT;
            break;
         default:
            deepcolor = E_MI_HDMI_DEEP_COLOR_MAX;
            break;
    }

    return deepcolor;
}

MI_HDMI_TimingType_e parsing_hdmiTiming(const char *str)
{
    MI_HDMI_TimingType_e timing;
    TimingArray_t *ptiming_desc = astTimingArray;
    MI_S32 index = 0;

    if (!str)
        return E_MI_HDMI_TIMING_MAX;

    for (index = 0; index < sizeof(astTimingArray)/sizeof(TimingArray_t);index++)
    {
        if (!strcmp(ptiming_desc[index].desc, str))
            return ptiming_desc[index].eHdmiTiming;

    }
    return E_MI_HDMI_TIMING_MAX;
}

MI_HDMI_DeviceId_e parsing_hdmiDevid(MI_S32 devid)
{
    MI_HDMI_DeviceId_e id = E_MI_HDMI_ID_0;
    switch (devid)
    {
        case 0:
        default:
            id = E_MI_HDMI_ID_0;
            break;
    }
    return id;
}

MI_S32 print_param(DevAttr_t *attr)
{
    if (!attr)
        return -EINVAL;
    printf("\nHdmi Attr:\n");
    printf("  hdmi enable: %d audio enable:%d disp enable:%d\n",
                    attr->stEnable.hdmi_enable,
                    attr->stEnable.audio_enable,
                    attr->stEnable.disp_enable);
    if (attr->stEnable.hdmi_enable)
    {
        printf("  hdmi devid         :%d\n", attr->stHdmi.eHdmi);
        printf("  video eTimingType  :%d\n", attr->stHdmi.stAttr.stVideoAttr.eTimingType);
        printf("  video eColorType   :%d\n", attr->stHdmi.stAttr.stVideoAttr.eColorType);
        printf("  video eDeepColor   :%d\n", attr->stHdmi.stAttr.stVideoAttr.eDeepColorMode);
        printf("  video OutMode      :%d\n", attr->stHdmi.stAttr.stVideoAttr.eOutputMode);
        printf("  audio bitwidth     :%d\n", attr->stHdmi.stAttr.stAudioAttr.eBitDepth);
        printf("  audio codetype     :%d\n", attr->stHdmi.stAttr.stAudioAttr.eCodeType);
        printf("  audio sample rate  :%d\n", attr->stHdmi.stAttr.stAudioAttr.eSampleRate);
        printf("  audio mutichannel  :%d\n", attr->stHdmi.stAttr.stAudioAttr.bIsMultiChannel);
    }
    if (attr->stEnable.audio_enable)
    {
        printf("\nAudio Attr:\n");
        printf("  DeviceId      :%d\n", attr->stAud.aoDevId);
        printf("  FrameNum      :%d\n", attr->stAud.stAoAttr.u32FrmNum);
        printf("  PtNumPerFrm   :%d\n", attr->stAud.stAoAttr.u32PtNumPerFrm);
        printf("  source  :%s\n", attr->stAud.FilePath);
    }
    if (attr->stEnable.disp_enable)
    {
        printf("\nDisp Attr:\n");
        printf("  DeviceId     :%d\n", attr->stDisp.DispDev);
        printf("  Layer        :%d\n", attr->stDisp.DispLayer);
        printf("  PortNum      :%d\n", attr->stDisp.u8PortNum);
        printf("  source  :%s\n", attr->stDisp.stInPortAttr.FilePath);
    }
    return 0;
}

MI_S32 param_parsing(DevAttr_t *attr, char *ini_file_name)
{
    dictionary *pParamDic = NULL;
    HdmiAttr_t *pHdmiAttr = NULL;
    AudioAttr_t *pAudioAttr = NULL;
    DispstAttr_t *pDispstAttr = NULL;
    Arg_t * pModEnable = NULL;

    char *var = NULL;
    const char **array = NULL;

    if (!attr || attr->stEnable.Inited)
        return -EINVAL;

    pParamDic = iniparser_load(ini_file_name);
    if (!pParamDic)
    {
        printf("parsing param failed\n");
        return -1;
    }

    attr->stEnable.Inited = true;
    pModEnable = &attr->stEnable;

    /* parsing hdmi param */
#define PARASING_MODULE "HDMI"
    pHdmiAttr = &attr->stHdmi;
    if(iniparser_find_entry(pParamDic, PARASING_MODULE))
    {
        MI_HDMI_Attr_t *pstAttr = &pHdmiAttr->stAttr;
        if(iniparser_getboolean(pParamDic, PARASING_MODULE":enable", 0))
        {
            pModEnable->hdmi_enable = true;
 
            pHdmiAttr->eHdmi = 
                parsing_hdmiDevid(iniparser_getboolean(pParamDic, PARASING_MODULE":devid", 0));
            pstAttr->stVideoAttr.eTimingType =
                parsing_hdmiTiming(iniparser_getstring(pParamDic, PARASING_MODULE":video_timming", 0));
            pstAttr->stVideoAttr.eColorType =
                parsing_hdmiColorType(iniparser_getstring(pParamDic, PARASING_MODULE":video_out_colortype", 0));
            pstAttr->stVideoAttr.eDeepColorMode =
                parsing_hdmiDeepColor(iniparser_getint(pParamDic, PARASING_MODULE":video_deep_color", 0));
            pstAttr->stVideoAttr.eOutputMode = parsing_hdmiOutMode(iniparser_getstring(pParamDic, PARASING_MODULE":video_outputmode", 0));
            
            pstAttr->stAudioAttr.bEnableAudio = true;
            pstAttr->stAudioAttr.eBitDepth =
                parsing_hdmiAudBitDepth(iniparser_getint(pParamDic, PARASING_MODULE":audio_bitwidth", 0));
            pstAttr->stAudioAttr.eCodeType =
                parsing_hdmiAudCodeType(iniparser_getstring(pParamDic, PARASING_MODULE":audio_codetype", 0));
            pstAttr->stAudioAttr.eSampleRate =
                parsing_hdmiAudSampleRate(iniparser_getint(pParamDic, PARASING_MODULE":audio_sample_rate", 0));
            pstAttr->stAudioAttr.bIsMultiChannel =
                iniparser_getint(pParamDic, PARASING_MODULE":audio_channel", 0) >= 2?true:false;
        }
    }

#undef PARASING_MODULE
#define PARASING_MODULE "AUDIO"
    pAudioAttr = &attr->stAud;
    /* parsing audio param */
    if(iniparser_find_entry(pParamDic, PARASING_MODULE))
    {
        MI_AUDIO_Attr_t *pstAttr = &pAudioAttr->stAoAttr;
        if(iniparser_getboolean(pParamDic, PARASING_MODULE":enable", 0))
        {
            MI_AUDIO_DEV aoDevId;
            MI_S32 paramcnt;
            const char ** array;

            pModEnable->audio_enable = true;

            pHdmiAttr->stAttr.stAudioAttr.bEnableAudio = true;
            aoDevId = iniparser_getboolean(pParamDic, PARASING_MODULE":devid", -1);
            pAudioAttr->aoDevId = (aoDevId==-1)?HDMI_AUDIO_DEVID:aoDevId;
            pAudioAttr->aoChn = 0;

            pstAttr->u32PtNumPerFrm =
                    iniparser_getint(pParamDic, PARASING_MODULE":numperfrm", 0);

            pstAttr->u32FrmNum =
                    iniparser_getint(pParamDic, PARASING_MODULE":frmcount", 0);

            /* parse source */
            strcpy(pAudioAttr->FilePath, iniparser_getstring(pParamDic, PARASING_MODULE":source", 0));
            if (access(pAudioAttr->FilePath, F_OK|O_RDONLY))
                return -EINVAL;

            /* set by hdmi param */
            pstAttr->eBitwidth =
                HdmiBitDepth_to_AudioBitwidth(pHdmiAttr->stAttr.stAudioAttr.eBitDepth);
            pstAttr->eSamplerate =
                HdmiSampleRate_to_AudioSampleRate(pHdmiAttr->stAttr.stAudioAttr.eSampleRate);
            pstAttr->eSoundmode =
                pHdmiAttr->stAttr.stAudioAttr.bIsMultiChannel?E_MI_AUDIO_SOUND_MODE_STEREO:E_MI_AUDIO_SOUND_MODE_MONO;
            pstAttr->u32ChnCnt = 2;
        }
    }

#undef PARASING_MODULE
#define PARASING_MODULE "DISP"
    pDispstAttr = &attr->stDisp;
    /* parsing video param */
    if(iniparser_find_entry(pParamDic, PARASING_MODULE))
    {
        StDispUtPort_t *pstPortAttr = &pDispstAttr->stInPortAttr;
        MI_DISP_PubAttr_t *pstPubAttr = &pDispstAttr->stPubAttr;
        MI_DISP_VideoLayerAttr_t *pstVideoLayAttr = &pDispstAttr->stLayerAttr;

        if(iniparser_getboolean(pParamDic, PARASING_MODULE":enable", 0))
        {
            MI_U32 u32LayerWidth, u32LayerHeight;

            pHdmiAttr->stAttr.stVideoAttr.bEnableVideo = true;
            pModEnable->disp_enable = true;
            pDispstAttr->DispDev =
                    iniparser_getboolean(pParamDic, PARASING_MODULE":devid", 0);
            pDispstAttr->DispLayer =
                    iniparser_getboolean(pParamDic, PARASING_MODULE":layer", 0);
            pDispstAttr->u8PortNum =
                    iniparser_getboolean(pParamDic, PARASING_MODULE":portnum", 0);

            /* parse source */
            strcpy(pstPortAttr->FilePath, iniparser_getstring(pParamDic, PARASING_MODULE":source", 0));
            if (access(pstPortAttr->FilePath, F_OK|O_RDONLY))
                return -EINVAL;

            /* set by hdmi param */
            HdmiTim_to_Format(pHdmiAttr->stAttr.stVideoAttr.eTimingType,
                   &u32LayerWidth, &u32LayerHeight);
            printf("\n=====================>hdmi to:%d,%dx%d\n",pHdmiAttr->stAttr.stVideoAttr.eTimingType,u32LayerWidth,u32LayerHeight);
            pstPortAttr->stInputPortAttr.stDispWin.u16X = 0;
            pstPortAttr->stInputPortAttr.stDispWin.u16Y = 0;
            pstPortAttr->stInputPortAttr.stDispWin.u16Width = u32LayerWidth;
            pstPortAttr->stInputPortAttr.stDispWin.u16Height = u32LayerHeight;
            pstPortAttr->stInputPortAttr.u16SrcWidth = u32LayerWidth;
            pstPortAttr->stInputPortAttr.u16SrcHeight = u32LayerHeight;
            pstPubAttr->u32BgColor=YUYV_BLACK;
            pstPubAttr->eIntfType = E_MI_DISP_INTF_HDMI;
            pstPubAttr->eIntfSync =
                    HdmiTim_to_DispTim(pHdmiAttr->stAttr.stVideoAttr.eTimingType);
            pstVideoLayAttr->ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            pstVideoLayAttr->stVidLayerSize.u16Width = u32LayerWidth;
            pstVideoLayAttr->stVidLayerSize.u16Height = u32LayerHeight;
            pstVideoLayAttr->stVidLayerDispWin.u16X = 0;
            pstVideoLayAttr->stVidLayerDispWin.u16Y = 0;
            pstVideoLayAttr->stVidLayerDispWin.u16Width = u32LayerWidth;
            pstVideoLayAttr->stVidLayerDispWin.u16Height = u32LayerHeight;
        }
    }

    print_param(attr);
    return 0;
}

int change_timming(DevAttr_t *pstAttr)
{
    DispstAttr_t *pstDispstAttr = &pstAttr->stDisp;
    HdmiAttr_t *pstHdmiAttr = &pstAttr->stHdmi;

    MI_HDMI_TimingType_e timing = Input_Support_Timing();
    if (timing!=E_MI_HDMI_TIMING_MAX)
    {
        MI_U32 u32LayerWidth, u32LayerHeight;
        pstHdmiAttr->stAttr.stVideoAttr.eTimingType = timing;

         /* set by hdmi param */
        HdmiTim_to_Format(timing, &u32LayerWidth, &u32LayerHeight);
        pstDispstAttr->stPubAttr.eIntfSync = HdmiTim_to_DispTim(timing);

        printf("pstDispstAttr->stPubAttr.eIntfSync %d\n", pstDispstAttr->stPubAttr.eIntfSync);
        pstDispstAttr->stLayerAttr.stVidLayerSize.u16Width = u32LayerWidth;
        pstDispstAttr->stLayerAttr.stVidLayerSize.u16Height = u32LayerHeight;
        pstDispstAttr->stLayerAttr.stVidLayerDispWin.u16Width = u32LayerWidth;
        pstDispstAttr->stLayerAttr.stVidLayerDispWin.u16Height = u32LayerHeight;

        pstDispstAttr->stInPortAttr.stInputPortAttr.stDispWin.u16Width = u32LayerWidth;
        pstDispstAttr->stInPortAttr.stInputPortAttr.stDispWin.u16Height = u32LayerHeight;

        hdmi_stop(pstAttr);
        MI_DISP_SetPubAttr(pstDispstAttr->DispDev, &pstDispstAttr->stPubAttr);
        MI_DISP_SetVideoLayerAttr(pstDispstAttr->DispLayer, &pstDispstAttr->stLayerAttr);
        MI_DISP_SetInputPortAttr(pstDispstAttr->DispLayer,0, &pstDispstAttr->stInPortAttr.stInputPortAttr);
        hdmi_start(pstAttr);
    } else {
        printf("Not Support Timing:%d \n", timing);
    }
}

int get_edid(DevAttr_t *pstAttr)
{
    MI_HDMI_Edid_t stEdid;
    MI_S32 s32ret, i;
    HdmiAttr_t *pstHdmiAttr = &pstAttr->stHdmi;

    s32ret = MI_SUCCESS;
    MI_HDMI_SinkInfo_t stSinkInfo;
    MI_HDMI_Attr_t stAttr;
    memset(&stSinkInfo, 0, sizeof(MI_HDMI_SinkInfo_t));
    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    s32ret = MI_HDMI_GetSinkInfo(E_MI_HDMI_ID_0, &stSinkInfo);
    printf("stSinkInfo.bConnected:%d\n",stSinkInfo.bConnected);
    printf("stSinkInfo.bSupportHdmi:%d\n",stSinkInfo.bSupportHdmi);
    s32ret = MI_HDMI_GetAttr(E_MI_HDMI_ID_0, &stAttr);
    //ST_WARN("..................................................................]]]\n");
    printf("stAttr.bConnec:%d\n",stAttr.bConnect);
    
    while(0 == stAttr.bConnect)
    {
        s32ret = MI_HDMI_GetAttr(E_MI_HDMI_ID_0, &stAttr);
        printf("hdmi plugout<<<<<<<<>>>>>>>>\n");
        return 0;
        sleep(1);
    }

    s32ret = MI_HDMI_ForceGetEdid(pstHdmiAttr->eHdmi, &stEdid);
/*
    for (i = 0; i < stEdid.u32Edidlength; i++)
    {
        if(!(i%8) && i!=0)
            printf("\n");
        printf("[0x%02x] ", stEdid.au8Edid[i]);
    }
    printf("\n");
*/
    s32ret = MI_HDMI_GetSinkInfo(E_MI_HDMI_ID_0, &stSinkInfo);
    for(int i=0;i<E_MI_HDMI_TIMING_MAX;i++)
        if(stSinkInfo.abVideoFmtSupported[i])
             printf("[%d]--------%s\n",_MI_HDMI_GetTimingMapFromVic((MI_HDMI_TimingType_e)i)->eHdmiTiming,_MI_HDMI_GetTimingMapFromVic((MI_HDMI_TimingType_e)i)->desc);

       
                   // printf("[%d]--------%dX%d@%d%c\n",i,timing_enum[i].u32Horizontal,timing_enum[i].u32Vertical,timing_enum[i].u32Freq,timing_enum[i].eScanMode ? 'P' : 'I');

    

}
int get_usr_edid(void)
{
    printf("change edid\n");

    MI_HDMI_Edid_t stEdid;
    MI_S32 s32ret, i;
    s32ret = MI_SUCCESS;
    MI_HDMI_SinkInfo_t stSinkInfo;
    MI_HDMI_Attr_t stAttr;
    FILE *edid = NULL;
    int length = 0;
    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
    //s32ret = MI_HDMI_GetSinkInfo(E_MI_HDMI_ID_0, &stSinkInfo);
    s32ret = MI_HDMI_GetAttr(E_MI_HDMI_ID_0, &stAttr);
    //ST_WARN("..................................................................]]]\n");


    if ((edid = fopen("samsung_edid.bin", "rb")) == NULL) {
        fprintf(stderr, "unable to open EDID data: %m\n");

    }

    fseek(edid, 0, SEEK_END);
    length = ftell(edid);
    fseek(edid, 0, SEEK_SET);
    printf("edid file size: %d\n",length);

    if (fread(stEdid.au8Edid, 1, length, edid) != length) {
        fprintf(stderr, "unable to read EDID: %m\n");

    }
    stEdid.u32Edidlength=256;
    stEdid.bEdidValid=1;   
    for (int i = 0; i < stEdid.u32Edidlength; i++)
        {
            if(!(i%8) && i!=0)
                printf("\n");
            printf("[0x%02x] ", stEdid.au8Edid[i]);
        }
    printf("\n");
    fclose(edid);

}

void deivce_stop()
{
     printf("catch exit signal\n");
     g_exit_app = true;
}
//=================================================================================================================================
//                    FB
//=================================================================================================================================
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
static char *framebuffer = NULL;
int fbFd = 0;
/**
 *dump fix info of Framebuffer
 */
void printFixedInfo ()
{
    printf ("Fixed screen info:\n"
            "\tid: %s\n"
            "\tsmem_start: 0x%lx\n"
            "\tsmem_len: %d\n"
            "\ttype: %d\n"
            "\ttype_aux: %d\n"
            "\tvisual: %d\n"
            "\txpanstep: %d\n"
            "\typanstep: %d\n"
            "\tywrapstep: %d\n"
            "\tline_length: %d\n"
            "\tmmio_start: 0x%lx\n"
            "\tmmio_len: %d\n"
            "\taccel: %d\n"
            "\n",
            finfo.id, finfo.smem_start, finfo.smem_len, finfo.type,
            finfo.type_aux, finfo.visual, finfo.xpanstep, finfo.ypanstep,
            finfo.ywrapstep, finfo.line_length, finfo.mmio_start,
            finfo.mmio_len, finfo.accel);
}

/**
 *dump var info of Framebuffer
 */
void printVariableInfo ()
{
    printf ("Variable screen info:\n"
            "\txres: %d\n"
            "\tyres: %d\n"
            "\txres_virtual: %d\n"
            "\tyres_virtual: %d\n"
            "\tyoffset: %d\n"
            "\txoffset: %d\n"
            "\tbits_per_pixel: %d\n"
            "\tgrayscale: %d\n"
            "\tred: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tgreen: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tblue: offset: %2d, length: %2d, msb_right: %2d\n"
            "\ttransp: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tnonstd: %d\n"
            "\tactivate: %d\n"
            "\theight: %d\n"
            "\twidth: %d\n"
            "\taccel_flags: 0x%x\n"
            "\tpixclock: %d\n"
            "\tleft_margin: %d\n"
            "\tright_margin: %d\n"
            "\tupper_margin: %d\n"
            "\tlower_margin: %d\n"
            "\thsync_len: %d\n"
            "\tvsync_len: %d\n"
            "\tsync: %d\n"
            "\tvmode: %d\n"
            "\n",
            vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual,
            vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel,
            vinfo.grayscale, vinfo.red.offset, vinfo.red.length,
            vinfo.red.msb_right, vinfo.green.offset, vinfo.green.length,
            vinfo.green.msb_right, vinfo.blue.offset, vinfo.blue.length,
            vinfo.blue.msb_right, vinfo.transp.offset, vinfo.transp.length,
            vinfo.transp.msb_right, vinfo.nonstd, vinfo.activate,
            vinfo.height, vinfo.width, vinfo.accel_flags, vinfo.pixclock,
            vinfo.left_margin, vinfo.right_margin, vinfo.upper_margin,
            vinfo.lower_margin, vinfo.hsync_len, vinfo.vsync_len,
            vinfo.sync, vinfo.vmode);
}
/**
 * draw Rectangle. the colormat of Framebuffer is ARGB8888
 */
void drawRect_rgb32 (int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel = 4;
    const int stride = finfo.line_length / bytesPerPixel;

    int *dest = (int *) (framebuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color;
        }
        dest += stride;
    }
}
void fb_clear_lcd(void)
{
    //test FBIOGET_SHOW
	MI_BOOL bShown;
    bShown  = false;
    if (ioctl(fbFd, FBIOSET_SHOW,&bShown)<0) {
        perror("Error: failed to FBIOSET_SHOW");
        exit(7);
    } 
}
void fb_black_lcd(int x, int y, int width, int height,int color)
{
    const char *devfile = "/dev/fb0";

    long int screensize = 0;

    fbFd = open (devfile, O_RDWR);

    if(fbFd == -1)
    {
        perror ("Error: cannot open framebuffer device");
        exit(1);
    }
    //get fb_fix_screeninfo
    if(ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        exit(2);
    }
    printFixedInfo();
    //get fb_var_screeninfo
    if(ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(3);
    }
    printVariableInfo();

    screensize = finfo.smem_len;

    /* Map the device to memory */
    framebuffer = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fbFd, 0);
    if(framebuffer == MAP_FAILED)
    {
        perror ("Error: Failed to map framebuffer device to memory");
        exit (4);
    }
    drawRect_rgb32(x, y, width, height,color);
}
//=================================================================================================================================
MI_S32 main(MI_S32 argc, char **argv)
{
    MI_S32 choice = 0,  ret = 0;
    char param_ini_file[50]=INIFILE_PATH;
    DevAttr_t attr = {};
    char mychar;
    signal(SIGINT, deivce_stop);
    while(-1 != (choice=getopt(argc,(char**)argv,"i:")) ){
        switch(choice){
        case 'i':
            strcpy(param_ini_file, optarg);
            break;
        case 'h':
            return ret;;
        default:
            break;
        }
    }

    if (access(param_ini_file, O_RDONLY | F_OK))
    {
        printf("\nfailed to access param_ini_file:%s \n\n", param_ini_file);
        return -EINVAL;
    }

    if(param_parsing(&attr, param_ini_file))
        return -EINVAL;

    attr.stEnable.Inited = true;
    

    ExecFunc(MI_SYS_Init(), MI_SUCCESS);



    ExecFunc(disp_start(&attr), MI_SUCCESS);
    printf("enter ,hdmi &  \n");
    //getchar();
    ExecFunc(hdmi_start(&attr), MI_SUCCESS);
    sleep(1);
        printf("enter ,clear osd \n");
    //getchar();
    //fb_clear_lcd();
    printf("enter ,hdmi & audio \n");
    //getchar();
    ExecFunc(audio_start(&attr), MI_SUCCESS);
 
   // ExecFunc(hdmi_start(&attr), MI_SUCCESS);

    

    while (1)
    {
        MI_U8 input[100];

        if (g_exit_app)
            break;

        fgets(input, sizeof(input), stdin);
        switch(input[0])
        {
            case 'c':
                change_timming(&attr);
                break;
            case 'e':
                get_edid(&attr);
                break;
            case 'u':
                get_usr_edid();
                break;   

             case 'q':
                g_exit_app = 1;
                break;                 
            default:
                printf("pls select choice: \n");
                printf("c: change timming\n");
                printf("e: get and print edid\n");
                printf("u: get ext and print edid\n");
                break;
        }

        usleep(100000);
    }
    printf("enter ,run MI_HDMI_SetAvMute\n");
    getchar();
   // ExecFunc(MI_HDMI_SetAvMute(0, true), MI_SUCCESS);//jackson
    
    printf("enter ,run disp_stop\n");
    getchar();
  
    ExecFunc(disp_stop(&attr), MI_SUCCESS);
    printf("enter ,run audio_stop\n");
    getchar();
    ExecFunc(audio_stop(&attr), MI_SUCCESS);
    printf("enter ,run hdmi_stop\n");
    getchar();
    ExecFunc(hdmi_stop(&attr), MI_SUCCESS);
    printf("enter ,run MI_SYS_Exit\n");
    getchar();   
    ExecFunc(MI_SYS_Exit(), MI_SUCCESS);
    return 0;
}

