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

int sstar_disp_init(MI_DISP_PubAttr_t* pstDispPubAttr);
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr);
void sstar_set_disp_InputPort(int input_x,int input_y,int input_width,int input_height);
int sstar_enable_divp(int output_width,int output_height);
void sstar_get_panel_size(MI_U16 *u16Width, MI_U16 *u16Height);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__SSTARDISP__H__
