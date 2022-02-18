/**
 * @file ss_fbdev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ss_fbdev.h"
#if USE_SS_FBDEV

#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <errno.h>
#include <dlfcn.h>

#include "lv_refr.h"
#include "ss_fbdev_typedef.h"

/*********************
 *      DEFINES
 *********************/
#ifndef FBDEV_PATH
#define FBDEV_PATH  "/dev/fb0"
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ss_fbdev_swap_null(void);

static void ss_fbdev_swap_and_sync(void);

static int __MI_LIB_Load();

static int __MI_SYS_Resolve();

static int __MI_GFX_Resolve();

static MI_S32 _SS_MI_Init(void);

static void *_SS_SYS_MemAlloc(size_t size, MI_PHY *phyAddr);

static void *_SS_SYS_GetMIAddr(size_t size, st_MI_ADDR *miAddr);

static void _SS_SYS_Free(void *pVa, MI_PHY phy, size_t size);

static void _SS_SYS_FreeMIAddr(st_MI_ADDR *miAddr);

#if SS_FBDEV_V3
static MI_PHY _SS_SYS_MemVA2PA(void *pVa);
#endif

static MI_GFX_ColorFmt_e _SS_GFX_GetColor(MI_U32 bits_pp);

static MI_S32 _SS_GFX_QuickFill(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect, MI_U32 u32ColorVal, MI_U16 *pu16Fence);

static MI_S32 _SS_GFX_BitBlit(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
                            MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
                            MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence);

static MI_S32 _SS_GFX_WaitAllDone(MI_BOOL bWaitAllDone, MI_U16 u16TargetFence);

/**********************
 *  STATIC VARIABLES
 **********************/
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static char *fbp = 0;
static MI_U32 fbFrameStride = 0;
static MI_U32 g_u32CurIdx = 0;
static struct
{
    MI_GFX_Surface_t fbSrf;
    char *fbSrfVa;
}fbSrfAttr[2];
static void (*fbswapfunc)(void) = &ss_fbdev_swap_null;
static long int screensize = 0;
static int fbfd = 0;
static lv_coord_t draw_hor_res = 0, draw_ver_res = 0;
static st_MI_ADDR draw_buf[2];

static struct
{
    void *hnd;
}g_stCamOSHnd;

static struct
{
    void *hnd;
}g_stCamFSHnd;

static struct
{
    void *hnd;
    /* MI_SYS */
#if SS_FBDEV_V2
    MI_S32 (*MI_SYS_Init)(void);
    MI_S32 (*MI_SYS_Exit)(void);
    MI_S32 (*MI_SYS_MemsetPa)(MI_PHY phyPa, MI_U32 u32Val, MI_U32 u32Lenth);
    MI_S32 (*MI_SYS_MemcpyPa)(MI_PHY phyDst, MI_PHY phySrc, MI_U32 u32Lenth);
    MI_S32 (*MI_SYS_MMA_Alloc)(MI_U8 *pstMMAHeapName, MI_U32 u32BlkSize, MI_PHY *phyAddr);
    MI_S32 (*MI_SYS_MMA_Free)(MI_PHY phyAddr);
#elif SS_FBDEV_V3
    MI_S32 (*MI_SYS_Init)(MI_U16 u16SocId);
    MI_S32 (*MI_SYS_Exit)(MI_U16 u16SocId);
    MI_S32 (*MI_SYS_MemsetPa)(MI_U16 u16SocId, MI_PHY phyPa, MI_U32 u32Val, MI_U32 u32Lenth);
    MI_S32 (*MI_SYS_MemcpyPa)(MI_U16 u16SocId, MI_PHY phyDst, MI_PHY phySrc, MI_U32 u32Lenth);
    MI_S32 (*MI_SYS_MMA_Alloc)(MI_U16 u16SocId, MI_U8 *pstMMAHeapName, MI_U32 u32BlkSize, MI_PHY *phyAddr);
    MI_S32 (*MI_SYS_MMA_Free)(MI_U16 u16SocId, MI_PHY phyAddr);
    MI_S32 (*MI_SYS_Va2Pa)(void *pVirtualAddress, MI_PHY *pPhyAddr);
#else
#error "Please specify MI version!!!"
#endif
    MI_S32 (*MI_SYS_Mmap)(MI_U64 phyAddr, MI_U32 u32Size, void **ppVirtualAddress, MI_BOOL bCache);
    MI_S32 (*MI_SYS_Munmap)(void *pVirtualAddress, MI_U32 u32Size);
    MI_S32 (*MI_SYS_FlushInvCache)(void *pVirtualAddress, MI_U32 u32Length);
}g_stSysHnd;

