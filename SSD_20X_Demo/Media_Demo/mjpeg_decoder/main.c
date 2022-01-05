#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <termios.h>
#include <string.h>
#include "v4l2.h"
#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_divp.h"
#include "mi_disp_datatype.h"
#include "mi_gfx_datatype.h"
#include "mi_venc.h"
#include "mi_venc_datatype.h"
#include "jdec.h"
#include "sstardisp.h"

#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

#define VENC_PERFORMANCE_TEST 0

#define VDEC_CHN_ID     0
    
#define VDEC_INPUT_WIDTH     1280
#define VDEC_INPUT_HEIGHT    720
    
#define VDEC_OUTPUT_WIDTH     1280
#define VDEC_OUTPUT_HEIGHT    720

#define DIVP_CHN0_ID     0
#define DIVP_CHN1_ID     1


#define DIVP_INPUT_WIDTH     1280
#define DIVP_INPUT_HEIGHT    720
    
#define DIVP_OUTPUT_WIDTH     352
#define DIVP_OUTPUT_HEIGHT    288

#define VENC_CHN_ID     0

#define LOCAL_VIDEO_W  1024
#define LOCAL_VIDEO_H  600
#define LOCAL_VIDEO_X  0
#define LOCAL_VIDEO_Y  0


#define ALIGN_BACK(x, a)            (((x) / (a)) * (a))

#define CAMERA_VIDEO_WIDTH      1280
#define CAMERA_VIDEO_HEIGHT     720

#define CAMERA_VIDEO_WIDTH_YUV      640
#define CAMERA_VIDEO_HEIGHT_YUV     480

#define CAMERA_VIDEO_WIDTH_MJPEG    1280
#define CAMERA_VIDEO_HEIGHT_MJPEG   720

#define TIMEUOT     100

#define JPEG_720P_FILE    "720p.jpg"

#define YUV_720P_FILE     "720p.yuv"

#define CIFYUVFILE      "CIF.h264"

#define H264_720P_FILE    "720p.h264"


static DeviceContex_t *ctx = NULL;
static Packet pkt;
static MI_U8 g_u8V4L2ThreadExitFlag = 0;
static MI_U8 g_u8V4L2ThreadExtDoneFlag = 0;

static MI_U8 g_u8CountNum = 0;

//x264
//static x264_t *h;
//static x264_picture_t pic;

typedef enum
{
    ST_DBG_NONE = 0,
    ST_DBG_ERR,
    ST_DBG_WRN,
    ST_DBG_INFO,
    ST_DBG_DEBUG,
    ST_DBG_ALL
}ST_DBG_LEVEL_e;

#define ASCII_COLOR_RED                          "\033[1;31m"
#define ASCII_COLOR_WHITE                        "\033[1;37m"
#define ASCII_COLOR_YELLOW                       "\033[1;33m"
#define ASCII_COLOR_BLUE                         "\033[1;36m"
#define ASCII_COLOR_GREEN                        "\033[1;32m"
#define ASCII_COLOR_END                          "\033[0m"

ST_DBG_LEVEL_e g_eSTDbgLevel = ST_DBG_WRN;
MI_U8 g_bSTFuncTrace = 0;

#define DBG_DEBUG(fmt, args...)     ({do{if(g_eSTDbgLevel>=ST_DBG_DEBUG){printf(ASCII_COLOR_GREEN"[APP INFO]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__,##args);}}while(0);})
#define DBG_INFO(fmt, args...)      ({do{if(g_eSTDbgLevel>=ST_DBG_INFO){printf(ASCII_COLOR_GREEN"[APP INFO]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__,##args);}}while(0);})
#define DBG_WRN(fmt, args...)       ({do{if(g_eSTDbgLevel>=ST_DBG_WRN){printf(ASCII_COLOR_YELLOW"[APP WRN ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##args);}}while(0);})
#define DBG_ERR(fmt, args...)       ({do{if(g_eSTDbgLevel>=ST_DBG_ERR){printf(ASCII_COLOR_RED"[APP ERR ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__,__LINE__, ##args);}}while(0);})
#define DBG_EXIT_ERR(fmt, args...)  ({do{printf(ASCII_COLOR_RED"<<<%s[%d] " fmt ASCII_COLOR_END,__FUNCTION__,__LINE__,##args);}while(0);})
#define DBG_ENTER()                 ({do{if(g_bSTFuncTrace){printf(ASCII_COLOR_BLUE">>>%s[%d] \n" ASCII_COLOR_END,__FUNCTION__,__LINE__);}}while(0);})
#define DBG_EXIT_OK()               ({do{if(g_bSTFuncTrace){printf(ASCII_COLOR_BLUE"<<<%s[%d] \n" ASCII_COLOR_END,__FUNCTION__,__LINE__);}}while(0);})

