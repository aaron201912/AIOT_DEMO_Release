#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "mi_sys.h"
#include "sstardisp.h"

#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_gfx.h"
#include "common.h"


#if ENABLE_HDMI
#include "mi_hdmi.h"
#endif

#include "SAT070CP50_1024x600.h"
//#include "SAT070JHH_1024x600.h"

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif
#if ENABLE_HDMI
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
#endif
#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)


static int disp_port_init(int disp_port_num)
{
    MI_U32 i = 0;
    MI_DISP_INPUTPORT u32InputPort = 0;
    MI_DISP_InputPortAttr_t stInputPortAttr;

    // 设置四副图像的显示大小与位置,Layer0
    for (i = 0; i < disp_port_num; i++)
    {
        if(disp_port_num == 1)//单路，全屏
        {
            stDispWnd[i].s32X    = OutX;
            stDispWnd[i].s32Y    = OutY;
            stDispWnd[i].u16PicW = OutDispWidth;
            stDispWnd[i].u16PicH = OutDispHeight;
        }
        else
        {
            stDispWnd[i].s32X    = ALIGN_BACK((OutDispWidth / 2) * (i % 2), 2);
            stDispWnd[i].s32Y    = ALIGN_BACK((OutDispHeight / 2) * (i / 2), 2);
            stDispWnd[i].u16PicW = ALIGN_BACK((OutDispWidth / 2), 2);
            stDispWnd[i].u16PicH = ALIGN_BACK((OutDispHeight / 2), 2);
        }
        printf("stDispWnd[%d]: pos_x %d, pos_y %d, width %d, height %d\n", i, stDispWnd[i].s32X, stDispWnd[i].s32Y, stDispWnd[i].u16PicW, stDispWnd[i].u16PicH);
    }
    // disp port 0,1,2,3 -> disp_layer0   port 4 -> disp_layer1
    for (i = 0; i < disp_port_num; i ++)
    {
        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));

        u32InputPort = i;
        STCHECKRESULT(MI_DISP_GetInputPortAttr(DISP_LAYER0, u32InputPort, &stInputPortAttr));
        stInputPortAttr.stDispWin.u16X      = stDispWnd[i].s32X;
        stInputPortAttr.stDispWin.u16Y      = stDispWnd[i].s32Y;
        stInputPortAttr.stDispWin.u16Width  = stDispWnd[i].u16PicW;
        stInputPortAttr.stDispWin.u16Height = stDispWnd[i].u16PicH;
        stInputPortAttr.u16SrcWidth         = ALIGN_BACK(stDispWnd[i].u16PicW, 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_BACK(stDispWnd[i].u16PicH, 32);

        STCHECKRESULT(MI_DISP_SetInputPortAttr(DISP_LAYER0, u32InputPort, &stInputPortAttr));
        STCHECKRESULT(MI_DISP_GetInputPortAttr(DISP_LAYER0, u32InputPort, &stInputPortAttr));
        STCHECKRESULT(MI_DISP_EnableInputPort(DISP_LAYER0, u32InputPort));
        STCHECKRESULT(MI_DISP_SetInputPortSyncMode(DISP_LAYER0, u32InputPort, E_MI_DISP_SYNC_MODE_FREE_RUN));
    }

    if(g_enable_pip)
    {
        //设置居中显示
        stExtraDispWnd.u16PicW = ALIGN_BACK((OutDispWidth / 2), 2);
        stExtraDispWnd.u16PicH = ALIGN_BACK((OutDispHeight / 2), 2);
        stExtraDispWnd.s32X    = (OutDispWidth - stExtraDispWnd.u16PicW) / 2 + OutX;
        stExtraDispWnd.s32Y    = (OutDispHeight - stExtraDispWnd.u16PicH) / 2 + OutY;
        //printf("stExtraDispWnd: pos_x %d, pos_y %d, width %d, height %d\n", stExtraDispWnd.s32X, stExtraDispWnd.s32Y, stExtraDispWnd.u16PicW, stExtraDispWnd.u16PicH);

        memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
        STCHECKRESULT(MI_DISP_GetInputPortAttr(DISP_LAYER1, DISP_PORT_EXTRA, &stInputPortAttr));
        stInputPortAttr.stDispWin.u16X      = stExtraDispWnd.s32X;
        stInputPortAttr.stDispWin.u16Y      = stExtraDispWnd.s32Y;
        stInputPortAttr.stDispWin.u16Width  = stExtraDispWnd.u16PicW;
        stInputPortAttr.stDispWin.u16Height = stExtraDispWnd.u16PicH;
        stInputPortAttr.u16SrcWidth         = ALIGN_BACK(stExtraDispWnd.u16PicW, 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_BACK(stExtraDispWnd.u16PicH, 32);

        STCHECKRESULT(MI_DISP_SetInputPortAttr(DISP_LAYER1, DISP_PORT_EXTRA, &stInputPortAttr));
        STCHECKRESULT(MI_DISP_GetInputPortAttr(DISP_LAYER1, DISP_PORT_EXTRA, &stInputPortAttr));
        STCHECKRESULT(MI_DISP_EnableInputPort(DISP_LAYER1, DISP_PORT_EXTRA));
        STCHECKRESULT(MI_DISP_SetInputPortSyncMode(DISP_LAYER1, DISP_PORT_EXTRA, E_MI_DISP_SYNC_MODE_FREE_RUN));
    }

    return 0;
}