static struct
{
    void *hnd;
#if SS_FBDEV_V2
    MI_S32 (*MI_GFX_Open)(void);
    MI_S32 (*MI_GFX_Close)(void);
    MI_S32 (*MI_GFX_WaitAllDone)(MI_BOOL bWaitAllDone, MI_U16 u16TargetFence);
    MI_S32 (*MI_GFX_QuickFill)(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
        MI_U32 u32ColorVal, MI_U16 *pu16Fence);
    MI_S32 (*MI_GFX_DrawLine)(MI_GFX_Surface_t *pstDst, MI_GFX_Line_t *pstLine, MI_U16 *pu16Fence);
    MI_S32 (*MI_GFX_BitBlit)(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
        MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence);
    MI_S32 (*MI_GFX_SetPalette)(MI_GFX_ColorFmt_e eColorFmt, MI_GFX_Palette_t* pstPalette);
    MI_S32 (*MI_GFX_CreateDev)(MI_GFX_DevAttr_t *pstDevAttr);
    MI_S32 (*MI_GFX_DestroyDev)(void);
#elif SS_FBDEV_V3
    MI_GFX_DEV gfxDevId;
    /* MI_GFX */
    MI_S32 (*MI_GFX_Open)(MI_GFX_DEV GfxDevId);
    MI_S32 (*MI_GFX_Close)(MI_GFX_DEV GfxDevId);
    MI_S32 (*MI_GFX_WaitAllDone)(MI_GFX_DEV GfxDevId, MI_BOOL bWaitAllDone, MI_U16 u16TargetFence);
    MI_S32 (*MI_GFX_QuickFill)(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
        MI_U32 u32ColorVal, MI_U16 *pu16Fence);
    MI_S32 (*MI_GFX_DrawLine)(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *pstDst, MI_GFX_Line_t *pstLine, MI_U16 *pu16Fence);
    MI_S32 (*MI_GFX_BitBlit)(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
        MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence);
    MI_S32 (*MI_GFX_SetPalette)(MI_GFX_DEV GfxDevId, MI_GFX_ColorFmt_e eColorFmt, MI_GFX_Palette_t* pstPalette);
    MI_S32 (*MI_GFX_CreateDev)(MI_GFX_DEV GfxDevId, MI_GFX_DevAttr_t *pstDevAttr);
    MI_S32 (*MI_GFX_DestroyDev)(MI_GFX_DEV GfxDevId);
    MI_S32 (*MI_GFX_GetARGB8888To1555AlphaThreshold)(MI_GFX_DEV GfxDevId, MI_U8 *pu8ThresholdValue);
    MI_S32 (*MI_GFX_SetARGB8888To1555AlphaThreshold)(MI_GFX_DEV GfxDevId, MI_U8 u8ThresholdValue);
    MI_S32 (*MI_GFX_GetARGB1555To8888AlphaValue)(MI_GFX_DEV GfxDevId, MI_U8 *pu8AlphaValue);
    MI_S32 (*MI_GFX_SetARGB1555To8888AlphaValue)(MI_GFX_DEV GfxDevId, MI_U8 u8AlphaValue);
    MI_S32 (*MI_GFX_GetInitialScalingCoeff)(MI_GFX_DEV GfxDevId, MI_U16 *pu16ScalingCoefficient);
    MI_S32 (*MI_GFX_SetInitialScalingCoeff)(MI_GFX_DEV GfxDevId, MI_U16 u16ScalingCoefficient);
#endif
}g_stGfxHnd;

/**********************
 *      MACROS
 **********************/
#define DLOPEN(lib, name, attr, ret) \
    do \
    { \
        if(NULL == (lib.hnd = dlopen(name, attr))) \
        { \
            printf("[%s:%d] --- Open library %s fail, %s(%d)\n", __FILE__, __LINE__, name, strerror(errno), errno); \
            return ret; \
        } \
    }while(0);

