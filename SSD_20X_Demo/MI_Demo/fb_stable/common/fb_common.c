#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include "fb_common.h"
#include "sstarFb.h"

typedef struct Rect
{
    int x;
    int y;
    int w;
    int h;
    int color;
}Rect_t;

// fb common
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo;
static char *framebuffer = NULL;
static int fd;
static struct fb_cmap *cmap = NULL;

// double buffer
static char *curr_buffer = NULL;
static Rect_t dirty_rect;
static int batch_mode = 0;
static pthread_t tid;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t cond_flush = PTHREAD_COND_INITIALIZER;
static unsigned char _bUseDoubleBuffer = 1;
static int refreshRunning = 1;

#define ERR_EXIT(str) \
    do {\
        printf("----> err exit: ");\
        perror(str);\
        exit(EXIT_FAILURE);\
    } while(0)

/////////////////////////////////////////////////////////////////
// Macro to convert color from ARGB8888 to others.
// @fmt Target color format
// @src Source color value
// @dest Target color value
// @cnt (Output) pixel count per color
/////////////////////////////////////////////////////////////////
#define __CONVERT_COLOR_FROM_ARGB8888(fmt, src, dest, cnt)\
    do {\
    int r, g, b, a;\
    int p0, p1;\
    switch (fmt)\
    {\
        case E_MI_FB_COLOR_FMT_RGB565:\
        {\
            r = (src & 0xff0000) >> (16 + 3);\
            g = (src & 0xff00) >> (8 + 2);\
            b = (src & 0xff) >> 3;\
            dest = b | (g << 5) | (r << (5 + 6));\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_ARGB4444:\
        {\
            a = (src & 0xff000000) >> (24 + 4);\
            r = (src & 0xff0000) >> (16 + 4);\
            g = (src & 0xff00) >> (8 + 4);\
            b = (src & 0xff) >> 4;\
            dest = b | (g << 4) | (r << 8) | (a << 12);\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_ARGB8888:\
        {\
            dest = src;\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_ARGB1555:\
        {\
            a = (src & 0xff000000) >> (24 + 7);\
            r = (src & 0xff0000) >> (16 + 3);\
            g = (src & 0xff00) >> (8 + 3);\
            b = (src & 0xff) >> 3;\
            dest = b | (g << 5) | (r << 10) | (a << 15);\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_YUV422:\
        {\
            p0 = (src >> 24) | ((src >> 16 & 0x00ff) << 8);\
            p1 = (src >> 24) | ((char)src) << 8;\
            dest = p0 | p1 << 16;\
            cnt = 2;\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_I8:\
        {\
            dest = src & 0x000000ff;\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_I4:\
        {\
            dest = src & 0x0000000f;\
            cnt = 2;\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_I2:\
        {\
            dest = src & 0x00000003;\
            cnt = 4;\
        }\
        break;\
        default:\
        {\
            printf("Warning, not implemented for color fmt %d", fmt);\
        }\
        break;\
    }\
    } while (0)

/////////////////////////////////////////////////////////////////
// MOcro to convert color to ARGB8888
// @fmt Target color format
// @src Source color value
// @dest Target color value
// @cnt (Output) pixel count per color
/////////////////////////////////////////////////////////////////
#define __CONVERT_COLOR_TO_ARGB8888(fmt, src, dest)\
    do {\
    unsigned char a, r, g, b;\
    r=a=b=g=r==1;\
    switch(fmt)\
    {\
        case E_MI_FB_COLOR_FMT_RGB565:\
        {\
            dest = ((src >> 11) & 0x1f) << 3;\
            dest |= ((src >> 13) & 0x7);\
            dest = dest << 8;\
            dest |= ((src >> 5)&0x3f) << 2;\
            dest |= ((src >> 9) &0x3);\
            dest = dest << 8;\
            dest |= (src&0x1f) << 3;\
            dest |= ((src>>2)&0x7);\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_ARGB4444:\
        {\
            dest = ((src >> 8) & 0xf) << 4;\
            dest |= ((src >> 8) & 0xf);\
            dest = dest << 8;\
            dest |= ((src >> 4) & 0xf) << 4;\
            dest |= ((src >> 4) & 0xf);\
            dest = dest << 8;\
            dest |= (src & 0xf) << 4;\
            dest |= (src & 0xf);\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_ARGB8888:\
        {\
            dest = src;\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_ARGB1555:\
        {\
            dest = ((src>>10)&0x1f)<<3;\
            dest |= ((src>>12)&0x7);\
            dest = dest << 8;\
            dest |= ((src>>5)&0x1f)<<3;\
            dest |= ((src>>7)&0x7);\
            dest = dest << 8;\
            dest |= (src&0x1f)<<3;\
            dest |= ((src>>2)&0x7);\
        }\
        break;\
        case E_MI_FB_COLOR_FMT_I8:\
        {\
            if (src < cmap->start || src >= cmap->len) break;\
            dest = cmap->red[src] << 16 | cmap->green[src] << 8 | cmap->blue[src];\
        }\
        break;\
        default:\
        {\
            printf("Warning, not implemented for color fmt %d", fmt);\
        }\
        break;\
    }\
    } while (0)

/////////////////////////////////////////////////////////////////
// Macro to draw a fill rect.
// @type Datatype of current color used.
// @fmt Color format.
// @buf dest buffer address.
// @pfinfo Pointer to fix info.
// @pvinfo Pointer to var info.
// @rect Rect_t
/////////////////////////////////////////////////////////////////
#define __DRAW_RECT(type, fmt, buf, pfinfo, pvinfo, rect) \
    do{\
        typedef type Color_t;\
        const int bytesPerPixel = sizeof(Color_t);\
        const int stride = pfinfo->line_length / bytesPerPixel;\
        Color_t color = 0;\
        int pixel_per_color = 1;\
        __CONVERT_COLOR_FROM_ARGB8888(fmt, rect.color, color, pixel_per_color);\
        rect.w /= pixel_per_color;\
        rect.x /= pixel_per_color;\
        Color_t *dest = (Color_t *)(buf) + rect.y * stride + rect.x;\
        int x, y;\
        for(y = 0; y < rect.h; ++y)\
        {\
            for(x = 0; x < rect.w; ++x)\
            {\
                dest[x] = color;\
            }\
            dest += stride;\
        }\
    } while (0)

struct fb_cmap *alloc_cmap(void)
{
    int i = 0;
    static struct fb_cmap *cmap = NULL;

    if(cmap != NULL)
        return cmap;

    cmap = malloc(sizeof(struct fb_cmap));
    cmap->start = 0;
    cmap->len = 256;
    cmap->red = calloc(cmap->len, sizeof(short));
    cmap->green = calloc(cmap->len, sizeof(short));
    cmap->blue = calloc(cmap->len, sizeof(short));
    cmap->transp = calloc(cmap->len, sizeof(short));

    for(i = 0; i < cmap->len; i++)
    {
        cmap->red[i] = i;
        cmap->green[i] = i;
        cmap->blue[i] = i;
        cmap->transp[i] = i;
    }

    return cmap;
}
void free_cmap(struct fb_cmap *cmap)
{
    free(cmap->red);
    free(cmap->green);
    free(cmap->blue);
    free(cmap->transp);
    free(cmap);
}

//-----------------------------------------------------------------------------
// get_Color_Format
//-----------------------------------------------------------------------------
static MI_FB_ColorFmt_e get_Color_Format(const struct fb_var_screeninfo *pvinfo)
{
    MI_FB_ColorFmt_e fmt = E_MI_FB_COLOR_FMT_INVALID;

    switch(pvinfo->bits_per_pixel)
    {
        case 2:
            fmt = E_MI_FB_COLOR_FMT_I2;
            break;

        case 4:
            fmt = E_MI_FB_COLOR_FMT_I4;
            break;

        case 8:
            fmt = E_MI_FB_COLOR_FMT_I8;
            break;

        case 16:
        {
            if(pvinfo->transp.length == 0 && pvinfo->red.length == 5)
                fmt = E_MI_FB_COLOR_FMT_RGB565;
            else if(pvinfo->transp.length == 1)
                fmt = E_MI_FB_COLOR_FMT_ARGB1555;
            else if(pvinfo->transp.length == 4)
                fmt = E_MI_FB_COLOR_FMT_ARGB4444;
            else
                fmt = E_MI_FB_COLOR_FMT_YUV422;
        }
        break;

        case 32:
        {
            fmt = E_MI_FB_COLOR_FMT_ARGB8888;
        }
        break;

        default:
            fmt = E_MI_FB_COLOR_FMT_INVALID;
        break;
    }
    return fmt;
}

//-----------------------------------------------------------------------------
// __draw_Rect
//-----------------------------------------------------------------------------
static void __draw_Rect(char *buf, const struct fb_fix_screeninfo *pfinfo,
        const struct fb_var_screeninfo *pvinfo, Rect_t rect)
{
    MI_FB_ColorFmt_e fmt = get_Color_Format(pvinfo);
    switch (fmt)
    {
        case E_MI_FB_COLOR_FMT_RGB565:
        case E_MI_FB_COLOR_FMT_ARGB4444:
        case E_MI_FB_COLOR_FMT_ARGB1555:
        {
            __DRAW_RECT(short, fmt, buf, pfinfo, pvinfo, rect);
        }
        break;

        case E_MI_FB_COLOR_FMT_ARGB8888:
        case E_MI_FB_COLOR_FMT_YUV422:
        {
            __DRAW_RECT(int, fmt, buf, pfinfo, pvinfo, rect);
        }
        break;

        case E_MI_FB_COLOR_FMT_I8:
        case E_MI_FB_COLOR_FMT_I4:
        case E_MI_FB_COLOR_FMT_I2:
        {
            if (NULL == cmap)
                cmap = alloc_cmap();
            if (ioctl(fd, FBIOPUTCMAP, cmap))
                ERR_EXIT("FBIOPUTCMAP");
            __DRAW_RECT(char, fmt, buf, pfinfo, pvinfo, rect);
        }
        break;

        default:
        {
            printf("Warning, draw_Rect not implemented for color fmt %d", fmt);
        }
        break;
    }
}

static void __copy_rect(char *dest, const char *src, Rect_t rect)
{
    const int byte_per_pixel = vinfo.bits_per_pixel / 8;
    dest = dest + rect.y * finfo.line_length + rect.x * byte_per_pixel;
    src = src + rect.y * finfo.line_length + rect.x * byte_per_pixel;
    int width = rect.w * byte_per_pixel;
    int x, y;
    for (y = 0; y <= rect.h; ++y)
    {
        for (x = 0; x <= width; ++x)
        {
            dest[x] = src[x];
        }
        dest += finfo.line_length;
        src += finfo.line_length;
    }
}

//-----------------------------------------------------------------------------
// __add_Dirty_Rect
//-----------------------------------------------------------------------------
static void __add_Dirty_Rect(Rect_t rect)
{
    if (dirty_rect.w == 0 && dirty_rect.h == 0)
    {
        dirty_rect = rect;
        return;
    }
    if (dirty_rect.x > rect.x)
    {
        dirty_rect.w += dirty_rect.x - rect.x;
        dirty_rect.x = rect.x;
    }
    if (dirty_rect.y > rect.y)
    {
        dirty_rect.h += dirty_rect.y - rect.y;
        dirty_rect.y = rect.y;
    }
    if (dirty_rect.x + dirty_rect.w < rect.x + rect.w)
    {
        dirty_rect.w += rect.x + rect.w - dirty_rect.x - dirty_rect.w;
    }
    if (dirty_rect.y + dirty_rect.h < rect.y + rect.h)
    {
        dirty_rect.h += rect.y + rect.h - dirty_rect.y - dirty_rect.h;
    }
}

//-----------------------------------------------------------------------------
// __get_Dirty_Rect
//-----------------------------------------------------------------------------
static int __get_Dirty_Rect(Rect_t *rect)
{
    if (dirty_rect.w == 0 && dirty_rect.h == 0)
        return 0;
    *rect = dirty_rect;
    dirty_rect.h = dirty_rect.w = 0;
    return 1;
}

//-----------------------------------------------------------------------------
// __refresh_Handle thread handle for refresh buffer.
//-----------------------------------------------------------------------------
static void *__refresh_Handle(void *arg)
{
    if (vinfo.yres_virtual / vinfo.yres < 2)
        return NULL;
    int buffersize = vinfo.yres * finfo.line_length;
    Rect_t last_rect;
    while (refreshRunning)
    {
        usleep(20000);
        pthread_mutex_lock(&mutex);
        if (!__get_Dirty_Rect(&last_rect))
        {
            pthread_mutex_unlock(&mutex);
            continue;
        }
        if (curr_buffer == framebuffer)
        {
            vinfo.yoffset = 0;
            if (-1 == ioctl(fd, FBIOPAN_DISPLAY, &vinfo))
                ERR_EXIT("FBIOPAN_DISPLAY.");
            __copy_rect(framebuffer + buffersize, framebuffer, last_rect);
            //memcpy(framebuffer + buffersize, framebuffer, buffersize);
            curr_buffer = framebuffer + buffersize;
        }
        else
        {
            vinfo.yoffset = vinfo.yres;
            if (-1 == ioctl(fd, FBIOPAN_DISPLAY, &vinfo))
                ERR_EXIT("FBIOPAN_DISPLAY.");
            __copy_rect(framebuffer, curr_buffer, last_rect);
            //memcpy(framebuffer, curr_buffer, buffersize);
            curr_buffer = framebuffer;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
static void __print_Fb_Fix_Info(const struct fb_fix_screeninfo *pFInfo)
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
        pFInfo->id,
        pFInfo->smem_start,
        pFInfo->smem_len,
        pFInfo->type,
        pFInfo->type_aux,
        pFInfo->visual,
        pFInfo->xpanstep,
        pFInfo->ypanstep,
        pFInfo->ywrapstep,
        pFInfo->line_length,
        pFInfo->mmio_start,
        pFInfo->mmio_len,
        pFInfo->accel
    );
}

static void __print_Fb_Var_Info(const struct fb_var_screeninfo * pVInfo)
{
    printf ("Variable screen info:\n"
        "\txres: %d\n"
        "\tyres: %d\n"
        "\txres_virtual: %d\n"
        "\tyres_virtual: %d\n"
        "\txoffset: %d\n"
        "\tyoffset: %d\n"
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
        pVInfo->xres, pVInfo->yres, pVInfo->xres_virtual, pVInfo->yres_virtual,
        pVInfo->xoffset, pVInfo->yoffset, pVInfo->bits_per_pixel,
        pVInfo->grayscale, pVInfo->red.offset, pVInfo->red.length,
        pVInfo->red.msb_right, pVInfo->green.offset, pVInfo->green.length,
        pVInfo->green.msb_right, pVInfo->blue.offset, pVInfo->blue.length,
        pVInfo->blue.msb_right, pVInfo->transp.offset, pVInfo->transp.length,
        pVInfo->transp.msb_right, pVInfo->nonstd, pVInfo->activate,
        pVInfo->height, pVInfo->width, pVInfo->accel_flags, pVInfo->pixclock,
        pVInfo->left_margin, pVInfo->right_margin, pVInfo->upper_margin,
        pVInfo->lower_margin, pVInfo->hsync_len, pVInfo->vsync_len,
        pVInfo->sync, pVInfo->vmode
    );
}

//-----------------------------------------------------------------------------
// User interface
//-----------------------------------------------------------------------------
void fb_Tc_Print_Disp_Attr(const MI_FB_DisplayLayerAttr_t *attr)
{
    printf("displayerAttr info:\n"
        "\tu32Xpos: %d\n"
        "\tu32YPos: %d\n"
        "\tu32dstWidth: %d\n"
        "\tu32dstHeight: %d\n"
        "\tu32DisplayWidth :%d\n"
        "\tu32DisplayHeight :%d\n"
        "\tu32ScreenWidth :%d\n"
        "\tu32ScreenHeight :%d\n"
        "\tbPreMul :%d\n"
        "eFbColorFmt :%d\n"
        "eFbOutputColorSpace:%d\n"
        "eFbDestDisplayPlane:%d\n"
        "\n",
        attr->u32Xpos,attr->u32YPos,attr->u32dstWidth,attr->u32dstHeight,
        attr->u32DisplayWidth,attr->u32DisplayHeight,attr->u32ScreenWidth,attr->u32ScreenHeight,
        attr->bPreMul,attr->eFbColorFmt,attr->eFbOutputColorSpace,
        attr->eFbDestDisplayPlane
    );
}
void fb_Tc_Print_Color_Key(const MI_FB_ColorKey_t *key)
{
    printf("ColorKey = [%d, %x, %x, %x]\n", key->bKeyEnable, key->u8Red, key->u8Green, key->u8Blue);
}
void fb_Tc_Print_Rect(const MI_FB_Rectangle_t *rect)
{
    printf("Rect = [%d, %d, %d, %d]\n", rect->u16Xpos, rect->u16Ypos, rect->u16Width, rect->u16Height);
}

void fb_Tc_Print_Fix_Info()
{
    __print_Fb_Fix_Info(&finfo);
}

void fb_Tc_Print_Var_Info()
{
    __print_Fb_Var_Info(&vinfo);
}

void fb_Tc_Get_Fix_Info(struct fb_fix_screeninfo *pfinfo)
{
    if (NULL == pfinfo)
        return ;
    memcpy(pfinfo, &finfo, sizeof(struct fb_fix_screeninfo));
}
void fb_Tc_Get_Var_Info(struct fb_var_screeninfo *pvinfo)
{
    if (NULL == pvinfo)
        return ;
    memcpy(pvinfo, &vinfo, sizeof(struct fb_var_screeninfo));
}
void fb_Tc_Set_Var_Info(const struct fb_var_screeninfo *pvinfo)
{
    if (NULL == pvinfo)
        return ;
    if (-1 == ioctl(fd, FBIOPUT_VSCREENINFO, pvinfo))
        ERR_EXIT("FBIOPUT_VSCREENINFO");
    memcpy(&vinfo, pvinfo, sizeof(struct fb_var_screeninfo));
}

void fb_Tc_Get_Color_Map(struct fb_cmap *pCmap)
{
    if (NULL == pCmap)
        return ;
    if (-1 == ioctl(fd, FBIOGETCMAP, &pCmap))
        ERR_EXIT("FBIOGETCMAP");
}
void fb_Tc_Set_Color_Map(const struct fb_cmap *pCmap)
{
    if (NULL == pCmap)
        return ;
    if (-1 == ioctl(fd, FBIOPUTCMAP, &pCmap))
        ERR_EXIT("FBIOGETCMAP");
}

void fb_Tc_Get_Show(MI_BOOL *bShow)
{
    if (NULL == bShow)
        return ;
    if (-1 == ioctl(fd, FBIOGET_SHOW , bShow))
        ERR_EXIT("FBIOGET_SHOW ");
}
void fb_Tc_Set_Show(const MI_BOOL *bShow)
{
    if (NULL == bShow)
        return ;
    if (-1 == ioctl(fd, FBIOSET_SHOW , bShow))
        ERR_EXIT("FBIOSET_SHOW ");
}

void fb_Tc_Get_ScreenLocation(MI_FB_Rectangle_t *pRect)
{
    if (NULL == pRect)
        return ;
    if (-1 == ioctl(fd, FBIOGET_SCREEN_LOCATION , pRect))
        ERR_EXIT("FBIOGET_SCREEN_LOCATION ");
}
void fb_Tc_Set_ScreenLocation(const MI_FB_Rectangle_t *pRect)
{
    if (NULL == pRect)
        return ;
    if (-1 == ioctl(fd, FBIOSET_SCREEN_LOCATION , pRect))
        ERR_EXIT("FBIOSET_SCREEN_LOCATION ");
}

void fb_Tc_Get_GlobalAlpha(MI_FB_GlobalAlpha_t *pAlpha)
{
    if (NULL == pAlpha)
        return ;
    if (-1 == ioctl(fd, FBIOGET_GLOBAL_ALPHA , pAlpha))
        ERR_EXIT("FBIOGET_GLOBAL_ALPHA ");
}
void fb_Tc_Set_GlobalAlpha(const MI_FB_GlobalAlpha_t *pAlpha)
{
    if (NULL == pAlpha)
        return ;
    if (-1 == ioctl(fd, FBIOSET_GLOBAL_ALPHA , pAlpha))
        ERR_EXIT("FBIOSET_GLOBAL_ALPHA ");
}

void fb_Tc_Get_DispAttr(MI_FB_DisplayLayerAttr_t *pDispAttr)
{
    if (NULL == pDispAttr)
        return ;
    if (-1 == ioctl(fd, FBIOGET_DISPLAYLAYER_ATTRIBUTES , pDispAttr))
        ERR_EXIT("FBIOGET_DISPLAYLAYER_ATTRIBUTES ");
}
void fb_Tc_Set_DispAttr(const MI_FB_DisplayLayerAttr_t *pDispAttr)
{
    if (NULL == pDispAttr)
        return ;
    if (-1 == ioctl(fd, FBIOSET_DISPLAYLAYER_ATTRIBUTES , pDispAttr))
        ERR_EXIT("FBIOSET_DISPLAYLAYER_ATTRIBUTES ");
}

void fb_Tc_Get_CursorAttr(MI_FB_CursorAttr_t *pCursorAttr)
{
    if (NULL == pCursorAttr)
        return ;
    if (-1 == ioctl(fd, FBIOGET_CURSOR_ATTRIBUTE , pCursorAttr))
        ERR_EXIT("FBIOGET_CURSOR_ATTRIBUTE ");
}
void fb_Tc_Set_CursorAttr(const MI_FB_CursorAttr_t *pCursorAttr)
{
    if (NULL == pCursorAttr)
        return ;
    if (-1 == ioctl(fd, FBIOSET_CURSOR_ATTRIBUTE , pCursorAttr))
        ERR_EXIT("FBIOSET_CURSOR_ATTRIBUTE ");
}

void fb_Tc_Get_ColorKey(MI_FB_ColorKey_t *pColorKey)
{
    if (NULL == pColorKey)
        return ;
    if (-1 == ioctl(fd, FBIOGET_COLORKEY , pColorKey))
        ERR_EXIT("FBIOGET_COLORKEY ");
}
void fb_Tc_Set_ColorKey(const MI_FB_ColorKey_t *pColorKey)
{
    if (NULL == pColorKey)
        return ;
    MI_FB_ColorFmt_e fmt = get_Color_Format(&vinfo);
    MI_FB_ColorKey_t key;
    MI_U32 color = 0xff000000 | pColorKey->u8Red << 16 | pColorKey->u8Green << 8 | pColorKey->u8Blue;
    MI_U32 tmp , cnt = 0;
    __CONVERT_COLOR_FROM_ARGB8888(fmt, color, tmp, cnt);
    __CONVERT_COLOR_TO_ARGB8888(fmt, tmp, color);
    key.bKeyEnable = pColorKey->bKeyEnable;
    key.u8Red = (color & 0xff0000) >> 16;
    key.u8Green = (color & 0xff00) >> 8;
    key.u8Blue = color & 0xff;
    if (-1 == ioctl(fd, FBIOSET_COLORKEY, &key))
        ERR_EXIT("FBIOSET_COLORKEY ");
}
void Fb_Tc_Pandisplay()
{
    if (-1 == ioctl(fd, FBIOPAN_DISPLAY, &vinfo))
        ERR_EXIT("FBIOPAN_DISPLAY");
}

void fb_Tc_Fill_Rect(int x0, int y0, int width, int height, int color)
{
    Rect_t rect = {x0, y0, width, height, color};
    if (_bUseDoubleBuffer && !batch_mode)
        pthread_mutex_lock(&mutex);

    __draw_Rect(curr_buffer, &finfo, &vinfo, rect);

    if (_bUseDoubleBuffer)
        __add_Dirty_Rect(rect);
    if (_bUseDoubleBuffer && !batch_mode)
        pthread_mutex_unlock(&mutex);
}

void fb_Tc_Fill_Buffer(char *buffer, unsigned int width, unsigned int height)
{
    if (_bUseDoubleBuffer && !batch_mode)
        pthread_mutex_lock(&mutex);
    int *dest = (int *)curr_buffer;
    int *src = (int *)buffer;
    int i, j;
    for (i = 0; i < vinfo.yres; ++i)
    {
        if (0 == i % height)
            src = (int *)buffer;
        for (j = 0; j < vinfo.xres; j += width)
        {
            memcpy(dest + j, src,
                (vinfo.xres - j < width ? vinfo.xres - j : width) * 4);
        }
        dest += vinfo.xres;
        src += width;
    }
    if (_bUseDoubleBuffer && !batch_mode)
        pthread_mutex_unlock(&mutex);
}

void fb_Tc_Begin_Batch_Draw()
{
    if (!_bUseDoubleBuffer)
        return;
    batch_mode = 1;
    pthread_mutex_lock(&mutex);
}

void fb_Tc_End_Batch_Draw()
{
    if (!_bUseDoubleBuffer)
        return;
    pthread_mutex_unlock(&mutex);
    batch_mode = 0;
}

void fb_Tc_Init(const char *devfile, MI_BOOL bDbuf)
{
    fd = open(devfile, O_RDWR);
    if (-1 == fd)
        ERR_EXIT("Open devfile.");

    if (-1 == ioctl(fd, FBIOGET_FSCREENINFO, &finfo))
        ERR_EXIT("FBIOGET_FSCREENINFO");

    if (-1 == ioctl(fd, FBIOGET_VSCREENINFO, &vinfo))
        ERR_EXIT("FBIOGET_VSCREENINFO");

    framebuffer = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (MAP_FAILED == framebuffer)
        ERR_EXIT("mmap failed.");

    _bUseDoubleBuffer = bDbuf;
    curr_buffer = framebuffer;

    if (_bUseDoubleBuffer)
    {
        refreshRunning = 1;
        if (0 != pthread_create(&tid, NULL, __refresh_Handle, NULL))
            ERR_EXIT("pthread_create.");
    }
}

void fb_Tc_Deinit()
{
    if (cmap)
        free_cmap(cmap);
    if (_bUseDoubleBuffer)
    {
        usleep(100000);
        refreshRunning = 0;
        if (NULL == pthread_join(tid, NULL))
            printf("pthread exit.\n");
    }
    curr_buffer = NULL;
    munmap(framebuffer, finfo.smem_len);
    framebuffer = NULL;
    close(fd);
}

