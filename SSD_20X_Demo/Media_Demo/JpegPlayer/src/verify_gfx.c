#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <mi_sys_datatype.h>
#include <mi_gfx_datatype.h>
#include "verify_gfx_type.h"
#include "verify_gfx.h"
#include "blitutil.h"

unsigned int getBpp(MI_GFX_ColorFmt_e eFmt)
{
    switch(eFmt) {
        case E_MI_GFX_FMT_I8:
            return 1;

        case E_MI_GFX_FMT_RGB565:
        case E_MI_GFX_FMT_ARGB1555:
        case E_MI_GFX_FMT_ARGB4444:
            return 2;

        case E_MI_GFX_FMT_ARGB8888:
            return 4;

        default:
            return -1;
    }
}

int _gfx_alloc_surface(MI_GFX_Surface_t *pSurf, char **data, char  *name)
{
    char surfName[128] = {0};
    snprintf(surfName, sizeof(surfName), "#%s", name);

    if(MI_SUCCESS != MI_SYS_MMA_Alloc(surfName,
                                      pSurf->u32Height * pSurf->u32Stride, &pSurf->phyAddr)) {
        printf("MI_SYS_MMA_Alloc fail\n");
        return -1;
    }

    if(MI_SUCCESS != MI_SYS_Mmap(pSurf->phyAddr,
                                 pSurf->u32Height * pSurf->u32Stride, (void **)data, FALSE)) {
        printf("MI_SYS_Mmap fail\n");
        return -1;
    }

    //memset(*data, 0, pSurf->u32Height * pSurf->u32Stride);
    return 0;
}
void _gfx_free_surface(MI_GFX_Surface_t *pSurf, char *data)
{
    MI_SYS_Munmap(data, pSurf->u32Height * pSurf->u32Stride);
    MI_SYS_MMA_Free(pSurf->phyAddr);
}
void _gfx_sink_surface(MI_GFX_Surface_t *pSurf,const char *data,const char  *name)
{
    FILE *fp = NULL;
    char sinkName[128] = {0};
    snprintf(sinkName, sizeof(sinkName), "%s_%dx%d.raw", name, pSurf->u32Width, pSurf->u32Height);
    fp = fopen(sinkName, "w+");

    if(fp == NULL) {
        fprintf(stderr, "fp == NULL\n");
    } else {
        const char *p = data;
        long n = pSurf->u32Height * pSurf->u32Stride;

        do {
            long n0 = fwrite(p, 1, n, fp);
            n = n -n0;
            p = data+n0;
        } while(n > 0);

        fclose(fp);
    }

}

int __test_rotate_ARGB_Normal(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    //dstSurf.u32Height = srcSurf.u32Width;
    //dstSurf.u32Width = srcSurf.u32Height;

    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dstY.h = dstSurf.u32Height;
    dstY.w = dstSurf.u32Width;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitNormal(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_ARGB_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                          MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    dstSurf.u32Height = srcSurf.u32Width;
    dstSurf.u32Width = srcSurf.u32Height;

    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(dstSurf.eColorFmt);
    dstY.h = dstSurf.u32Height;
    dstY.w = dstSurf.u32Width;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCW(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}


int __test_rotate_ARGB_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    dstSurf.u32Height = srcSurf.u32Width;
    dstSurf.u32Width = srcSurf.u32Height;


    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    dstY.h = srcY.h;
    dstY.w = srcY.w;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitHVFlip(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_ARGB_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = srcSurf.eColorFmt;
    srcY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = dstSurf.eColorFmt;
    dstY.BytesPerPixel = getBpp(srcSurf.eColorFmt);
    dstY.h = dstSurf.u32Height;
    dstY.w = dstSurf.u32Width;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCCW(&srcY, &dstY, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_YUV420SP_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                              MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.w;
    dstY.w = srcY.h;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCW(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.w;
    dstUV.w = srcUV.h;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitCW(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_YUV420SP_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                               MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.h;
    dstY.w = srcY.w;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitHVFlip(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.h;
    dstUV.w = srcUV.w;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitHVFlip(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}

int __test_rotate_YUV420SP_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                               MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT srcClip;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    srcY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    srcY.BytesPerPixel = 1;
    srcY.h = srcSurf.u32Height;
    srcY.w = srcSurf.u32Width;
    srcY.pitch = srcY.w * srcY.BytesPerPixel;
    srcY.phy_addr = srcSurf.phyAddr;

    dstY.eGFXcolorFmt = E_MI_GFX_FMT_I8;
    dstY.BytesPerPixel = 1;
    dstY.h = srcY.w;
    dstY.w = srcY.h;
    dstY.pitch = dstY.w * dstY.BytesPerPixel;
    dstY.phy_addr = dstSurf.phyAddr;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcY.h;
    srcClip.right = srcY.w;
    SstarBlitCCW(&srcY, &dstY, &srcClip);

    srcUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    srcUV.BytesPerPixel = 2;
    srcUV.h = srcSurf.u32Height / 2;
    srcUV.w = srcSurf.u32Width / 2;
    srcUV.pitch = srcUV.w * srcUV.BytesPerPixel;
    srcUV.phy_addr = srcSurf.phyAddr + srcY.h * srcY.pitch;

    dstUV.eGFXcolorFmt = E_MI_GFX_FMT_ARGB4444;
    dstUV.BytesPerPixel = 2;
    dstUV.h = srcUV.w;
    dstUV.w = srcUV.h;
    dstUV.pitch = dstUV.w * dstUV.BytesPerPixel;
    dstUV.phy_addr = dstSurf.phyAddr + dstY.h * dstY.pitch;

    srcClip.left = 0;
    srcClip.top = 0;
    srcClip.bottom = srcUV.h;
    srcClip.right = srcUV.w;
    SstarBlitCCW(&srcUV, &dstUV, &srcClip);
    JDEC_PERF(ts1, ts2, 1);

    if(bSinkSurf)
        _gfx_sink_surface(&dstSurf, dstData, __FUNCTION__);

    return 0;
}