#define DLSYM(lib, sym, ret) \
    do \
    { \
        if(NULL == (lib.sym = (typeof(lib.sym))dlsym(lib.hnd, #sym))) \
        { \
            printf("[%s:%d] --- Get symbol %s fail, %s(%d)\n", __FILE__, __LINE__, #sym, strerror(errno), errno); \
            return ret; \
        } \
    }while(0);

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void ss_fbdev_init(void)
{
    char *fpbuf = NULL;

    // Open the file for reading and writing
    fbfd = open(FBDEV_PATH, O_RDWR);
    if(fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        return;
    }
    printf("The framebuffer device was opened successfully.\n");

    // Make sure that the display is on.
    if (ioctl(fbfd, FBIOBLANK, FB_BLANK_UNBLANK) != 0) {
        perror("ioctl(FBIOBLANK)");
        return;
    }

    // Get fixed screen information
    if(ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        return;
    }

    // Get variable screen information
    if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        return;
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize =  finfo.smem_len; //finfo.line_length * vinfo.yres;    

    // Map the device to memory
    fpbuf = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if((intptr_t)fpbuf == -1) {
        perror("Error: failed to map framebuffer device to memory");
        return;
    }
    memset(fpbuf, 0, screensize);
    memset(fbSrfAttr, 0, sizeof(fbSrfAttr));
    g_u32CurIdx = 0;
    //fbp = fbSrfAttr[0].fbSrfVa = fpbuf;
    fbSrfAttr[0].fbSrfVa = fpbuf;

    fbFrameStride = finfo.line_length * vinfo.yres;
    fbSrfAttr[0].fbSrf.phyAddr = (MI_PHY)finfo.smem_start;
    fbSrfAttr[0].fbSrf.eColorFmt = _SS_GFX_GetColor(vinfo.bits_per_pixel);
    fbSrfAttr[0].fbSrf.u32Width = vinfo.xres;
    fbSrfAttr[0].fbSrf.u32Height = vinfo.yres;
    fbSrfAttr[0].fbSrf.u32Stride = finfo.line_length;
    if(vinfo.yres_virtual >= vinfo.yres * 2)
    {
        g_u32CurIdx = 1;
        fbswapfunc = &ss_fbdev_swap_and_sync;
        fbSrfAttr[1].fbSrfVa = fpbuf + fbFrameStride;
        fbSrfAttr[1].fbSrf.phyAddr = (MI_PHY)finfo.smem_start + fbFrameStride;
        fbSrfAttr[1].fbSrf.eColorFmt = fbSrfAttr[0].fbSrf.eColorFmt;
        fbSrfAttr[1].fbSrf.u32Width = fbSrfAttr[0].fbSrf.u32Width;
        fbSrfAttr[1].fbSrf.u32Height = fbSrfAttr[0].fbSrf.u32Height;
        fbSrfAttr[1].fbSrf.u32Stride = fbSrfAttr[0].fbSrf.u32Stride;
        /* Drawing at fbSrfAttr[0].fbSrfVa on startup, so display fbSrf 1 first */
        // vinfo.yoffset = vinfo.yres;
        // ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);
    }

    fbp = fbSrfAttr[g_u32CurIdx].fbSrfVa;       // fb为单buf时，直接绘制在buf上；fb为双buf时，绘制在后台buf上

    memset(draw_buf, 0, sizeof(draw_buf));
    _SS_MI_Init();

    printf("The framebuffer device was mapped to memory successfully.\n");

    printf("fb info:\n");
    printf("\tfb_phyAddr: 0x%08lx, %ld\n", finfo.smem_start, finfo.smem_start);
    printf("\tfb_virAddr: %p, %lu\n", fpbuf, (unsigned long)fpbuf);
    printf("\tfb_width:   %d\n", vinfo.xres);
    printf("\tfb_height:  %d\n", vinfo.yres);
    printf("\tfb_stride:  %d\n", finfo.line_length);
    printf("\tfb_format:  %d\n", (int)(_SS_GFX_GetColor(vinfo.bits_per_pixel)));

    printf("\tbuf0_phyAddr:  0x%08llx, %lld\n", fbSrfAttr[0].fbSrf.phyAddr, fbSrfAttr[0].fbSrf.phyAddr);
    printf("\tbuf0_virAddr:  %p, %lu\n", fbSrfAttr[0].fbSrfVa, (unsigned long)fbSrfAttr[0].fbSrfVa);

    printf("\tbuf0_phyAddr:  0x%08llx, %lld\n", fbSrfAttr[1].fbSrf.phyAddr, fbSrfAttr[1].fbSrf.phyAddr);
    printf("\tbuf0_virAddr:  %p, %lu\n", fbSrfAttr[1].fbSrfVa, (unsigned long)fbSrfAttr[1].fbSrfVa);
}

void ss_fbdev_exit(void)
{
    close(fbfd);
}

int ss_fbdev_get_draw_buf(void **draw_buf1, void **draw_buf2, lv_coord_t hor_res, lv_coord_t ver_res, int bpp)
{
    int buf_len = 0;

    if(NULL == draw_buf1 || 0 == hor_res || 0 == ver_res || 0 == bpp)
    {
        return -1;
    }
    buf_len = hor_res * ver_res * bpp;
    draw_hor_res = hor_res;
    draw_ver_res = ver_res;
    if(NULL == (*draw_buf1 = _SS_SYS_GetMIAddr(buf_len, &draw_buf[0])))
    {
        printf("_SS_SYS_GetMIAddr for draw_buf1 fail.\n");
        return -1;
    }
    if(NULL != draw_buf2)
    {
        if(NULL == (*draw_buf2 = _SS_SYS_GetMIAddr(buf_len, &draw_buf[1])))
        {
            printf("_SS_SYS_GetMIAddr for draw_buf2 fail.\n");
            return -1;
        }
    }

    return 0;
}

int ss_fbdev_get_fb_buf(void **buf1, void **buf2)
{
    if(NULL == buf1 || NULL == buf2)
    {
        printf("[%s:%d] --- FATAL, parameter error!\n", __FILE__, __LINE__);
        return -1;
    }

    *buf1 = fbSrfAttr[1].fbSrfVa;
    *buf2 = fbSrfAttr[0].fbSrfVa;

    draw_buf[0].len = fbSrfAttr[1].fbSrf.u32Stride * fbSrfAttr[0].fbSrf.u32Height;
    draw_buf[0].phy = fbSrfAttr[1].fbSrf.phyAddr;
    draw_buf[0].pVa = fbSrfAttr[1].fbSrfVa;

    draw_buf[1].len = fbSrfAttr[0].fbSrf.u32Stride * fbSrfAttr[1].fbSrf.u32Height;
    draw_buf[1].phy = fbSrfAttr[0].fbSrf.phyAddr;
    draw_buf[1].pVa = fbSrfAttr[0].fbSrfVa;

    draw_hor_res = fbSrfAttr[0].fbSrf.u32Width;
    draw_ver_res = fbSrfAttr[0].fbSrf.u32Height;

    return 0;
}

void ss_fbdev_fill(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    MI_GFX_Surface_t *pstSrf;
    MI_GFX_Rect_t stRect;
    MI_U32 u32ColorVal;
    MI_U16 u16Fence;

    pstSrf = &fbSrfAttr[g_u32CurIdx].fbSrf;
    stRect.s32Xpos = fill_area->x1;
    stRect.s32Ypos = fill_area->y1;
    stRect.u32Width = fill_area->x2 - fill_area->x1 + 1;
    stRect.u32Height = fill_area->y2 - fill_area->y1 + 1;
    u32ColorVal = *(uint32_t *)&color;
    _SS_GFX_QuickFill(pstSrf, &stRect, u32ColorVal, &u16Fence);
    _SS_GFX_WaitAllDone(false, u16Fence);

    return;
}

void ss_fbdev_fill_ex(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    MI_GFX_Surface_t stDstSrf;
    MI_GFX_Rect_t stRect;
    MI_U32 u32ColorVal;
    MI_U16 u16Fence;

    stDstSrf.phyAddr = draw_buf[0].phy;
    stDstSrf.eColorFmt = fbSrfAttr[g_u32CurIdx].fbSrf.eColorFmt;
    stDstSrf.u32Width = draw_hor_res;
    stDstSrf.u32Height = draw_ver_res;
    stDstSrf.u32Stride = dest_width * sizeof(lv_color_t);
    stRect.s32Xpos = fill_area->x1;
    stRect.s32Ypos = fill_area->y1;
    stRect.u32Width = fill_area->x2 - fill_area->x1 + 1;
    stRect.u32Height = fill_area->y2 - fill_area->y1 + 1;
    u32ColorVal = *(uint32_t *)&color;
    _SS_GFX_QuickFill(&stDstSrf, &stRect, u32ColorVal, &u16Fence);
    _SS_GFX_WaitAllDone(false, u16Fence);

    return;
}

void ss_fbdev_fill_onebuf(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    MI_GFX_Surface_t stDstSrf;
    MI_GFX_Rect_t stRect;
    MI_U32 u32ColorVal;
    MI_U16 u16Fence;

    stDstSrf.phyAddr = draw_buf[0].phy;
    stDstSrf.eColorFmt = fbSrfAttr[g_u32CurIdx].fbSrf.eColorFmt;
    stDstSrf.u32Width = draw_hor_res;
    stDstSrf.u32Height = draw_ver_res;
    stDstSrf.u32Stride = dest_width * sizeof(lv_color_t);
    stRect.s32Xpos = fill_area->x1;
    stRect.s32Ypos = fill_area->y1;
    stRect.u32Width = fill_area->x2 - fill_area->x1 + 1;
    stRect.u32Height = fill_area->y2 - fill_area->y1 + 1;
    u32ColorVal = *(uint32_t *)&color;
    _SS_GFX_QuickFill(&stDstSrf, &stRect, u32ColorVal, &u16Fence);
    _SS_GFX_WaitAllDone(false, u16Fence);

    return;
}

void ss_fbdev_fill_full_onebuf(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    MI_GFX_Surface_t stDstSrf;
    MI_GFX_Rect_t stRect;
    MI_U32 u32ColorVal;
    MI_U16 u16Fence;

    stDstSrf.phyAddr = draw_buf[0].phy;
    stDstSrf.eColorFmt = fbSrfAttr[g_u32CurIdx].fbSrf.eColorFmt;
    stDstSrf.u32Width = draw_hor_res;
    stDstSrf.u32Height = draw_ver_res;
    stDstSrf.u32Stride = dest_width * sizeof(lv_color_t);
    stRect.s32Xpos = fill_area->x1;
    stRect.s32Ypos = fill_area->y1;
    stRect.u32Width = fill_area->x2 - fill_area->x1 + 1;
    stRect.u32Height = fill_area->y2 - fill_area->y1 + 1;
    u32ColorVal = *(uint32_t *)&color;
    _SS_GFX_QuickFill(&stDstSrf, &stRect, u32ColorVal, &u16Fence);
    _SS_GFX_WaitAllDone(false, u16Fence);

    return;
}

void ss_fbdev_fill_directly(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    MI_GFX_Surface_t stDstSrf;
    MI_GFX_Rect_t stRect;
    MI_U32 u32ColorVal;
    MI_U16 u16Fence;

    stDstSrf.phyAddr = fbSrfAttr[g_u32CurIdx].fbSrf.phyAddr;    //draw_buf[0].phy;
    stDstSrf.eColorFmt = fbSrfAttr[g_u32CurIdx].fbSrf.eColorFmt;
    stDstSrf.u32Width = draw_hor_res;
    stDstSrf.u32Height = draw_ver_res;
    stDstSrf.u32Stride = dest_width * sizeof(lv_color_t);
    stRect.s32Xpos = fill_area->x1;
    stRect.s32Ypos = fill_area->y1;
    stRect.u32Width = fill_area->x2 - fill_area->x1 + 1;
    stRect.u32Height = fill_area->y2 - fill_area->y1 + 1;
    u32ColorVal = *(uint32_t *)&color;

    _SS_GFX_QuickFill(&stDstSrf, &stRect, u32ColorVal, &u16Fence);
    _SS_GFX_WaitAllDone(false, u16Fence);

    return;
}

/**
 * Flush a buffer to the marked area
 * @param drv pointer to driver where this function belongs
 * @param area an area where to copy `color_p`
 * @param color_p an array of pixel to copy to the `area` part of the screen
 */
void ss_fbdev_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(fbp == NULL ||
            area->x2 < 0 ||
            area->y2 < 0 ||
            area->x1 > (int32_t)vinfo.xres - 1 ||
            area->y1 > (int32_t)vinfo.yres - 1) {
        lv_disp_flush_ready(drv);
        return;
    }

    /*Truncate the area to the screen*/
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > (int32_t)vinfo.xres - 1 ? (int32_t)vinfo.xres - 1 : area->x2;
    int32_t act_y2 = area->y2 > (int32_t)vinfo.yres - 1 ? (int32_t)vinfo.yres - 1 : area->y2;

    lv_coord_t w = (act_x2 - act_x1 + 1);
    long int location = 0;
    long int byte_location = 0;
    unsigned char bit_location = 0;

    /*32 or 24 bit per pixel*/
    if(vinfo.bits_per_pixel == 32 || vinfo.bits_per_pixel == 24) {
        uint32_t * fbp32 = (uint32_t *)fbp;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            location = (act_x1 + vinfo.xoffset) + (y + vinfo.yoffset) * finfo.line_length / 4;
            memcpy(&fbp32[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1) * 4);
            color_p += w;
        }
    }
    /*16 bit per pixel*/
    else if(vinfo.bits_per_pixel == 16) {
        uint16_t * fbp16 = (uint16_t *)fbp;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            location = (act_x1 + vinfo.xoffset) + (y + vinfo.yoffset) * finfo.line_length / 2;
            memcpy(&fbp16[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1) * 2);
            color_p += w;
        }
    }
    /*8 bit per pixel*/
    else if(vinfo.bits_per_pixel == 8) {
        uint8_t * fbp8 = (uint8_t *)fbp;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            location = (act_x1 + vinfo.xoffset) + (y + vinfo.yoffset) * finfo.line_length;
            memcpy(&fbp8[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1));
            color_p += w;
        }
    }
    /*1 bit per pixel*/
    else if(vinfo.bits_per_pixel == 1) {
        uint8_t * fbp8 = (uint8_t *)fbp;
        int32_t x;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            for(x = act_x1; x <= act_x2; x++) {
                location = (x + vinfo.xoffset) + (y + vinfo.yoffset) * vinfo.xres;
                byte_location = location / 8; /* find the byte we need to change */
                bit_location = location % 8; /* inside the byte found, find the bit we need to change */
                fbp8[byte_location] &= ~(((uint8_t)(1)) << bit_location);
                fbp8[byte_location] |= ((uint8_t)(color_p->full)) << bit_location;
                color_p++;
            }

            color_p += area->x2 - act_x2;
        }
    } else {
        /*Not supported bit per pixel*/
    }

    //May be some direct update command is required
    //ret = ioctl(state->fd, FBIO_UPDATE, (unsigned long)((uintptr_t)rect));

    if(lv_disp_flush_is_last(drv))
    {
        fbswapfunc();
    #if 0
        static long long fno = 0;
        printf("Flush frame %lld done!!!\n", fno++);
    #endif
    }

    lv_disp_flush_ready(drv);
}

