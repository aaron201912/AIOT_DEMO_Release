#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "mi_sys.h"
#include "sstardisp.h"
#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_divp.h"
#include "common.h"


#include "SAT070CP50_1024x600.h"
//#include "BOE_JD9365DA_10_MIPI.h"
//#include "EK79007_1024x600_MIPI.h"
//#include "ILI9881_800x1280_MIPI.h"
//#include "SAT070AT50_800x480.h"


#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif
int sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_PANEL_LinkType_e eLinkType = E_MI_PNL_LINK_TTL;

    MI_SYS_Init();

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
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
    }

    MI_DISP_SetPubAttr(0, pstDispPubAttr);
    MI_DISP_Enable(0);
    MI_DISP_BindVideoLayer(0, 0);
    MI_DISP_EnableVideoLayer(0);

    MI_DISP_EnableInputPort(0, 0);
    MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        if (MI_PANEL_Init(eLinkType) != MI_SUCCESS)
        {
            return 0;
        }
        MI_PANEL_SetPanelParam(&stPanelParam);
        if (eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
        }
    }
    return 0;
}
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr)
{

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);

    switch (pstDispPubAttr->eIntfType)
    {
    case E_MI_DISP_INTF_HDMI:
        break;

    case E_MI_DISP_INTF_VGA:
        break;

    case E_MI_DISP_INTF_LCD:
    default:
        MI_PANEL_DeInit();
    }

    MI_SYS_Exit();
    printf("sstar_disp_Deinit...\n");

    return 0;
}


void sstar_set_disp_InputPort(int input_x,int input_y,int input_width,int input_height)
{
    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    stInputPortAttr.u16SrcWidth = input_width;
    stInputPortAttr.u16SrcHeight = input_height;

    stInputPortAttr.stDispWin.u16Width = stPanelParam.u16Width;
    stInputPortAttr.stDispWin.u16Height = stPanelParam.u16Height;
    stInputPortAttr.stDispWin.u16X = input_x;
    stInputPortAttr.stDispWin.u16Y = input_y;
    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
}

int sstar_enable_divp(int output_width,int output_height)
{

    if(_divp_is_enable == 1)
    {
        return 0;
    }
    MI_DIVP_OutputPortAttr_t DivpOutputPortAttr = {0};
    MI_SYS_ChnPort_t stSrcChnPort = {0};
    MI_SYS_ChnPort_t stDstChnPort = {0};
    MI_DIVP_ChnAttr_t stDivpChnAttr = {0};

    //init divp
    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = 0;
    stDivpChnAttr.stCropRect.u16Height  = 0;
    stDivpChnAttr.u32MaxWidth           = MAX_RESOLURION_W;
    stDivpChnAttr.u32MaxHeight          = MAX_RESOLURION_H;
    STCHECKRESULT(MI_DIVP_CreateChn(DIVP_CHN0_ID, &stDivpChnAttr));
    STCHECKRESULT(MI_DIVP_StartChn(DIVP_CHN0_ID));

    memset(&DivpOutputPortAttr, 0, sizeof(DivpOutputPortAttr));
    DivpOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    DivpOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    DivpOutputPortAttr.u32Width           = ALIGN_UP(output_width, 2);
    DivpOutputPortAttr.u32Height          = ALIGN_UP(output_height, 2);
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
    _divp_is_enable = 1;
    return 0;
}


void sstar_get_panel_size(MI_U16 *u16Width, MI_U16 *u16Height)
{
    *u16Width = stPanelParam.u16Width;
    *u16Height = stPanelParam.u16Height;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
