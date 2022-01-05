#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <termios.h>
#include <string.h>
#include "stdio.h"
#include <stdint.h>
#include <getopt.h>

#include "jdec.h"

#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_divp.h"
#include "mi_disp.h"
#include "mi_disp_datatype.h"
#include "mi_gfx_datatype.h"
#include "mi_venc.h"
#include "mi_venc_datatype.h"
#include "sstardisp.h"
#include "common.h"




static int _img_width = 0;
static int _img_height = 0;
static MI_U16 panel_width;
static MI_U16 panel_height;
static int file_yuv_size = 0;

static MI_S32 bRota;


extern void ST_I420ToNV12(const uint8_t* pu8src_y,
                       int src_stride_y,
                       const uint8_t* pu8src_u,
                       int src_stride_u,
                       const uint8_t* pu8src_v,
                       int src_stride_v,
                       uint8_t* pu8dst_y,
                       int dst_stride_y,
                       uint8_t* pu8dst_uv,
                       int dst_stride_uv,
                       int width,
                       int height);

extern void ST_I422ToYUY2(const uint8_t* pu8src_y,
                   int src_stride_y,
                   const uint8_t* pu8src_u,
                   int src_stride_u,
                   const uint8_t* pu8src_v,
                   int src_stride_v,
                   uint8_t* pu8dst_yuy2,
                   int dst_stride_yuy2,
                   int width,
                   int height);

extern int parse_marker(char* buf,int buf_size,int* image_width,int* image_height);


static int _OpenFile(const char *pFilePath)
{
    //printf("FilePath:%s \n",pFilePath);
    int s32Fd = open(pFilePath, O_RDWR);
    if (s32Fd < 0)
    {
        perror("open");

        return -1;
    }

    return s32Fd;
}
static int _CloseFile(int s32Fd)
{
    //fsync(s32Fd);
    close(s32Fd);
    return 0;
}

static int _FileWrite(int fd, char *pBuf, int size)
{
    int s32WriteCnt = 0;
    int s32Ret = 0;
    do
    {
        s32Ret = write(fd, pBuf, size - s32WriteCnt);
        if (s32Ret < 0)
        {
            perror("write");
            return -1;
        }
        if (s32Ret == 0)
        {
            break;
        }
        s32WriteCnt += s32Ret;
        pBuf += s32Ret;
    }while(s32WriteCnt < size);

    return s32WriteCnt;
}

static int _FileRead(int fd, char *pBuf, int size)
{
    int s32ReadCnt = 0;
    int s32Ret = 0;
    do
    {
        s32Ret = read(fd, pBuf, size - s32ReadCnt);
        if (s32Ret < 0)
        {
            return -1;
        }
        if (s32Ret == 0)
        {
            break;
        }
        s32ReadCnt += s32Ret;
        pBuf += s32Ret;
    }while(s32ReadCnt < size);

    return s32ReadCnt;
}

int sdk_Init(void)
{
    MI_DISP_PubAttr_t stDispPubAttr = {0};

    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;
    sstar_disp_init(&stDispPubAttr);

    return 0;
}

int sdk_DeInit(void)
{
    //unbind modules
    MI_DISP_PubAttr_t stDispPubAttr = {0};

    if(g_divp_enable)
    {
        MI_SYS_ChnPort_t stSrcChnPort;
        MI_SYS_ChnPort_t stDstChnPort;
        //unbind disp0 2 divp0
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
        STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));

        //deinit divp0
        STCHECKRESULT(MI_DIVP_StopChn(DIVP_CHN0_ID));
        STCHECKRESULT(MI_DIVP_DestroyChn(DIVP_CHN0_ID));
        _divp_is_enable = 0;
    }

    //deinit sys
    //STCHECKRESULT(MI_SYS_Exit());
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;
    sstar_disp_Deinit(&stDispPubAttr);

    return 0;
}


int _sys_mma_alloc(jdecIMAGE *pImage, int size)
{
    if (0 != MI_SYS_MMA_Alloc((unsigned char*)"#jdecI0", size, &(pImage->phyAddr)))
    {
        printf("MI_SYS_MMA_Alloc fail \n");
        return -1;
    }
    if (0 != MI_SYS_Mmap(pImage->phyAddr,size, (void *)&(pImage->virtAddr), 1))
    {
        MI_SYS_MMA_Free(pImage->phyAddr);
        printf("MI_SYS_Mmap fail \n");
        return -1;
    }

    return 0;
}