static void disp_port_deinit(int disp_port_num)
{
    MI_U32 i = 0;
    MI_DISP_INPUTPORT u32InputPort = 0;
    for (i = 0; i < disp_port_num; i ++)
    {
        u32InputPort = i;
        MI_DISP_DisableInputPort(DISP_LAYER0, u32InputPort);
    }
    MI_DISP_DisableVideoLayer(DISP_LAYER0);
    MI_DISP_UnBindVideoLayer(DISP_LAYER0, DISP_DEV);
    if(g_enable_pip)
    {
        MI_DISP_DisableInputPort(DISP_LAYER1, DISP_PORT_EXTRA);
        MI_DISP_DisableVideoLayer(DISP_LAYER1);
        MI_DISP_UnBindVideoLayer(DISP_LAYER1, DISP_DEV);
    }
    MI_DISP_Disable(0);
}

static void disp_setvideo_layerattr()
{
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));
    if(bRota && (bRota == E_MI_DISP_ROTATE_180))
    {
        stLayerAttr.stVidLayerSize.u16Width  = stPanelParam.u16Width;
        stLayerAttr.stVidLayerSize.u16Height = stPanelParam.u16Height;
    }
    else
    {
        stLayerAttr.stVidLayerSize.u16Width  = stPanelParam.u16Height;
        stLayerAttr.stVidLayerSize.u16Height = stPanelParam.u16Width;
    }
    stLayerAttr.stVidLayerDispWin.u16X      = 0;
    stLayerAttr.stVidLayerDispWin.u16Y      = 0;
    stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;

    MI_DISP_SetVideoLayerAttr(DISP_LAYER0, &stLayerAttr);
    MI_DISP_EnableVideoLayer(DISP_LAYER0);
    MI_DISP_BindVideoLayer(DISP_LAYER0, DISP_DEV);

    if(g_enable_pip)
    {
        MI_DISP_VideoLayerAttr_t stLayerAttr1;
        memset(&stLayerAttr1, 0, sizeof(stLayerAttr1));
        stLayerAttr1.stVidLayerSize.u16Width  = ALIGN_BACK((stLayerAttr.stVidLayerSize.u16Width / 2), 2);
        stLayerAttr1.stVidLayerSize.u16Height = ALIGN_BACK((stLayerAttr.stVidLayerSize.u16Height / 2), 2);
        stLayerAttr1.stVidLayerDispWin.u16X   = (stLayerAttr.stVidLayerSize.u16Width - stLayerAttr1.stVidLayerSize.u16Width) / 2;
        stLayerAttr1.stVidLayerDispWin.u16Y   = (stLayerAttr.stVidLayerSize.u16Height - stLayerAttr1.stVidLayerSize.u16Height) / 2;

        stLayerAttr1.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;

        MI_DISP_SetVideoLayerAttr(DISP_LAYER1, &stLayerAttr1);
        MI_DISP_EnableVideoLayer(DISP_LAYER1);
        MI_DISP_BindVideoLayer(DISP_LAYER1, DISP_DEV);
    }
}

