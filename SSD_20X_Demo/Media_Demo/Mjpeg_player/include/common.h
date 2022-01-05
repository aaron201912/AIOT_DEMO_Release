#ifndef __COMMON__H__
#define __COMMON__H__


#ifdef __cplusplus
        extern "C"{
#endif // __cplusplus

#define ALIGN_BACK(x, a)            (((x) / (a)) * (a))
#define ALIGN_UP(x, a)            (((x+a-1) / (a)) * (a))

#define STCHECKRESULT(result)\
            if (result != MI_SUCCESS)\
            {\
                printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
                return 1;\
            }

int _divp_is_enable;

#define ENABLE_DIVP    1
#define ENABLE_ROTATE_90

#define DIVP_CHN0_ID     0
#define DIVP_CHN1_ID     1

#define DISP_PANEL_W  1024
#define DISP_PANEL_H  600
#define DISP_PANEL_X  0
#define DISP_PANEL_Y  0

char g_mjpeg_file[256];

MI_S32 g_divp_enable;

#define MAX_RESOLURION_W  1920 //DIVP 可以做缩放
#define MAX_RESOLURION_H  1920

#define YUV420  3/2
#define YUV422  2
#define YUV444  3

#define YUV_TYPE  YUV422

#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE MAKE_YUYV_VALUE(29,225,107)

#define TIME_DIFF_PRE_FRAME 33*1000





#ifdef __cplusplus
    }
#endif // __cplusplus

#endif //__COMMON__H__

