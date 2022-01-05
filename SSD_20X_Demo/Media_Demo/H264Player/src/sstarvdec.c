#include <sys/prctl.h>

#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"

#include "sstardisp.h"
#include "sstarvdec.h"
#include "common.h"

#define  VDEC_CHN_NUM       4

MI_VDEC_ChnAttr_t stVdecChnConf[VDEC_CHN_NUM];
MI_VDEC_CodecType_e _eCodecType = E_MI_VDEC_CODEC_TYPE_H264;

void init_vdec_channel(int VChan_num)
{
    MI_U32 i;
    MI_VDEC_CHN stVdecChn;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_InitParam_t stInitParm;

    memset(&stInitParm, 0x0, sizeof(stInitParm));
    stInitParm.bDisableLowLatency = g_bframe;
    MI_VDEC_InitDev(&stInitParm);

    // 设置视频的解码选项
    for (i = 0; i < VChan_num; i ++)
    {
        stVdecChnConf[i].eCodecType     = E_MI_VDEC_CODEC_TYPE_H264;
        stVdecChnConf[i].eVideoMode     = E_MI_VDEC_VIDEO_MODE_FRAME;
        stVdecChnConf[i].eDpbBufMode    = E_MI_VDEC_DPB_MODE_NORMAL;
        stVdecChnConf[i].u32PicWidth    = VDEC_INPUT_WIDTH;//1280;    // 视频源的实际宽高
        stVdecChnConf[i].u32PicHeight   = VDEC_INPUT_HEIGHT;//720;
    }

    // vdec chn 0,1,2,3 -> disp_layer0   chn 4 -> disp_layer1
    for (i = 0; i < VChan_num; i ++)
    {
        stVdecChn = i;
        memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
        stVdecChnAttr.eCodecType     = stVdecChnConf[i].eCodecType;
        stVdecChnAttr.u32PicWidth    = stVdecChnConf[i].u32PicWidth;
        stVdecChnAttr.u32PicHeight   = stVdecChnConf[i].u32PicHeight;
        stVdecChnAttr.eDpbBufMode    = stVdecChnConf[i].eDpbBufMode;
        stVdecChnAttr.eVideoMode     = stVdecChnConf[i].eVideoMode;

        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 5;
        stVdecChnAttr.u32BufSize     = 1 * 1920 * 1080;
        stVdecChnAttr.u32Priority    = 0;

        STCHECKRESULT(MI_VDEC_CreateChn(stVdecChn, &stVdecChnAttr));
        STCHECKRESULT(MI_VDEC_StartChn(stVdecChn));

        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        stOutputPortAttr.u16Width    = ALIGN_BACK(SSMIN(s32VideoWidth , stDispWnd[i].u16PicW), 32);  // 解码器缩放后的宽高
        stOutputPortAttr.u16Height   = ALIGN_BACK(SSMIN(s32VideoHeight, stDispWnd[i].u16PicH), 32);  // 与disp输入保持一致
        STCHECKRESULT(MI_VDEC_SetOutputPortAttr(stVdecChn, &stOutputPortAttr));
    }


    // 前四幅图像采用vdec 0~3通道解码，第五幅图像采用vdec 4通道解码
    if(g_enable_pip)
    {
        memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
        stVdecChnAttr.eCodecType     = E_MI_VDEC_CODEC_TYPE_H264;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 5;
        stVdecChnAttr.eVideoMode     = E_MI_VDEC_VIDEO_MODE_FRAME;
        stVdecChnAttr.u32BufSize     = 1 * 1920 * 1080;
        stVdecChnAttr.u32PicWidth    = VDEC_INPUT_WIDTH;//1280;
        stVdecChnAttr.u32PicHeight   = VDEC_INPUT_HEIGHT;//720;
        stVdecChnAttr.eDpbBufMode    = E_MI_VDEC_DPB_MODE_NORMAL;
        stVdecChnAttr.u32Priority    = 0;

        STCHECKRESULT(MI_VDEC_CreateChn(VChan_num, &stVdecChnAttr));
        STCHECKRESULT(MI_VDEC_StartChn(VChan_num));

        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        stOutputPortAttr.u16Width       = ALIGN_BACK(stExtraDispWnd.u16PicW, 32);  // 解码器缩放后的宽高
        stOutputPortAttr.u16Height      = ALIGN_BACK(stExtraDispWnd.u16PicH, 32);  // 与disp输入保持一致
        STCHECKRESULT(MI_VDEC_SetOutputPortAttr(VChan_num, &stOutputPortAttr));
    }


}
int vdec_bind_to_disp(int VChan_num)
{
    MI_U32 i = 0;
    MI_SYS_ChnPort_t stDispChnPort;
    MI_SYS_ChnPort_t stVdecChnPort;
    MI_VDEC_CHN stVdecChn;
    MI_DISP_CHN stDispChn;

    // bind vdec chn 0~3 -> disp chn 0~3
    for (i = 0; i < VChan_num; i ++)
    {
        stVdecChn = i;
        stDispChn = i;
        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = 0;
        stDispChnPort.u32ChnId              = stDispChn;
        stDispChnPort.u32PortId             = 0;

        memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stVdecChnPort.eModId                = E_MI_MODULE_ID_VDEC;
        stVdecChnPort.u32DevId              = 0;
        stVdecChnPort.u32ChnId              = stVdecChn;
        stVdecChnPort.u32PortId             = 0;

        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stVdecChnPort, 0, 3));
        STCHECKRESULT(MI_SYS_BindChnPort(&stVdecChnPort, &stDispChnPort, 30 ,30));
    }
    if(g_enable_pip)
    {
        //说明: 画中画使用了两个IDSP_LAYER, LAYER0对应的DISP的0~15通道, LAYER1对应的是DISP的16通道
        //所以这里需要将VDEC的通道绑定到DISP的16通道上, 保证VDEC的图像输出到LAYER1上
        // bind vdec chn 4 -> disp chn 16
        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = 0;
        stDispChnPort.u32ChnId              = DISP_CHN_EXTRA;
        stDispChnPort.u32PortId             = 0;

        memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stVdecChnPort.eModId                = E_MI_MODULE_ID_VDEC;
        stVdecChnPort.u32DevId              = 0;
        stVdecChnPort.u32ChnId              = VChan_num;
        stVdecChnPort.u32PortId             = 0;

        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stVdecChnPort, 0, 3));
        STCHECKRESULT(MI_SYS_BindChnPort(&stVdecChnPort, &stDispChnPort, 30 ,30));
    }

    return 0;
}