static void sstar_hdmi_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
#if ENABLE_HDMI
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t stAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;

    if (E_MI_DISP_INTF_HDMI == pstDispPubAttr->eIntfType)
    {
        stInitParam.pCallBackArgs = NULL;
        stInitParam.pfnHdmiEventCallback = Hdmi_callback_impl;
        MI_HDMI_Init(&stInitParam);
        MI_HDMI_Open(E_MI_HDMI_ID_0);

        memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
        stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
        stAttr.stAudioAttr.bEnableAudio = TRUE;
        stAttr.stAudioAttr.bIsMultiChannel = 0;
        stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
        stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
        stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
        stAttr.stVideoAttr.bEnableVideo = TRUE;
        stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type

#if ENABLE_HDMI_4K
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_24BIT;
        stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_30P;
#else
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
        stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
#endif
        stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
        MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr);
        MI_HDMI_Start(E_MI_HDMI_ID_0);
        pstDispPubAttr->u32BgColor = YUYV_BLACK;
#if ENABLE_HDMI_4K
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_3840x2160_30;
#else
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_1080P60;
#endif
        MI_DISP_SetPubAttr(0, pstDispPubAttr);

        MI_DISP_Enable(0);
        MI_DISP_BindVideoLayer(0, 0);
        memset(&stLayerAttr, 0, sizeof(stLayerAttr));

#if ENABLE_HDMI_4K
        if(bRota)
        {
        stLayerAttr.stVidLayerSize.u16Width  = 2160;
        stLayerAttr.stVidLayerSize.u16Height = 3840;
        }
        else
        {
        stLayerAttr.stVidLayerSize.u16Width  = 3840;
        stLayerAttr.stVidLayerSize.u16Height = 2160;
        }
#else
        if(bRota)
        {
        stLayerAttr.stVidLayerSize.u16Width  = OutDispHeight;
        stLayerAttr.stVidLayerSize.u16Height = OutDispWidth;
        }
        else
        {
        stLayerAttr.stVidLayerSize.u16Width  = OutDispWidth;
        stLayerAttr.stVidLayerSize.u16Height = OutDispHeight;
        }

#endif

        stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        stLayerAttr.stVidLayerDispWin.u16X      = OutX;
        stLayerAttr.stVidLayerDispWin.u16Y      = OutY;

#if ENABLE_HDMI_4K
        if(bRota)
        {
        stLayerAttr.stVidLayerDispWin.u16Width  = 2160;
        stLayerAttr.stVidLayerDispWin.u16Height = 3840;
        }
        else
        {
        stLayerAttr.stVidLayerDispWin.u16Width  = 3840;
        stLayerAttr.stVidLayerDispWin.u16Height = 2160;
        }
#else
        if(bRota)
        {
        stLayerAttr.stVidLayerDispWin.u16Width  = OutDispHeight;
        stLayerAttr.stVidLayerDispWin.u16Height = OutDispWidth;
        }
        else
        {
        stLayerAttr.stVidLayerDispWin.u16Width  = OutDispWidth;
        stLayerAttr.stVidLayerDispWin.u16Height = OutDispHeight;
        }
#endif

        MI_DISP_SetVideoLayerAttr(0, &stLayerAttr);
        MI_DISP_EnableVideoLayer(0);

        stInputPortAttr.u16SrcWidth = inDispWidth;
        stInputPortAttr.u16SrcHeight = inDispHeight;
        stInputPortAttr.stDispWin.u16X = OutX;
        stInputPortAttr.stDispWin.u16Y = OutY;