int _sys_mma_free(jdecIMAGE *pImage, int size)
{
    if (0 != MI_SYS_Munmap(pImage->virtAddr, size))
    {
        printf("MI_SYS_Munmap fail \n");
        return -1;
    }
    if (0 != MI_SYS_MMA_Free(pImage->phyAddr))
    {
        printf("MI_SYS_MMA_Free fail \n");
        return -1;
    }
    pImage->virtAddr = NULL;
    pImage->phyAddr = NULL;
    return 0;
}

char* get_buf_from_file(char* fpath,long* length)
{
    int s32fd;
    long file_len;
    long actual_read_len;
    char *pbuf = NULL;

    s32fd = _OpenFile(fpath);
    if(s32fd == -1)
    {
        printf("Open file %s fail,please check \n",fpath);
        return NULL;
    }

    file_len = lseek(s32fd, 0, SEEK_END);
    lseek(s32fd, 0, SEEK_SET);
    if(pbuf == NULL)
    {
        pbuf =( char*) malloc(file_len);
        if(pbuf == NULL)
        {
            printf("malloc mjpeg_buf fail \n");
            return NULL;
        }
    }

    actual_read_len = _FileRead(s32fd,pbuf,file_len);
    *length = actual_read_len;

    printf("the actually read length is %ld file_len=%ld \n",*length,file_len);
    _CloseFile(s32fd);

    return pbuf;
}

void dump_file(char* path,void *pbuf,int size)
{
    int tmp_s32fd;
    tmp_s32fd = open(path, O_RDWR|O_APPEND|O_CREAT);
    if(tmp_s32fd == -1)
    {
        printf("Open file path=%s fail\n",path);
        return ;
    }
    _FileWrite(tmp_s32fd,pbuf,size);

    _CloseFile(tmp_s32fd);

}

static void NV12Rotate90(char* src_y,char* src_uv,char* dst_y,char* dst_uv,int width,int height)
{
    // Rotate the Y luma
    int i = 0;
    int x = 0;
    int y = 0;
    for(x = 0;x < width;x++)
    {
        for(y = height;y > 0;y--)
        {
            dst_y[i] = src_y[y*width-width+x]; //遍历每一列，从第一列开始，列变为行
            i++;
        }
    }

    i = 0;
    for(x = 0;x < width/2;x++)//UV数据作为一组，不能分开
    {
        for(y = height/2;y > 0;y--)
        {
            dst_uv[i] = src_uv[y*width-width+2*x]; //UV数据一次就旋转所以是2*x
            dst_uv[i+1] = src_uv[y*width-width+2*x+1];
            i+=2;
        }
    }

}

#if 1
static void YUYV422Rotate90(char* src,char* dst,int width,int height)
{
    // Rotate the Y luma
    int i = 0;
    int x = 0;
    int y = 0;
    for(x = 0;x < width;x++)
    {
        for(y = height;y > 0;y--)
        {
            dst[i] = src[y*width*2-width*2+2*x];
            dst[i+1] = src[y*width*2-width*2+2*x+1];
            i+=2;
        }
    }
}

#else
void YUYV422Rotate90(char *src, char* dst , int width, int height)
{
    const int copyBytes    = 4;
    const int bytesPerLine = width << 1;
    const int step         = height  << 2;
    const int offset       = (height - 1) * bytesPerLine;
    if(NULL == dst)
    {
        return false;
    }
    unsigned char * dest       = dst;
    unsigned char * source  = src;
    unsigned char * psrc       = NULL;
    unsigned char * pdst[2]   = {NULL, NULL};

    for (int i = 0; i < bytesPerLine; i += copyBytes)
    {
        pdst[0] = dest;
        pdst[1] = dest   + (height << 1);
        psrc    = source + offset;

        for (int j = 0; j < height; ++j)
        {
            int k = j % 2;

            // 拷贝4个字节
            *((unsigned int *)pdst[k]) = *((unsigned int *)psrc);

            // Y分量交换，保证每个像素点的亮度不改变否则产生锯齿
            if(1 == k)
            {
                unsigned char temp = *(pdst[0] - 1);
                *(pdst[0] - 1) = *(pdst[1] + 1);
                *(pdst[1] + 1) = temp;
            }

            pdst[k] += copyBytes;
            psrc    -= bytesPerLine;
        }

        dest   += step;
        source += copyBytes;
    }

    return true;
}
#endif


