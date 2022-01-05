#ifndef _FB_COMMON_H
#define _FB_COMMON_H

#include <linux/fb.h>
#include "mi_common_datatype.h"
#include "sstarFb.h"

void fb_Tc_Fill_Rect(int, int, int, int, int);
void fb_Tc_Fill_Buffer(char *, unsigned int, unsigned int);
void fb_Tc_Begin_Batch_Draw();
void fb_Tc_End_Batch_Draw();

void fb_Tc_Print_Fix_Info();
void fb_Tc_Print_Var_Info();
void fb_Tc_Print_Rect(const MI_FB_Rectangle_t *rect);
void fb_Tc_Print_Color_Key(const MI_FB_ColorKey_t *key);
void fb_Tc_Print_Disp_Attr(const MI_FB_DisplayLayerAttr_t *attr);

void fb_Tc_Init(const char *devfile, MI_BOOL bDbuf);
void fb_Tc_Deinit();

void fb_Tc_Get_Fix_Info(struct fb_fix_screeninfo *pfinfo);
void fb_Tc_Get_Var_Info(struct fb_var_screeninfo *pvinfo);
void fb_Tc_Set_Var_Info(const struct fb_var_screeninfo *pvinfo);

void Fb_Tc_Pandisplay();

void fb_Tc_Get_Show(MI_BOOL *bShow);
void fb_Tc_Set_Show(const MI_BOOL *bShow);

void fb_Tc_Get_ScreenLocation(MI_FB_Rectangle_t *pRect);
void fb_Tc_Set_ScreenLocation(const MI_FB_Rectangle_t *pRect);

void fb_Tc_Get_GlobalAlpha(MI_FB_GlobalAlpha_t *pAlpha);
void fb_Tc_Set_GlobalAlpha(const MI_FB_GlobalAlpha_t *pAlpha);

void fb_Tc_Get_DispAttr(MI_FB_DisplayLayerAttr_t *pCursorAttr);
void fb_Tc_Set_DispAttr(const MI_FB_DisplayLayerAttr_t *pCursorAttr);

void fb_Tc_Get_CursorAttr(MI_FB_CursorAttr_t *pCursorAttr);
void fb_Tc_Set_CursorAttr(const MI_FB_CursorAttr_t *pCursorAttr);

void fb_Tc_Get_ColorKey(MI_FB_ColorKey_t *pColorKey);
void fb_Tc_Set_ColorKey(const MI_FB_ColorKey_t *pColorKey);

#endif /* _FB_COMMON_H */