//struct timeval tv1 = {0, 0};
//struct timeval tv2 = {0, 0};

#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE MAKE_YUYV_VALUE(29,225,107)

#define SIZE_BUFFER_YUV420 CAMERA_VIDEO_WIDTH_MJPEG*CAMERA_VIDEO_HEIGHT_MJPEG*3/2
#define SIZE_BUFFER_YUV422 CAMERA_VIDEO_WIDTH_MJPEG*CAMERA_VIDEO_HEIGHT_MJPEG*2
#define TIME_DIFF_PRE_FRAME 33*1000

extern void ST_I422ToYUY2(const uint8_t* pu8src_y,
                           int src_stride_y,
                           const uint8_t* pu8src_u,
                           int src_stride_u,
                           const uint8_t* pu8src_v,
                           int src_stride_v,
                           uint8_t* pu8dst_yuy2,
                           int dst_stride_yuy2,
                           int width,
                           int height);

int sdk_Init(void)
{
    MI_DISP_PubAttr_t stDispPubAttr = {0};
    MI_SYS_ChnPort_t stSrcChnPort = {0};
    MI_SYS_ChnPort_t stDstChnPort = {0};
    MI_DIVP_ChnAttr_t stDivpChnAttr = {0};
    MI_DIVP_OutputPortAttr_t DivpOutputPortAttr = {0};
    //init sys
    //STCHECKRESULT(MI_SYS_Init());

    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;
    sstar_disp_init(&stDispPubAttr);

    //init divp
    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = CAMERA_VIDEO_WIDTH_MJPEG;
    stDivpChnAttr.stCropRect.u16Height  = CAMERA_VIDEO_HEIGHT_MJPEG;
    stDivpChnAttr.u32MaxWidth           = CAMERA_VIDEO_WIDTH_MJPEG;
    stDivpChnAttr.u32MaxHeight          = CAMERA_VIDEO_HEIGHT_MJPEG;
    STCHECKRESULT(MI_DIVP_CreateChn(DIVP_CHN0_ID, &stDivpChnAttr));
    STCHECKRESULT(MI_DIVP_StartChn(DIVP_CHN0_ID));
    memset(&DivpOutputPortAttr, 0, sizeof(DivpOutputPortAttr));
    DivpOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    DivpOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    DivpOutputPortAttr.u32Width           = ALIGN_BACK(LOCAL_VIDEO_W, 2);
    DivpOutputPortAttr.u32Height          = ALIGN_BACK(LOCAL_VIDEO_H, 2);
    STCHECKRESULT(MI_DIVP_SetOutputPortAttr(DIVP_CHN0_ID, &DivpOutputPortAttr));


    memset(&stSrcChnPort, 0, sizeof(stSrcChnPort));
    memset(&stDstChnPort, 0, sizeof(stDstChnPort));
    stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stSrcChnPort.u32ChnId = DIVP_CHN0_ID;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32PortId = 0;
    MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, 30, 30);
    return 0;
}

int sdk_DeInit(void)
{
    //unbind modules
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_DISP_PubAttr_t stDispPubAttr = {0};

    //unbind disp0 2 divp0
    memset(&stSrcChnPort, 0, sizeof(stSrcChnPort));
    memset(&stDstChnPort, 0, sizeof(stDstChnPort));
    stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stSrcChnPort.u32ChnId = DIVP_CHN0_ID;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));

    //deinit divp0
    STCHECKRESULT(MI_DIVP_StopChn(DIVP_CHN0_ID));
    STCHECKRESULT(MI_DIVP_DestroyChn(DIVP_CHN0_ID));

    //deinit sys
    //STCHECKRESULT(MI_SYS_Exit());
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;
    sstar_disp_Deinit(&stDispPubAttr);
    
    return 0;
}