int _mi_ChnInputPortPutBuf(MI_SYS_BUF_HANDLE bufHandle, jdecIMAGE* pImage, MI_SYS_BufInfo_t* stBufInfo)
{
    unsigned char* pu8STSrcU = NULL;
    unsigned char* pu8STSrcV = NULL;
    int u32Index = 0;
    char *tmp_yuv_buf = NULL;
    char *tmp_frame_buf = NULL;
    stBufInfo->stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stBufInfo->stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
    stBufInfo->stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
    stBufInfo->bEndOfStream = 0;
    if(pImage->esubsamp == SAMP_422)//divp 只支持YUV422_YUYV      YV16-->420
    {
        printf("YUV 422\n");
        tmp_frame_buf = (char *)malloc(pImage->width*pImage->height*2);
        if(tmp_frame_buf == NULL)
        {
            printf("malloc tmp_frame_buf fail \n");
            return -1;
        }

        pu8STSrcU = (unsigned char*)pImage->virtAddr+pImage->width*pImage->height;
        pu8STSrcV = (unsigned char*)pImage->virtAddr+pImage->width*pImage->height+pImage->width*pImage->height/2;

        ST_I422ToYUY2((unsigned char*)pImage->virtAddr,pImage->width,
                        pu8STSrcU,pImage->width/2,
                        pu8STSrcV,pImage->width/2,
                        (unsigned char*)tmp_frame_buf,pImage->width*2,
                        pImage->width,pImage->height);
        //printf("pImage->width=%d, pImage->height=%d u32Stride=%d xinhua\n",pImage->width,pImage->height,stBufInfo->stFrameData.u32Stride[0]);
        //dump_file("/customer/422_img",tmp_frame_buf,pImage->height*pImage->width*2);
        if(bRota)
        {
            tmp_yuv_buf = (char *)malloc(pImage->width*pImage->height*2);
            if(tmp_yuv_buf == NULL)
            {
                printf("malloc tmp_yuv_buf fail \n");
                return -1;
            }
            //dump_file("src_img",stBufInfo->stFrameData.pVirAddr[0],pImage->width*pImage->height*2);
            YUYV422Rotate90(tmp_frame_buf,tmp_yuv_buf,pImage->width,pImage->height);
            dump_file("/customer/422_rotate_img",tmp_yuv_buf,pImage->height*pImage->width*2);
        }
        else
        {
            tmp_yuv_buf = tmp_frame_buf;
        }

        memset(stBufInfo->stFrameData.pVirAddr[0],0xff,pImage->width*pImage->height*2);

        for (u32Index = 0; u32Index < stBufInfo->stFrameData.u16Height; u32Index ++)
        {
            memcpy(stBufInfo->stFrameData.pVirAddr[0]+(u32Index*stBufInfo->stFrameData.u32Stride[0]),
                       tmp_yuv_buf+(u32Index*stBufInfo->stFrameData.u16Width*2), stBufInfo->stFrameData.u16Width*2);
        }

        stBufInfo->stFrameData.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YVYU;
    }
    else if(pImage->esubsamp == SAMP_420)//divp 只支持NV12（YUV420sp）
    {
        printf("YUV 420\n");
        tmp_frame_buf = (char *)malloc(pImage->width*pImage->height*3/2);
        if(tmp_frame_buf == NULL)
        {
            printf("malloc tmp_frame_buf fail \n");
            return -1;
        }
        char *tmp_uv_buf = tmp_frame_buf + (pImage->width*pImage->height);

        pu8STSrcU = (unsigned char*)pImage->virtAddr+pImage->width*pImage->height;
        pu8STSrcV = (unsigned char*)pImage->virtAddr+pImage->width*pImage->height+pImage->width*pImage->height/2/2;

        ST_I420ToNV12((unsigned char*)pImage->virtAddr,pImage->width,
                        pu8STSrcU,pImage->width/2,
                        pu8STSrcV,pImage->width/2,
                        (unsigned char*)tmp_frame_buf,pImage->width,
                        (unsigned char*)tmp_uv_buf,pImage->width,
                        pImage->width,pImage->height);

        //dump_file("/customer/origin_img",tmp_frame_buf,pImage->width*pImage->height*3/2);

        if(bRota)
        {
            tmp_yuv_buf = (char *)malloc(pImage->width*pImage->height*3/2);
            if(tmp_yuv_buf == NULL)
            {
                printf("malloc tmp_yuv_buf fail \n");
                return -1;
            }
            NV12Rotate90(tmp_frame_buf,tmp_uv_buf,
                            tmp_yuv_buf,tmp_yuv_buf+(pImage->width*pImage->height),pImage->width,pImage->height);

        }
        else
        {
            //printf("u32Stride0=%d u32Stride1=%d width=%d\n",stBufInfo->stFrameData.u32Stride[0],stBufInfo->stFrameData.u32Stride[1],pImage->width);
            tmp_yuv_buf = tmp_frame_buf;
        }

        for (u32Index = 0; u32Index < stBufInfo->stFrameData.u16Height; u32Index ++)
        {
            memcpy(stBufInfo->stFrameData.pVirAddr[0]+(u32Index*stBufInfo->stFrameData.u32Stride[0]),
                       tmp_yuv_buf+(u32Index*stBufInfo->stFrameData.u16Width), stBufInfo->stFrameData.u16Width);
        }
        for (u32Index = 0; u32Index < stBufInfo->stFrameData.u16Height / 2; u32Index ++)
        {
            memcpy(stBufInfo->stFrameData.pVirAddr[1]+(u32Index*stBufInfo->stFrameData.u32Stride[1]),
                        tmp_yuv_buf+(u32Index*stBufInfo->stFrameData.u16Width)+(pImage->width*pImage->height),stBufInfo->stFrameData.u16Width);
        }

        stBufInfo->stFrameData.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    }

    if(bRota)
    {
        if(tmp_frame_buf)
        {
            free(tmp_frame_buf);
            tmp_frame_buf = NULL;
        }
        if(tmp_yuv_buf)
        {
            free(tmp_yuv_buf);
            tmp_yuv_buf = NULL;
        }
    }
    else
    {
        if(tmp_frame_buf)
        {
            free(tmp_frame_buf);
            tmp_frame_buf = NULL;
            tmp_yuv_buf = NULL;
        }
    }

    if(MI_SUCCESS != MI_SYS_ChnInputPortPutBuf(bufHandle, stBufInfo, 0))
    {
        printf("MI_SYS_ChnInputPortPutBuf fail \n");
        return -1;
    }
    return 0;
}