void ss_fbdev_flush_ex(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
#if 0
    uint32_t i;
    MI_GFX_Surface_t *pstSrc, *pstDst;
    MI_GFX_Rect_t stRect;
    MI_GFX_Opt_t stGfxOpt;
    MI_U16 u16Fence;
    lv_disp_t * disp = _lv_refr_get_disp_refreshing();

    /* Swap */
    vinfo.yoffset = g_u32CurIdx ? vinfo.yres : 0;
    pstSrc = &fbSrfAttr[g_u32CurIdx].fbSrf;
    g_u32CurIdx = (0 == g_u32CurIdx);
    pstDst = &fbSrfAttr[g_u32CurIdx].fbSrf;
    fbp = fbSrfAttr[g_u32CurIdx].fbSrfVa;
    ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);

    /* Sync */
    memset(&stGfxOpt, 0, sizeof(MI_GFX_Opt_t));
    stGfxOpt.stClipRect.s32Xpos = 0;
    stGfxOpt.stClipRect.s32Ypos = 0;
    stGfxOpt.stClipRect.u32Width = vinfo.xres;
    stGfxOpt.stClipRect.u32Height = vinfo.yres;
    stGfxOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stGfxOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    for(i = 0; i < disp->inv_p; i++)
    {
        if(0 != disp->inv_area_joined[i])
        {
            continue;
        }
        stRect.s32Xpos = disp->inv_areas[i].x1;
        stRect.s32Ypos = disp->inv_areas[i].y1;
        stRect.u32Width = disp->inv_areas[i].x2 - disp->inv_areas[i].x1 + 1;
        stRect.u32Height = disp->inv_areas[i].y2 - disp->inv_areas[i].y1 + 1;
        //stGfxOpt.stClipRect = stRect;
        _SS_GFX_BitBlit(pstSrc, &stRect, pstDst, &stRect, &stGfxOpt, &u16Fence);
    }
    _SS_GFX_WaitAllDone(true, 0);