int sstar_vdec_init(int VChan_num)
{
    //init vdec
    init_vdec_channel(VChan_num);
    vdec_bind_to_disp(VChan_num);
    return 0;
}

int sstar_vdec_deInit(int VChan_num)
{
    //Unbind vdec 2 disp
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_VDEC_CHN stVdecChn;
    MI_DISP_CHN stDispChn;
    int i = 0;

    for (i = 0; i < VChan_num; i ++)
    {
        stVdecChn = i;
        stDispChn = i;
        memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
        stSrcChnPort.u32DevId = 0;
        stSrcChnPort.u32ChnId = stVdecChn;
        stSrcChnPort.u32PortId = 0;

        stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stDstChnPort.u32DevId = 0;
        stDstChnPort.u32ChnId = stDispChn;
        stDstChnPort.u32PortId = 0;

        STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));
        STCHECKRESULT(MI_VDEC_StopChn(stVdecChn));
        STCHECKRESULT(MI_VDEC_DestroyChn(stVdecChn));

    }
    if(g_enable_pip)
    {
        //说明: 画中画使用了两个IDSP_LAYER, LAYER0对应的DISP的0~15通道, LAYER1对应的是DISP的16通道
        //所以这里需要将VDEC的通道绑定到DISP的16通道上, 保证VDEC的图像输出到LAYER1上
        // bind vdec chn 4 -> disp chn 16
        memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
        stSrcChnPort.u32DevId = 0;
        stSrcChnPort.u32ChnId = VChan_num;
        stSrcChnPort.u32PortId = 0;

        stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stDstChnPort.u32DevId = 0;
        stDstChnPort.u32ChnId = DISP_CHN_EXTRA;
        stDstChnPort.u32PortId = 0;


        STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));
        STCHECKRESULT(MI_VDEC_StopChn(VChan_num));
        STCHECKRESULT(MI_VDEC_DestroyChn(VChan_num));
    }

    STCHECKRESULT(MI_VDEC_DeInitDev());

    return 0;
}


/********************************************************************************/

#define MAX_ONE_FRM_SIZE (2 * 1024 * 1024)
#define DROP_SLICE_ID (3)



ReOrderSlice_t _stVdecSlice[100];
MI_U32 _u32SliceIdx = 0;

MI_BOOL _bFrmMode = FALSE;      //根据输入视频是否带B帧设置
MI_BOOL _bReOrderSlice = FALSE;
MI_BOOL _bSleep = FALSE;
MI_BOOL _bDropFrm = FALSE;


