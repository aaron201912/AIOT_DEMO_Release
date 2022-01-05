#define _GUN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>

#define SUPPORT_JPEG 1
#define SUPPORT_PNG 0
#include <mi_common.h>
#include <mi_sys_datatype.h>
#include <mi_sys.h>
#include "mstarFb.h"
#include "sstardisp.h"
#include "bmp.h"
#include "jpeg.h"
//#include "mypng.h"
#include <mi_gfx.h>
#include <mi_gfx_datatype.h>
#include "verify_gfx_type.h"
#include "verify_gfx.h"
#include "blitutil.h"


#define TEST_GFX    1


struct fb_var_screeninfo vinfo = {0};
struct fb_fix_screeninfo finfo = {0};
MI_FB_DisplayLayerAttr_t g_stLayerInfo = {0};

//Start of frame buffer mem
static char *frameBuffer = NULL;
int _fbFd = 0;
MI_BOOL _bEnable_Rotate;
char _file_path[256];


#define LOGO_FILE_RAW "/sstar_configs/logo.raw"
#define LOGO_FILE_JPG "./logo.jpg"
#define LOGO_FILE_PNG "/sstar_configs/logo.png"

#define LOGO_FILE LOGO_FILE_JPG

#define LOGO_SUFFIX_RAW ".raw"
#define LOGO_SUFFIX_JPEG ".jpg"
#define LOGO_SUFFIX_PNG ".png"

#define LOG() printf("%s %d \n",__FUNCTION__,__LINE__);
#if (SUPPORT_JPEG||SUPPORT_PNG)
static void syncFormat(BITMAP *bmp, struct fb_var_screeninfo *vinfo)
{
    MI_U32 Rmask;
    MI_U32 Gmask;
    MI_U32 Bmask;
    MI_U32 Amask;
    int i;

    Rmask = 0;

    for(i = 0; i < vinfo->red.length; ++i)
    {
        Rmask <<= 1;
        Rmask |= (0x00000001 << vinfo->red.offset);
    }

    Gmask = 0;

    for(i = 0; i < vinfo->green.length; ++i)
    {
        Gmask <<= 1;
        Gmask |= (0x00000001 << vinfo->green.offset);
    }

    Bmask = 0;

    for(i = 0; i < vinfo->blue.length; ++i)
    {
        Bmask <<= 1;
        Bmask |= (0x00000001 << vinfo->blue.offset);
    }

    Amask = 0;

    for(i = 0; i < vinfo->transp.length; ++i)
    {
        Amask <<= 1;
        Amask |= (0x00000001 << vinfo->transp.offset);
    }

    CompileFormat(&bmp->pxFmt, bmp->bmBitsPerPixel, Rmask, Gmask, Bmask, Amask);
}
#endif

#define __USE_GNU

#include <sched.h>
#include <pthread.h>
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
FILE *fp;

void *readTOPlogo(void *pos)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    LOG();

    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
        errExit("sched_setaffinity");

    LOG();

    fseek(fp, 0, SEEK_SET);
    size_t n = fread(frameBuffer, (MI_U32)pos, 1, fp);
    printf("%s %d %d %x \n", __FUNCTION__, __LINE__, n,(MI_U32)pos);
    return NULL;

}

void *readBTMlogo(void *pos)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(1, &set);
    LOG();

    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
        errExit("sched_setaffinity");

    LOG();

    fseek(fp, (MI_U32)pos, SEEK_SET);
    size_t n = fread(frameBuffer + (MI_U32)pos, (MI_U32)pos, 1, fp);
    printf("%s %d %d %x \n", __FUNCTION__, __LINE__, n,(MI_U32)pos);
    return NULL;
}

MI_GFX_Surface_t _srcSurf;
MI_GFX_Rect_t _src_Rect;

MI_GFX_Surface_t _dstSurf;
MI_GFX_Rect_t _dst_Rect;