#else
    MI_PHY srcPhy, dstPhy;

    vinfo.yoffset = g_u32CurIdx ? vinfo.yres : 0;
    srcPhy = draw_buf[0].phy;
    /* Swap */
    dstPhy = fbSrfAttr[g_u32CurIdx].fbSrf.phyAddr;
    g_u32CurIdx = (0 == g_u32CurIdx);
    fbp = fbSrfAttr[g_u32CurIdx].fbSrfVa;
    /* Sync, just copy the whole buffer */
#if SS_FBDEV_V2
    g_stSysHnd.MI_SYS_MemcpyPa(dstPhy, srcPhy, fbFrameStride);
#elif SS_FBDEV_V3
    g_stSysHnd.MI_SYS_MemcpyPa(0, dstPhy, srcPhy, fbFrameStride);
#endif
    ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);
#endif

#if 0
    static long long fno = 0;
    printf("Flush frame %lld done!!!\n", fno++);
#endif
    lv_disp_flush_ready(drv);

    return;
}

void ss_fbdev_flush_onebuf(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(fbp == NULL ||
            area->x2 < 0 ||
            area->y2 < 0 ||
            area->x1 > (int32_t)vinfo.xres - 1 ||
            area->y1 > (int32_t)vinfo.yres - 1) {
        lv_disp_flush_ready(drv);
        return;
    }

    /*Truncate the area to the screen*/
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > (int32_t)vinfo.xres - 1 ? (int32_t)vinfo.xres - 1 : area->x2;
    int32_t act_y2 = area->y2 > (int32_t)vinfo.yres - 1 ? (int32_t)vinfo.yres - 1 : area->y2;

    lv_coord_t w = (act_x2 - act_x1 + 1);
    long int location = 0;
    long int byte_location = 0;
    unsigned char bit_location = 0;

    /*32 or 24 bit per pixel*/
    if(vinfo.bits_per_pixel == 32 || vinfo.bits_per_pixel == 24) {
        uint32_t * fbp32 = (uint32_t *)fbp;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            location = (act_x1 + vinfo.xoffset) + (y + vinfo.yoffset) * finfo.line_length / 4;
            memcpy(&fbp32[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1) * 4);
            color_p += w;
        }
    }
    /*16 bit per pixel*/
    else if(vinfo.bits_per_pixel == 16) {
        uint16_t * fbp16 = (uint16_t *)fbp;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            location = (act_x1 + vinfo.xoffset) + (y + vinfo.yoffset) * finfo.line_length / 2;
            memcpy(&fbp16[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1) * 2);
            color_p += w;
        }
    }
    /*8 bit per pixel*/
    else if(vinfo.bits_per_pixel == 8) {
        uint8_t * fbp8 = (uint8_t *)fbp;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            location = (act_x1 + vinfo.xoffset) + (y + vinfo.yoffset) * finfo.line_length;
            memcpy(&fbp8[location], (uint32_t *)color_p, (act_x2 - act_x1 + 1));
            color_p += w;
        }
    }
    /*1 bit per pixel*/
    else if(vinfo.bits_per_pixel == 1) {
        uint8_t * fbp8 = (uint8_t *)fbp;
        int32_t x;
        int32_t y;
        for(y = act_y1; y <= act_y2; y++) {
            for(x = act_x1; x <= act_x2; x++) {
                location = (x + vinfo.xoffset) + (y + vinfo.yoffset) * vinfo.xres;
                byte_location = location / 8; /* find the byte we need to change */
                bit_location = location % 8; /* inside the byte found, find the bit we need to change */
                fbp8[byte_location] &= ~(((uint8_t)(1)) << bit_location);
                fbp8[byte_location] |= ((uint8_t)(color_p->full)) << bit_location;
                color_p++;
            }

            color_p += area->x2 - act_x2;
        }
    } else {
        /*Not supported bit per pixel*/
    }

    //May be some direct update command is required
    //ret = ioctl(state->fd, FBIO_UPDATE, (unsigned long)((uintptr_t)rect));

    if(lv_disp_flush_is_last(drv))
    {
        fbswapfunc();
    }

    lv_disp_flush_ready(drv);

    return;
}