void * sstar_video_thread(void* arg)
{
    MI_VDEC_CHN vdecChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32TimeOutMs = 20;
    MI_U32 u32WrPtr = 0;
    MI_S32 s32NaluType = 0;
    MI_U8 *pu8Buf = NULL;
    MI_U8 *pSlice2FrmBuf = NULL;
    MI_U8 *pSliceReOrder = NULL;
    MI_U64 u64Pts = 0;
    MI_U64 u64FisrtSlicePushTime = 0;
    MI_BOOL bFirstFrm = FALSE;
    MI_BOOL frameType = 0;
    MI_U32 u32SliceCount = 0;
    MI_U32 u32FpBackLen = 0; // if send stream failed, file pointer back length
    FILE *fReadFile = NULL;
    int i = 0;
    NALU_t *pstNalu;
    MI_VDEC_VideoStream_t stVdecStream;

    _bFrmMode = g_bframe;

    pSlice2FrmBuf = (unsigned char *)malloc(MAX_ONE_FRM_SIZE);
    if (!pSlice2FrmBuf) {
        printf("ST_VdecSendStream: malloc frame buf error\n");
        return NULL;
    }

    if (_bReOrderSlice) {
        pSliceReOrder = (unsigned char *)malloc(MAX_ONE_FRM_SIZE);
        if (!pSliceReOrder) {
            printf("ST_VdecSendStream: malloc reorder buf error\n");
            return NULL;
        }
    }

    char *video_file = (char *)arg;

    pstNalu = AllocNALU(MAX_ONE_FRM_SIZE);
    if (!pstNalu) {
        return NULL;
    }

    prctl(PR_SET_NAME, "send_thrd_0");

    fReadFile = fopen(video_file, "rb"); //ES
    if (!fReadFile)
    {
        printf("Open %s failed!\n", video_file);
        return NULL;
    }

    printf("Open %s success, vdec chn:%d\n", video_file, vdecChn);
    pu8Buf = (MI_U8 *)malloc(MAX_ONE_FRM_SIZE);
    if (!pu8Buf) {
        printf("ST_VdecSendStream: malloc pbuf error\n");
        return NULL;
    }

    while (!bExit)
    {
        s32Ret = GetAnnexbNALU(pstNalu, vdecChn, fReadFile);
        if (s32Ret <= 0) {
            printf("GetAnnexbNALU: read nal data error\n");
            continue;
        }

        stVdecStream.pu8Addr = (MI_U8 *)pstNalu->buf;
        if(9 == pstNalu->len
            && 0 == *(pstNalu->buf)
            && 0 == *(pstNalu->buf+1)
            && 0 == *(pstNalu->buf+2)
            && 1 == *(pstNalu->buf+3)
            && 0x68 == *(pstNalu->buf+4)
            && 0 == *(pstNalu->buf+pstNalu->len-1))
        {
            stVdecStream.u32Len = 8;
        }
        else {
            stVdecStream.u32Len = pstNalu->len;
        }
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        if (_bChkStreamEnd && (feof (fReadFile)))
        {
            stVdecStream.bEndOfStream = 1;
            printf("set end of file flag done\n");
        }

        u32FpBackLen = stVdecStream.u32Len; //back length
        if(0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1]
            && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]
            && (0x65 == stVdecStream.pu8Addr[4] || 0x61 == stVdecStream.pu8Addr[4]
            || 0x26 == stVdecStream.pu8Addr[4] || 0x02 == stVdecStream.pu8Addr[4]
            || 0x41 == stVdecStream.pu8Addr[4]))
        {
            usleep(30 * 1000);    //帧率控制不超过30
        }

        if (_eCodecType == E_MI_VDEC_CODEC_TYPE_H265) {
            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]) {
                bFirstFrm = (stVdecStream.pu8Addr[6] & 0x80);
                s32NaluType = (stVdecStream.pu8Addr[4] & 0x7E) >> 1;
            }

            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x01 == stVdecStream.pu8Addr[2]) {
                bFirstFrm = (stVdecStream.pu8Addr[5] & 0x80);
                s32NaluType = (stVdecStream.pu8Addr[3] & 0x7E) >> 1;
            }

            if (s32NaluType <= 31) {
                ///frame type
                frameType = 1;
            } else {
                frameType = 0;
            }
        } else {
            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]) {
                bFirstFrm = (stVdecStream.pu8Addr[5] & 0x80);
                s32NaluType = stVdecStream.pu8Addr[4] & 0xF;
            }

            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x01 == stVdecStream.pu8Addr[2]) {
                bFirstFrm = (stVdecStream.pu8Addr[4] & 0x80);
                s32NaluType = stVdecStream.pu8Addr[3] & 0xF;
            }
            if (1 <= s32NaluType && s32NaluType <= 5) {
                ///frame type
                frameType = 1;
            } else {
                frameType = 0;
            }
            //printf("nal data[4]: %x, data[5]: %x, bFirstFrm: %d\n", stVdecStream.pu8Addr[4], stVdecStream.pu8Addr[5], bFirstFrm);
        }

        if (bFirstFrm) {
            u32SliceCount = 0;
        }

        if (u64FisrtSlicePushTime == 0) {
            u64FisrtSlicePushTime = getOsTime();
        }

        if (_bDropFrm && frameType && (getOsTime() - u64FisrtSlicePushTime > 3000) && (u32SliceCount == DROP_SLICE_ID)) {
            printf("drop slice, id=%d, 0x%02x, type:%d\n", u32SliceCount, stVdecStream.pu8Addr[4], s32NaluType);
            u32SliceCount++;
            continue;
        }

        if (_bFrmMode) {
            if (u32WrPtr && bFirstFrm) {
                MI_U8 *pTmp = stVdecStream.pu8Addr;
                MI_U32 u32TmpLen = stVdecStream.u32Len;

                stVdecStream.pu8Addr = pSlice2FrmBuf;
                stVdecStream.u32Len = u32WrPtr;

                if (_bReOrderSlice && frameType && (getOsTime() - u64FisrtSlicePushTime > 3000)) {
                    int WLen = 0;
                    for (int sliceID = 0; sliceID < _u32SliceIdx; ++sliceID) {
                        if (sliceID == 0) {
                            memcpy(pSliceReOrder, _stVdecSlice[0].pos, _stVdecSlice[0].len);
                            WLen += _stVdecSlice[0].len;
                        } else {
                            memcpy(pSliceReOrder + WLen, _stVdecSlice[_u32SliceIdx - sliceID].pos, _stVdecSlice[_u32SliceIdx - sliceID].len);
                            WLen += _stVdecSlice[_u32SliceIdx - sliceID].len;
                        }
                    }

                    stVdecStream.pu8Addr = pSliceReOrder;
                    //printf("reorder done...\n");
                }
                //printf("send data to vdec addr: %p, length: %d\n", stVdecStream.pu8Addr, stVdecStream.u32Len);
                if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
                {
                    printf("chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", vdecChn, stVdecStream.u32Len, s32Ret);
                    fseek(fReadFile, - u32FpBackLen, SEEK_CUR);
                    continue;
                } else {
                    stVdecStream.pu8Addr = pTmp;
                    stVdecStream.u32Len = u32TmpLen;
                    u32WrPtr = 0;
                    if (_bReOrderSlice) {
                        _u32SliceIdx = 0;
                    }
                }

                //usleep(30 * 1000);
            }

            memcpy(pSlice2FrmBuf + u32WrPtr, stVdecStream.pu8Addr, stVdecStream.u32Len);
            if (_bReOrderSlice) {
                _stVdecSlice[_u32SliceIdx].pos = pSlice2FrmBuf + u32WrPtr;
                _stVdecSlice[_u32SliceIdx].len = stVdecStream.u32Len;
                _u32SliceIdx++;
            }
            u32WrPtr += stVdecStream.u32Len;
            //printf("pSlice2FrmBuf addr %p, length %d\n", pSlice2FrmBuf, u32WrPtr);
        } else {
            if (_bSleep) {
            }
            for(i=0; i < g_vdec_num; i++)
            {
                vdecChn = i;
                if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
                {
                    //printf("chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", vdecChn, stVdecStream.u32Len, s32Ret);
                    fseek(fReadFile, - u32FpBackLen, SEEK_CUR);
                    usleep(30 * 1000);
                }
            }
            if(g_enable_pip)
            {
                if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(g_vdec_num, &stVdecStream, s32TimeOutMs)))
                {
                    //printf("chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", vdecChn, stVdecStream.u32Len, s32Ret);
                    fseek(fReadFile, - u32FpBackLen, SEEK_CUR);
                    usleep(30 * 1000);
                }
            }
        }

        if (_bChkStreamEnd && (feof (fReadFile)))
        {
            printf("end of stream, wait dec done...\n");
            usleep(5 * 1000 * 1000);
            bExit = TRUE;
        }

        u32SliceCount++;
    }
    free(pu8Buf);
    FreeNALU(pstNalu);
    fclose(fReadFile);
    free(pSlice2FrmBuf);
    printf("end of sstar_video_thread\n");

    return NULL;
}