#if ENABLE_HDMI_4K
        if(bRota)
        {
        stInputPortAttr.stDispWin.u16Width = OutDispHeight;
        stInputPortAttr.stDispWin.u16Height = OutDispWidth;
        }
        else
        {
        stInputPortAttr.stDispWin.u16Width = OutDispWidth;
        stInputPortAttr.stDispWin.u16Height = OutDispHeight;
        }
#else
        if(bRota)
        {
        stInputPortAttr.stDispWin.u16Width = OutDispHeight;
        stInputPortAttr.stDispWin.u16Height = OutDispWidth;
        }
        else
        {
        stInputPortAttr.stDispWin.u16Width = OutDispWidth;
        stInputPortAttr.stDispWin.u16Height = OutDispHeight;
        }
#endif

        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
    }

#endif
}

int sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr, int disp_port_num)
{
    MI_PANEL_LinkType_e eLinkType;

    MI_SYS_Init();
    MI_GFX_Open();

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        pstDispPubAttr->u32BgColor = YUYV_BLACK;
        pstDispPubAttr->stSyncInfo.u16Vact = stPanelParam.u16Height;
        pstDispPubAttr->stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth +
                                                                      stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Hact = stPanelParam.u16Width;
        pstDispPubAttr->stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth +
                                                                      stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Bvact = 0;
        pstDispPubAttr->stSyncInfo.u16Bvbb = 0;
        pstDispPubAttr->stSyncInfo.u16Bvfb = 0;
        pstDispPubAttr->stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
        pstDispPubAttr->stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
        pstDispPubAttr->stSyncInfo.u32FrameRate = stPanelParam.u16DCLK * 1000000 / (stPanelParam.u16HTotal * stPanelParam.u16VTotal);
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_USER;
        pstDispPubAttr->eIntfType = E_MI_DISP_INTF_LCD;
        eLinkType = stPanelParam.eLinkType;

        MI_DISP_SetPubAttr(0, pstDispPubAttr);
        MI_DISP_Enable(0);

        disp_setvideo_layerattr();
        disp_port_init(disp_port_num);

        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);
        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
        }
    }
    else if(pstDispPubAttr->eIntfType == E_MI_DISP_INTF_HDMI)
    {
        sstar_hdmi_init(pstDispPubAttr);
    }
    else
    {
        printf("Invalid eIntfType:%d \n",pstDispPubAttr->eIntfType);
        return -1;
    }
    return 0;
}

int sstar_disp_set_rotatemode(MI_DISP_RotateMode_e rotate)
{
    MI_DISP_RotateConfig_t stRotateConfig;
    memset(&stRotateConfig, 0, sizeof(MI_DISP_RotateConfig_t));

    stRotateConfig.eRotateMode      = rotate;
    printf("eRotateMode = %d  \n",stRotateConfig.eRotateMode);
    MI_DISP_SetVideoLayerRotateMode(0, &stRotateConfig);
    if(g_enable_pip)
    {
        MI_DISP_SetVideoLayerRotateMode(1, &stRotateConfig);
    }
    return 0;
}

int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr, int disp_port_num)
{
    disp_port_deinit(disp_port_num);

    switch(pstDispPubAttr->eIntfType) {
#if ENABLE_HDMI
        case E_MI_DISP_INTF_HDMI:
            MI_HDMI_Stop(E_MI_HDMI_ID_0);
            MI_HDMI_Close(E_MI_HDMI_ID_0);
            MI_HDMI_DeInit();
            break;
#else
        case E_MI_DISP_INTF_VGA:
            break;

        case E_MI_DISP_INTF_LCD:
            MI_PANEL_DeInit();
            break;
#endif
        default: break;
    }

    MI_SYS_Exit();
    printf("sstar_disp_Deinit...\n");

    return 0;
}

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