void ss_fbdev_flush_full_onebuf(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    MI_PHY srcPhy, dstPhy;

    vinfo.yoffset = g_u32CurIdx ? vinfo.yres : 0;
    srcPhy = draw_buf[0].phy;
    /* Swap */
    dstPhy = fbSrfAttr[g_u32CurIdx].fbSrf.phyAddr;
    g_u32CurIdx = (0 == g_u32CurIdx);
    fbp = fbSrfAttr[g_u32CurIdx].fbSrfVa;       // next fb draw buffer

    /* Sync, just copy the whole buffer */
#if SS_FBDEV_V2
    //g_stSysHnd.MI_SYS_MemcpyPa(dstPhy, srcPhy, draw_buf[0].len);
    g_stSysHnd.MI_SYS_MemcpyPa(dstPhy, srcPhy, fbFrameStride);
#elif SS_FBDEV_V3
    //g_stSysHnd.MI_SYS_MemcpyPa(0, dstPhy, srcPhy, draw_buf[0].len);
    g_stSysHnd.MI_SYS_MemcpyPa(0, dstPhy, srcPhy, fbFrameStride);
#endif
    ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);

    lv_disp_flush_ready(drv);

    return;
}

void ss_fbdev_flush_directly(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    MI_PHY frontPhy, backPhy;

    //printf("flush ----------------\n");
    vinfo.yoffset = g_u32CurIdx ? vinfo.yres : 0;
    //srcPhy = draw_buf[0].phy;
    /* Swap */
    ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);

    frontPhy = fbSrfAttr[g_u32CurIdx].fbSrf.phyAddr;
    g_u32CurIdx = (0 == g_u32CurIdx);
    fbp = fbSrfAttr[g_u32CurIdx].fbSrfVa;

    backPhy = fbSrfAttr[g_u32CurIdx].fbSrf.phyAddr;
    /* Sync, just copy the whole buffer */
#if SS_FBDEV_V2
    g_stSysHnd.MI_SYS_MemcpyPa(backPhy, frontPhy, fbFrameStride);
#elif SS_FBDEV_V3
    g_stSysHnd.MI_SYS_MemcpyPa(0, backPhy, frontPhy, fbFrameStride);
#endif
    

#if 0
    static long long fno = 0;
    printf("Flush frame %lld done!!!\n", fno++);
#endif
    lv_disp_flush_ready(drv);

    return;
}

void ss_fbdev_get_sizes(uint32_t *width, uint32_t *height) {
    if (width)
        *width = vinfo.xres;

    if (height)
        *height = vinfo.yres;
}

static void ss_fbdev_swap_null(void)
{
    return;
}

#if 1
static void ss_fbdev_swap_and_sync(void)
{
    MI_PHY srcPhy, dstPhy;

    /* Swap */
    vinfo.yoffset = g_u32CurIdx ? vinfo.yres : 0;
    srcPhy = fbSrfAttr[g_u32CurIdx].fbSrf.phyAddr;
    g_u32CurIdx = (0 == g_u32CurIdx);
    dstPhy = fbSrfAttr[g_u32CurIdx].fbSrf.phyAddr;
    fbp = fbSrfAttr[g_u32CurIdx].fbSrfVa;
    ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);

    /* Sync */
#if SS_FBDEV_V2
    g_stSysHnd.MI_SYS_MemcpyPa(dstPhy, srcPhy, fbFrameStride);
#elif SS_FBDEV_V3
    g_stSysHnd.MI_SYS_MemcpyPa(0, dstPhy, srcPhy, fbFrameStride);