int __create_dst_surface_ARGB(char *data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;

    _dstSurf.eColorFmt = _srcSurf.eColorFmt;
    _dstSurf.phyAddr = finfo.smem_start;
    _dstSurf.u32Width = vinfo.xres;
    _dstSurf.u32Height = vinfo.yres;
    _dstSurf.u32Stride = _dstSurf.u32Width * getBpp(_dstSurf.eColorFmt);
    _dst_Rect.s32Xpos = 0;
    _dst_Rect.s32Ypos =  0;
    _dst_Rect.u32Width = _dstSurf.u32Width;
    _dst_Rect.u32Height = _dstSurf.u32Height;
    data = frameBuffer;

    //printf("dstSurf.u32Width:%d ,dstSurf.u32Height:%d ,dstSurf.eColorFmt:%d ,dstSurf.u32Stride:%d,dstRect.s32Xpos:%d,dstRect.s32Ypos:%d,dstRect.u32Width:%d,dstRect.u32Height:%d\n",
    //    _dstSurf.u32Width,_dstSurf.u32Height,_dstSurf.eColorFmt,_dstSurf.u32Stride,_dstSurf.s32Xpos,dstRect.s32Ypos,dstRect.u32Width,dstRect.u32Height);

    color = 0xFF000000;
    MI_GFX_QuickFill(&_dstSurf, &_dst_Rect, color, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;
}
int __create_src_surface_ARGB(char **data)
{
    MI_U32 color = 0;
    MI_U16 fence = 0;
    MI_S32 ret = 0;

    switch(g_stLayerInfo.eFbColorFmt)
    {
        case E_MI_FB_COLOR_FMT_RGB565:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_BGR565;
            break;
        case E_MI_FB_COLOR_FMT_ARGB4444:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB4444;
            break;
        case E_MI_FB_COLOR_FMT_ARGB8888:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB8888;
            break;
        case E_MI_FB_COLOR_FMT_ARGB1555:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB1555;
            break;
        case E_MI_FB_COLOR_FMT_YUV422:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_YUV422;
            break;
        case E_MI_FB_COLOR_FMT_I8:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_I8;
            break;
        case E_MI_FB_COLOR_FMT_I4:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_I4;
            break;
        case E_MI_FB_COLOR_FMT_I2:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_I2;
            break;
        default:
            _srcSurf.eColorFmt =E_MI_GFX_FMT_ARGB8888;
            break;
    }
    _srcSurf.phyAddr = 0;
    _srcSurf.u32Width = vinfo.yres;//vinfo.xres;//假设图片是600X1024,所以这里是反的，只是用来测试。
    _srcSurf.u32Height = vinfo.xres;//vinfo.yres;
    _srcSurf.u32Stride = _srcSurf.u32Width * getBpp(_srcSurf.eColorFmt);

    if((ret = _gfx_alloc_surface(&_srcSurf, data, "blendSrc")) < 0) {
        return ret;
    }

    _src_Rect.s32Xpos = 0;
    _src_Rect.s32Ypos = 0;
    _src_Rect.u32Width = _srcSurf.u32Width;
    _src_Rect.u32Height = _srcSurf.u32Height;
    color = 0xFF000000;
    MI_GFX_QuickFill(&_srcSurf, &_src_Rect, color, &fence);
    MI_GFX_WaitAllDone(FALSE, fence);
    return 0;
}

int open_fb()
{
    const char *devfile = "/dev/fb0";

    memset(&finfo, 0, sizeof(struct fb_fix_screeninfo));
    memset(&vinfo, 0, sizeof(struct fb_var_screeninfo));
    memset(&g_stLayerInfo, 0, sizeof(MI_FB_DisplayLayerAttr_t));

    /* Open the file for reading and writing */
    _fbFd = open(devfile, O_RDWR);

    if(_fbFd == -1)
    {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    int show = 0;

    if(ioctl(_fbFd, FBIOSET_SHOW, &show) < 0)
    {
        return -1;
    }

    //get fb_fix_screeninfo
    if(ioctl(_fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        return -1;
    }

    //get fb_var_screeninfo
    if(ioctl(_fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        return -1;
    }
    //get FBIOGET_DISPLAYLAYER_ATTRIBUTES
    if(ioctl(_fbFd, FBIOGET_DISPLAYLAYER_ATTRIBUTES, &g_stLayerInfo) == -1)
    {
        perror("3Error reading variable information");
        return -1;
    }

    frameBuffer = (char *) mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, _fbFd, 0);
    if(frameBuffer == MAP_FAILED)
    {
        perror("Error: Failed to map framebuffer device to memory");
        return -1;
    }
    return 0;
}
void close_fb()
{
    munmap(frameBuffer, finfo.smem_len);
    close(_fbFd);
}
#if 0
void load_png(BITMAP *fb)
{
    FILE *fp;
    if((fp = fopen(_file_path, "r")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", _file_path);
        return 0;
    }
    load_PNG_file(fp, fb);

    fclose(fp);
}
#endif

int load_jpeg(BITMAP *fb)
{
    BITMAP logo;
    FILE *fp;

    if((fp = fopen(_file_path, "r")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", _file_path);
        return -1;
    }
    struct jpeg_decompress_struct  cinfo;
    struct my_error_mgr jerr;

    init_JPEG_file(fp, &logo, &cinfo, &jerr);

    load_JPEG_file(fb, &logo, &cinfo);

    fclose(fp);
    return 0;
}

void display_help(void)
{
    printf("************************* Video usage *************************\n");
    printf("--pic_path : pic file path\n");
    printf("-R         : choice:[0-1]0=NONE,1=rotate_90\n");

    printf("eg:./JpegPlayer --pic_path ./logo.jpg -R 1\n");

    return;
}

int parse_args(int argc, char **argv)
{
    int option_index=0;
    MI_S32 s32Opt = 0;

    struct option long_options[] = {
            {"pic_path", required_argument, NULL, 'P'},
            {"help", no_argument, NULL, 'h'},
            {0, 0, 0, 0}
    };

    while ((s32Opt = getopt_long(argc, argv, "R:",long_options, &option_index))!= -1 )
    {
        switch(s32Opt)
        {
            case 'P':
            {
                if(strlen(optarg) != 0)
                {
                   strcpy(_file_path,optarg);
                }
                break;
            }
            case 'R':
            {
                _bEnable_Rotate = atoi(optarg);
                //printf("-P  _bEnable_Panel=%d \n",_bEnable_Panel);
                break;
            }
            case 'h':
            default:
            {
                display_help();
                return -1;
                break;
            }

        }
    }
    return 0;
}

#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)

int main(int argc, char **argv)
{
    BITMAP fb;

    MI_DISP_PubAttr_t stDispPubAttr = {0};
    struct timeval tv1 = {0, 0};
    struct timeval tv2 = {0, 0};

    if(argc < 2)
    {
        display_help();
        return 0;
    }

    if(parse_args(argc, argv) != 0)
    {
        return 0;
    }


    char *srcData = NULL;
    char *dstData = NULL;

    gettimeofday(&tv1, NULL);

    //gettimeofday(&tv2, NULL);
    //printf("%s %d %ld \n \n \n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);

    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;
    sstar_disp_init(&stDispPubAttr);
    if (open_fb() != 0)
    {
        goto EXIT;
    }
    gettimeofday(&tv2, NULL);
    printf("%s %d %ld\n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);

    if (_bEnable_Rotate)
    {
        __create_src_surface_ARGB(&srcData);
        __create_dst_surface_ARGB(dstData);
        fb.bmBits = srcData;
        fb.bmPhyAddr = _srcSurf.phyAddr;
        fb.bmWidth = _srcSurf.u32Width;
        fb.bmHeight = _srcSurf.u32Height;
        fb.bmPitch = _srcSurf.u32Stride;
    }
    else
    {
        fb.bmBits = frameBuffer;
        fb.bmPhyAddr = finfo.smem_start;
        fb.bmHeight = vinfo.yres;
        fb.bmWidth = vinfo.xres;
        fb.bmPitch = finfo.line_length;
    }
    fb.bmBitsPerPixel =  vinfo.bits_per_pixel;
    fb.bmBytesPerPixel = vinfo.bits_per_pixel / 8;

    printf("xres: %d,yres: %d\n",fb.bmWidth,fb.bmHeight);
    printf("xres: %d,yres: %d,,finfo.line_length: %d,g_stLayerInfo.eFbColorFmt: %d \n",vinfo.xres,vinfo.yres,finfo.line_length,g_stLayerInfo.eFbColorFmt);
    printf("fb.bmWidth: %d,fb.bmHeight: %d,fb.bmPitch: %d,fb.bmBitsPerPixel: %d,fb.bmBytesPerPixel: %d\n",fb.bmWidth,fb.bmHeight,fb.bmPitch,fb.bmBitsPerPixel,fb.bmBytesPerPixel);

    syncFormat(&fb, &vinfo);

    if(strstr(_file_path, LOGO_SUFFIX_JPEG) != NULL)
    {
        printf("Jpeg \n");
        load_jpeg(&fb);
    }
    else if(strstr(LOGO_FILE, LOGO_SUFFIX_PNG) != NULL)
    {
        printf("Png \n");
        //load_png(&fb);
    }

    if (_bEnable_Rotate)
    {
        __test_rotate_ARGB_90(_srcSurf, _src_Rect, _dstSurf, _dst_Rect, dstData, FALSE);
    }


    //Pandisplay
    if(ioctl(_fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(5);
    }
    printf("%s %d ,Any key_down will lead to exit \n", __FUNCTION__, __LINE__);
    getchar();
    gettimeofday(&tv2, NULL);
    printf("%s %d %ld\n", __FUNCTION__, __LINE__, (tv2.tv_usec - tv1.tv_usec) / 1000);
    if (_bEnable_Rotate)
    {
        _gfx_free_surface(&_srcSurf, srcData);
    }
EXIT:

    sstar_disp_Deinit(&stDispPubAttr);
    //unmap buffer
    close_fb();
    return 0;
}