int _mi_ChnInputPortGetBuf(MI_SYS_BUF_HANDLE* bufHandle, jdecIMAGE* pImage, MI_SYS_BufInfo_t* stBufInfo)
{
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_ChnPort_t stSrcChnPort;

    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
    memset(&stSrcChnPort, 0, sizeof(stSrcChnPort));

    if(bufHandle == NULL || stBufInfo == NULL)
    {
        printf("Err,Null point \n");
        return -1;
    }

    stBufConf.u64TargetPts = 0;//stTv.tv_sec*1000000 + stTv.tv_usec;
    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.stFrameCfg.u16Width = ALIGN_UP(_img_width, 2);
    stBufConf.stFrameCfg.u16Height = ALIGN_UP(_img_height, 2);
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stBufConf.u32Flags = MI_SYS_MAP_VA;

    if(g_divp_enable)
    {
        stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    }
    else
    {
        stSrcChnPort.eModId = E_MI_MODULE_ID_DISP;
    }
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32PortId = 0;


    switch(pImage->esubsamp)//divp only support YUYV422&&YUV420SP
    {
        case SAMP_422://divp 只支持YUV422_YUYV      YV16-->420
        {
            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        }
        break;
        case SAMP_420://divp/DISP 只支持NV12（YUV420sp）
        {
            stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        }
        break;
        case SAMP_444:
        default:
        {
            printf("not support this YUV type,esubsamp=%d \n",pImage->esubsamp);
            return -1;
        }
    }
    if(MI_SUCCESS != MI_SYS_ChnInputPortGetBuf(&stSrcChnPort, &stBufConf, stBufInfo, bufHandle, 0))
    {
        printf("MI_SYS_ChnInputPortGetBuf fail \n");
        return -1;
    }
    return 0;
}


void display_help(void)
{
    printf("--mpath : mjpeg file path \n");
    printf("-E : Enable Divp \n");
    printf("-R : Video rotate choice:[0-1]0=NONE,1=rotate_90 \n");
    return;
}