#endif

    return;
}
#else
static void ss_fbdev_swap_and_sync(void)
{
    MI_GFX_Surface_t *pstSrc, *pstDst;
    MI_GFX_Rect_t stSrcRect, stDstRect;
    MI_GFX_Opt_t stGfxOpt = {0};
    MI_U16 fence = 0;

    /* Swap */
    vinfo.yoffset = g_u32CurIdx ? vinfo.yres : 0;
    pstSrc = &fbSrfAttr[g_u32CurIdx].fbSrf;
    g_u32CurIdx = (0 == g_u32CurIdx);
    pstDst = &fbSrfAttr[g_u32CurIdx].fbSrf;
    fbp = fbSrfAttr[g_u32CurIdx].fbSrfVa;
    ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo);

    /* Sync */
    memset(&stSrcRect, 0, sizeof(MI_GFX_Rect_t));
    stSrcRect.s32Xpos = 0;
    stSrcRect.s32Ypos = 0;
    stSrcRect.u32Width = vinfo.xres;
    stSrcRect.u32Height = vinfo.yres;
    memset(&stDstRect, 0, sizeof(MI_GFX_Rect_t));
    stDstRect.s32Xpos = 0;
    stDstRect.s32Ypos = 0;
    stDstRect.u32Width = vinfo.xres;
    stDstRect.u32Height = vinfo.yres;
    memset(&stGfxOpt, 0, sizeof(MI_GFX_Opt_t));
    stGfxOpt.stClipRect = stDstRect;
    stGfxOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stGfxOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    _SS_GFX_BitBlit(pstSrc, &stSrcRect, pstDst, &stDstRect, &stGfxOpt, &fence);
    _SS_GFX_WaitAllDone(false, fence);

    return;
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/
static int __MI_LIB_Load()
{
    int ret = -1, flags = 0;

    flags = RTLD_NOW | RTLD_GLOBAL;
    DLOPEN(g_stCamOSHnd, "libcam_os_wrapper.so", flags, ret);
    DLOPEN(g_stCamFSHnd, "libcam_fs_wrapper.so", flags, ret);
    DLOPEN(g_stSysHnd, "libmi_sys.so", flags, ret);
    DLOPEN(g_stGfxHnd, "libmi_gfx.so", flags, ret);

    return 0;
}

static int __MI_SYS_Resolve()
{
    int ret = -1;

    DLSYM(g_stSysHnd, MI_SYS_Init, ret);
    DLSYM(g_stSysHnd, MI_SYS_Exit, ret);
    DLSYM(g_stSysHnd, MI_SYS_MemsetPa, ret);
    DLSYM(g_stSysHnd, MI_SYS_MemcpyPa, ret);
    DLSYM(g_stSysHnd, MI_SYS_MMA_Alloc, ret);
    DLSYM(g_stSysHnd, MI_SYS_MMA_Free, ret);
    DLSYM(g_stSysHnd, MI_SYS_Mmap, ret);
    DLSYM(g_stSysHnd, MI_SYS_Munmap, ret);
    DLSYM(g_stSysHnd, MI_SYS_FlushInvCache, ret);
#if SS_FBDEV_V3
    DLSYM(g_stSysHnd, MI_SYS_Va2Pa, ret);
#endif

    return 0;
}

static int __MI_GFX_Resolve()
{
    int ret = -1;

    DLSYM(g_stGfxHnd, MI_GFX_Open, ret);
    DLSYM(g_stGfxHnd, MI_GFX_Close, ret);
    DLSYM(g_stGfxHnd, MI_GFX_WaitAllDone, ret);
    DLSYM(g_stGfxHnd, MI_GFX_QuickFill, ret);
    DLSYM(g_stGfxHnd, MI_GFX_DrawLine, ret);
    DLSYM(g_stGfxHnd, MI_GFX_BitBlit, ret);
    DLSYM(g_stGfxHnd, MI_GFX_SetPalette, ret);
#if SS_FBDEV_V3
    DLSYM(g_stGfxHnd, MI_GFX_CreateDev, ret);
    DLSYM(g_stGfxHnd, MI_GFX_DestroyDev, ret);
    DLSYM(g_stGfxHnd, MI_GFX_GetARGB8888To1555AlphaThreshold, ret);
    DLSYM(g_stGfxHnd, MI_GFX_SetARGB8888To1555AlphaThreshold, ret);
    DLSYM(g_stGfxHnd, MI_GFX_GetARGB1555To8888AlphaValue, ret);
    DLSYM(g_stGfxHnd, MI_GFX_SetARGB1555To8888AlphaValue, ret);
    DLSYM(g_stGfxHnd, MI_GFX_GetInitialScalingCoeff, ret);
    DLSYM(g_stGfxHnd, MI_GFX_SetInitialScalingCoeff, ret);

    g_stGfxHnd.gfxDevId = 0;
#endif

    return 0;
}

static MI_S32 _SS_MI_Init(void)
{
    if(0 != __MI_LIB_Load())
    {
        printf("[%s:%d] --- _MI_LIB_Load fail\n", __FILE__, __LINE__);
        return -1;
    }
    if(0 != __MI_SYS_Resolve() || 0 != __MI_GFX_Resolve())
    {
        printf("[%s:%d] --- _MI_SYS_Resolve or _MI_GFX_Resolve resolve fail\n", __FILE__, __LINE__);
        return -1;
    }

#if SS_FBDEV_V2
    g_stSysHnd.MI_SYS_Init();
    g_stGfxHnd.MI_GFX_Open();
#elif SS_FBDEV_V3
    g_stSysHnd.MI_SYS_Init(0);
    g_stGfxHnd.MI_GFX_Open(g_stGfxHnd.gfxDevId);
#endif

    return MI_SUCCESS;
}

