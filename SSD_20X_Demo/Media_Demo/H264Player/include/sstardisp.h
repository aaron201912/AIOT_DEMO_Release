#ifndef __SSTARDISP__H__
#define __SSTARDISP__H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <mi_disp_datatype.h>
#ifdef MI_HDMI
#include "mi_hdmi_datatype.h"
#include "mi_hdmi.h"
typedef struct stTimingArray_s
{
    char desc[50];
    MI_DISP_OutputTiming_e eOutputTiming;
    MI_HDMI_TimingType_e eHdmiTiming;
    MI_U16 u16Width;
    MI_U16 u16Height;
}stTimingArray_t;
#endif

#define ENABLE_HDMI_4K      0

int inDispWidth,inDispHeight,OutX,OutY,OutDispWidth,OutDispHeight;


MI_DISP_RotateMode_e bRota;


#define  DISP_PORT_NUM      4
typedef struct ST_Sys_Rect_s
{
    MI_S32 s32X;
    MI_S32 s32Y;
    MI_U16 u16PicW;
    MI_U16 u16PicH;
} ST_Rect_t;
ST_Rect_t stDispWnd[DISP_PORT_NUM], stExtraDispWnd;

int sstar_disp_init(MI_DISP_PubAttr_t* pstDispPubAttr, int disp_port_num);
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr, int disp_port_num);
int sstar_disp_set_rotatemode(MI_DISP_RotateMode_e rotate);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__SSTARDISP__H__