int parse_args(int argc, char **argv)
{
    int option_index=0;
    MI_S32 s32Opt = 0;

    struct option long_options[] = {
            {"mpath", required_argument, NULL, 'M'},
            {"help", no_argument, NULL, 'h'},
            {0, 0, 0, 0}
    };

    while ((s32Opt = getopt_long(argc, argv, "M:R:E:",long_options, &option_index))!= -1 )
    {
        switch(s32Opt)
        {

            //mjpeg file
            case 'M':
            {
                if(strlen(optarg) != 0)
                {
                   strcpy(g_mjpeg_file,optarg);
                }
                break;
            }
            //enable divp
            case 'E':
            {
                if(strlen(optarg) != 0)
                {
                    g_divp_enable = atoi(optarg);
                }
                break;
            }
            case 'R':
            {
                bRota = atoi(optarg);
                break;
            }
            case '?':
            {
                if(optopt == 'M')
                {
                    printf("Missing Video file path, please --vpath 'video_path' \n");
                }

                return -1;
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

int check_resoulution(int image_width, int image_height)
{
    if(bRota)
    {
        _img_width = image_height;
        _img_height = image_width;
    }
    else
    {
        _img_width = image_width;
        _img_height = image_height;
    }

    sstar_get_panel_size(&panel_width, &panel_height);
    if(!g_divp_enable)
    {
        if((_img_width > panel_width) || (_img_height > panel_height))
        {
            printf("file_size is large than disp size,panel_width=%d panel_height=%d _img_width=%d _img_height=%d\n",panel_width,panel_height,_img_width,_img_height);
            return -1;
        }
    }
    else
    {
        if((_img_width > MAX_RESOLURION_W) || (_img_height > MAX_RESOLURION_H))
        {
            printf("file_size is large than divp max channel size \n");
            return -1;
        }

    }
    return 0;
}

int main(int argc, char* argv[])
{

    jdecIMAGE image0 = {0};
    long file_len = 0;
    int image_width = 0;
    int image_height = 0;
    long one_jepg_size = 0;
    int decode_cnt = 0;
    char *mjpeg_buf = NULL;
    char* tmp_jpeg_buf = NULL;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE bufHandle;

    if(argc < 2)
    {
        display_help();
        return 0;
    }
    if(parse_args(argc, argv) != 0)
    {
        return 0;
    }

    sdk_Init();

    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&bufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    mjpeg_buf = get_buf_from_file(g_mjpeg_file,&file_len);
    if(file_len < 0 || mjpeg_buf == NULL)
    {
        sdk_DeInit();
        return 0;
    }
    tmp_jpeg_buf = mjpeg_buf;

    for(;;)
    {
        if(file_len - one_jepg_size <= 0)
        {
            printf("File end,finish \n");
            goto EXIT;
        }

        tmp_jpeg_buf += one_jepg_size;
        file_len -= one_jepg_size;
        one_jepg_size = parse_marker(tmp_jpeg_buf,file_len - one_jepg_size,&image_width,&image_height);
        if(one_jepg_size < 0)
        {
            printf("File end,finish \n");
            goto EXIT;
        }

        if(check_resoulution(image_width,image_height) != 0)
        {
            goto EXIT;
        }

        if(file_yuv_size != image_width*image_height*YUV_TYPE)
        {
            if(image0.phyAddr != NULL)
            {
                _sys_mma_free(&image0, file_yuv_size);
                printf("alloc  again \n");
            }

            file_yuv_size = image_width*image_height*YUV_TYPE;

            if (0 != _sys_mma_alloc(&image0,file_yuv_size))
            {
                printf("_sys_mma_alloc fail \n");
                return 0;
            }
        }

        if(0 != jdec_decodeYUVFromBuf(tmp_jpeg_buf, one_jepg_size, &image0, TANSFORM_NONE))
        {
            printf("Decode done, width=%d height=%d decode_cnt=%d \n", image0.width, image0.height,decode_cnt);
            goto EXIT;
        }
        decode_cnt ++;
        usleep(TIME_DIFF_PRE_FRAME);//帧率控制,33ms播放一帧，也就是30帧/s
        //printf("bRota=%d yuv_size=%d image_size=%d width=%d height=%d decode_cnt=%d one_jepg_size=%ld esubsamp=%d\n",bRota,yuv_size, image0.width*image0.height*3/2,image0.width ,image0.height,decode_cnt,one_jepg_size,image0.esubsamp);

        if(g_divp_enable)
        {
            sstar_set_disp_InputPort(0, 0,  panel_width, panel_height);
            sstar_enable_divp(panel_width, panel_height);
        }
        else
        {
            if(image0.esubsamp != SAMP_420)
            {
                printf("Disp Not support this YUV Type,esubsamp=%d \n",image0.esubsamp);
                goto EXIT;
            }
            sstar_set_disp_InputPort(0, 0, _img_width, _img_height);
        }
        if(0 == _mi_ChnInputPortGetBuf(&bufHandle, &image0, &stBufInfo))
        {
            if(0 != _mi_ChnInputPortPutBuf(bufHandle, &image0, &stBufInfo))
            {
                printf("_mi_ChnInputPortPutBuf fail ,exit\n");
                goto EXIT;
            }
        }
        else
        {
            printf("_mi_ChnInputPortGetBuf fail \n");
            goto EXIT;
        }

    }
EXIT:
    getchar();
    sdk_DeInit();
    if(image0.phyAddr != NULL)
    {
        _sys_mma_free(&image0, file_yuv_size);
    }
    if(mjpeg_buf != NULL)
    {
        free(mjpeg_buf);
        mjpeg_buf = NULL;
    }
    return 0;
}