static void *_SS_SYS_MemAlloc(size_t size, MI_PHY *phyAddr)
{
    MI_S32 ret = -1;
    MI_PHY phy;
    void *pVa = NULL;

#if SS_FBDEV_V2
    ret = g_stSysHnd.MI_SYS_MMA_Alloc(NULL, size, &phy);
#elif SS_FBDEV_V3
    ret = g_stSysHnd.MI_SYS_MMA_Alloc(0, NULL, size, &phy);
#endif
    if(MI_SUCCESS != ret)
    {
        printf("[%s:%d] --- MI_SYS_MMA_Alloc fail!\n", __FILE__, __LINE__);
        return NULL;
    }
    if(MI_SUCCESS != g_stSysHnd.MI_SYS_Mmap(phy, size, &pVa, false))
    {
        printf("[%s:%d] --- MI_SYS_Mmap fail!\n", __FILE__, __LINE__);
    #if SS_FBDEV_V2
        g_stSysHnd.MI_SYS_MMA_Free(phy);
    #elif SS_FBDEV_V3
        g_stSysHnd.MI_SYS_MMA_Free(0, phy);
    #endif
        return NULL;
    }
    if(NULL != phyAddr)
    {
        *phyAddr = phy;
    }

    return pVa;
}

static void *_SS_SYS_GetMIAddr(size_t size, st_MI_ADDR *miAddr)
{
    if(0 == size || NULL == miAddr)
    {
        return NULL;
    }
    miAddr->len = size;
    miAddr->pVa = _SS_SYS_MemAlloc(size, &miAddr->phy);

    return miAddr->pVa;
}

static void _SS_SYS_Free(void *pVa, MI_PHY phy, size_t size)
{
    if(NULL == pVa || 0 == size)
    {
        return;
    }

    g_stSysHnd.MI_SYS_Munmap(pVa, size);
#if SS_FBDEV_V2
    g_stSysHnd.MI_SYS_MMA_Free(phy);
#elif SS_FBDEV_V3
    g_stSysHnd.MI_SYS_MMA_Free(0, phy);
#endif

    return;
}

static void _SS_SYS_FreeMIAddr(st_MI_ADDR *miAddr)
{
    _SS_SYS_Free(miAddr->pVa, miAddr->phy, miAddr->len);
    return;
}

#if SS_FBDEV_V3
static MI_PHY _SS_SYS_MemVA2PA(void *pVa)
{
    MI_PHY phy;

    if(MI_SUCCESS != g_stSysHnd.MI_SYS_Va2Pa(pVa, &phy))
    {
        return 0;
    }

    return phy;
}
#endif

static MI_GFX_ColorFmt_e _SS_GFX_GetColor(MI_U32 bits_pp)
{
    MI_GFX_ColorFmt_e color = E_MI_GFX_FMT_MAX;

    switch(bits_pp)
    {
        case 1:
        case 8:
        case 16:
        {
            /* Fix me */
            break;
        }
        case 32:
        {
            /* Fix me */
            color = E_MI_GFX_FMT_ARGB8888;
            break;
        }
    }

    return color;
}

static MI_S32 _SS_GFX_QuickFill(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect, MI_U32 u32ColorVal, MI_U16 *pu16Fence)
{
    MI_S32 s32Ret = MI_SUCCESS;
#if SS_FBDEV_V2
    s32Ret = g_stGfxHnd.MI_GFX_QuickFill(pstDst, pstDstRect, u32ColorVal, pu16Fence);
#elif SS_FBDEV_V3
    s32Ret = g_stGfxHnd.MI_GFX_QuickFill(g_stGfxHnd.gfxDevId, pstDst, pstDstRect, u32ColorVal, pu16Fence);
#endif
    if(MI_SUCCESS != s32Ret)
    {
        printf("[%s:%d] --- MI_GFX_QuickFill fail!, ret=%x\n", __FILE__, __LINE__, s32Ret);
        return -1;
    }

    return 0;
}

static MI_S32 _SS_GFX_BitBlit(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
                            MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
                            MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence)
{
#if 0
    printf("[%s:%d] --- _SS_GFX_BitBlit start, src[%lld] - dst[%lld]\n", __FILE__, __LINE__, pstSrc->phyAddr, pstDst->phyAddr);
    printf("[%s:%d] --- _SS_GFX_BitBlit src(%d, %d), dst(%d, %d)\n", __FILE__, __LINE__, pstSrc->u32Width, pstSrc->u32Height, pstDst->u32Width, pstDst->u32Height);
    printf("[%s:%d] --- _SS_GFX_BitBlit srcRect[(%d, %d)@(%d, %d)]\n", __FILE__, __LINE__, pstSrcRect->u32Width, pstSrcRect->u32Height, pstSrcRect->s32Xpos, pstSrcRect->s32Ypos);
    printf("[%s:%d] --- _SS_GFX_BitBlit dstRect[(%d, %d)@(%d, %d)]\n", __FILE__, __LINE__, pstDstRect->u32Width, pstDstRect->u32Height, pstDstRect->s32Xpos, pstDstRect->s32Ypos);
#endif

#if SS_FBDEV_V2
    if(MI_SUCCESS != g_stGfxHnd.MI_GFX_BitBlit(pstSrc, pstSrcRect, pstDst, pstDstRect, pstOpt, pu16Fence))
#elif SS_FBDEV_V3
    if(MI_SUCCESS != g_stGfxHnd.MI_GFX_BitBlit(g_stGfxHnd.gfxDevId, pstSrc, pstSrcRect, pstDst, pstDstRect, pstOpt, pu16Fence))
#endif
    {
        printf("[%s:%d] --- MI_GFX_BitBlit fail!\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

static MI_S32 _SS_GFX_WaitAllDone(MI_BOOL bWaitAllDone, MI_U16 u16TargetFence)
{
#if SS_FBDEV_V2
    return g_stGfxHnd.MI_GFX_WaitAllDone(bWaitAllDone, u16TargetFence);
#elif SS_FBDEV_V3
    return g_stGfxHnd.MI_GFX_WaitAllDone(g_stGfxHnd.gfxDevId, bWaitAllDone, u16TargetFence);
#endif
}

#endif /*USE_SS_FBDEV*/