static void *V4L2_Entry(void *args)
{
    int ret;
    int timeout = 0;
    uint8_t* pu8STSrcU = NULL;
    uint8_t* pu8STSrcV = NULL;
    jdecIMAGE image0 = {0};
    jdecIMAGE image1 = {0};

    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE bufHandle;
    MI_SYS_ChnPort_t stSrcChnPort;

    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&bufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    memset(&stSrcChnPort, 0, sizeof(stSrcChnPort));

    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stBufConf.stFrameCfg.u16Width = CAMERA_VIDEO_WIDTH_MJPEG;
    stBufConf.stFrameCfg.u16Height = CAMERA_VIDEO_HEIGHT_MJPEG;
    stBufConf.u32Flags = MI_SYS_MAP_VA;
    
    stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stSrcChnPort.u32ChnId = DIVP_CHN0_ID;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32PortId = 0;

    if (MI_SUCCESS != MI_SYS_MMA_Alloc((MI_U8 *)"#jdecI0",SIZE_BUFFER_YUV422, &image0.phyAddr))
    {
        DBG_ERR("MI_SYS_MMA_Alloc fail");
        return 0;
    }
    if (MI_SUCCESS != MI_SYS_Mmap(image0.phyAddr,SIZE_BUFFER_YUV422, (void *)&image0.virtAddr, TRUE))
    {
        MI_SYS_MMA_Free(image0.phyAddr);
        DBG_ERR("MI_SYS_Mmap fail");
        return 0;
    }
    if (MI_SUCCESS != MI_SYS_MMA_Alloc((MI_U8 *)"#jdecI1",SIZE_BUFFER_YUV422, &image1.phyAddr))
    {
        DBG_ERR("MI_SYS_MMA_Alloc fail");
        return 0;
    }
    if (MI_SUCCESS != MI_SYS_Mmap(image1.phyAddr,SIZE_BUFFER_YUV422, (void *)&image1.virtAddr, TRUE))
    {
        MI_SYS_MMA_Free(image1.phyAddr);
        DBG_ERR("MI_SYS_Mmap fail");
        return 0;
    }

    //gettimeofday(&tv1, NULL);
    //gettimeofday(&tv2, NULL);
    //u64StartTimer = tv2.tv_sec * 1000000L + tv2.tv_usec;
    g_u8CountNum = 0;
    while(!g_u8V4L2ThreadExitFlag)
    {
        ret = v4l2_read_packet(ctx, &pkt);
        if(ret == -EAGAIN) 
        {
            if(++timeout < TIMEUOT)
            {
                usleep(1*1000);
                continue;
            }
            else
            {
                DBG_ERR("V4L2_Entry read fail exit\n");
                g_u8V4L2ThreadExitFlag = TRUE;
                return 0;
            }
        }
        #if 0
        if(ret >=0) 
        {
            u8test1 = u8test1 + 1;
            if(u8test1%2)
            {
                gettimeofday(&tv1, NULL);
                DBG_WRN("difftimer:%ld:u8test1:%d\n",(((tv1.tv_sec - tv2.tv_sec) * 1000000L + tv1.tv_usec) - tv2.tv_usec),u8test1);
            }
            else
            {
                gettimeofday(&tv2, NULL);
                DBG_WRN("difftimer:%ld:u8test1:%d\n",(((tv2.tv_sec - tv1.tv_sec) * 1000000L + tv2.tv_usec) - tv1.tv_usec),u8test1);
            }
            timeout = 0;
            v4l2_read_packet_end(ctx, &pkt);
        }
        #endif
        if(ret >=0) 
        {
            timeout = 0;
            DBG_DEBUG("Get Pkt size=%d\n", pkt.size);
            jdec_decodeYUVFromBuf((char*)pkt.data, pkt.size, &image0, TANSFORM_NONE, SAMP_420);
            MI_SYS_ChnInputPortGetBuf(&stSrcChnPort, &stBufConf, &stBufInfo, &bufHandle, 0);
            if(bufHandle != NULL)
            {
                if (image0.phyAddr)
                {
#if 0
                    gettimeofday(&tv2, NULL);
                    u64EndTimer = tv2.tv_sec * 1000000L + tv2.tv_usec;
                    u64DiffTimer = u64EndTimer - u64StartTimer;
                    DBG_WRN("difftimer:%lld,pts:%lld\n",u64DiffTimer,pkt.pts);
                    u64StartTimer = u64EndTimer;
#endif
                    pu8STSrcU = (uint8_t*)image0.virtAddr+image0.width*image0.height;
                    pu8STSrcV = (uint8_t*)image0.virtAddr+image0.width*image0.height+image0.width*image0.height/2;
                    ST_I422ToYUY2((uint8_t*)image0.virtAddr,image0.width,
                                    pu8STSrcU,image0.width/2,
                                    pu8STSrcV,image0.width/2,
                                    stBufInfo.stFrameData.pVirAddr[0],image0.width*2,
                                    image0.width,image0.height);
                    stBufInfo.u64Pts = pkt.pts;
                }
                else
                {
                   // memcpy(stBufInfo.stFrameData.pVirAddr[0], image0.virtAddr, image0.height * image0.pitch);
                   DBG_ERR("no image0.phyAddr\n");
                }
                MI_SYS_ChnInputPortPutBuf(bufHandle, &stBufInfo, FALSE);
            }
            else
            {
                DBG_ERR("MI_SYS_ChnInputPortGetBuf fail \n");
                return 0;
            }
            //save_file(pkt.data, pkt.size, 1);
            v4l2_read_packet_end(ctx, &pkt);
        }

    }
    MI_SYS_FlushInvCache(image0.virtAddr,SIZE_BUFFER_YUV422); 
    MI_SYS_Munmap(image0.virtAddr, SIZE_BUFFER_YUV422);
    MI_SYS_MMA_Free(image0.phyAddr);
    MI_SYS_FlushInvCache(image1.virtAddr,SIZE_BUFFER_YUV422); 
    MI_SYS_Munmap(image1.virtAddr, SIZE_BUFFER_YUV422);
    MI_SYS_MMA_Free(image1.phyAddr);
    DBG_WRN("V4L2_Entry pthread_exit\n");
    g_u8V4L2ThreadExtDoneFlag = 1;
    return 0;
}


int main(void)
{
    pthread_t V4L2ThreadID;
    g_u8V4L2ThreadExitFlag = FALSE;
    
    //init sdk
    sdk_Init();

    //open camera
    v4l2_dev_init(&ctx,  (char*)"/dev/video0");
    v4l2_dev_set_fmt(ctx, V4L2_PIX_FMT_MJPEG, CAMERA_VIDEO_WIDTH_MJPEG, CAMERA_VIDEO_HEIGHT_MJPEG);

    if (v4l2_read_header(ctx))
    {
        DBG_ERR("Can't find usb camera\n");
        return -1;
    }
    pthread_create(&V4L2ThreadID, NULL, V4L2_Entry, NULL);

    while(!g_u8V4L2ThreadExitFlag)
    {
        char c;
        c = getchar();
        if(c == 'q')
        {
            break;
        }
        usleep(100*1000);
    }

    //exit pthread
    g_u8V4L2ThreadExitFlag = TRUE;
    while(g_u8V4L2ThreadExtDoneFlag == 0)
    {
        DBG_WRN("wait forV4L2Thread exit\n");
        usleep(100*1000);
    }
    //deinit v4l2 
    v4l2_read_close(ctx);
    v4l2_dev_deinit(ctx);
    
    g_u8V4L2ThreadExitFlag = TRUE;
    g_u8V4L2ThreadExtDoneFlag = TRUE;
    pthread_join(V4L2ThreadID, NULL);

    //deinit sdk
    sdk_DeInit();
    return 0;
}
