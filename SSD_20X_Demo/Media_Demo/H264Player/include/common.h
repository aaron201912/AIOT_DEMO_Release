#ifndef __COMMON__H__
#define __COMMON__H__


#ifdef __cplusplus
    extern "C"{
#endif // __cplusplus

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "mi_common_datatype.h"
#include <unistd.h>



#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
    }

#define ALIGN_N(x, align)           (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_BACK(x, a)            (((x) / (a)) * (a))
#define ALIGN_FRONT(x, a)           ((((x)+(a)/2) / (a)) * (a))
#define SSMAX(a,b) ((a) > (b) ? (a) : (b))
#define SSMIN(a,b) ((a) > (b) ? (b) : (a))

MI_BOOL bExit;
MI_BOOL _bChkStreamEnd;
MI_S32  s32VideoWidth;
MI_S32  s32VideoHeight;

/*********************************************video param*****************************************************/

MI_S32 g_enable_pip;
MI_S32 g_vdec_num;
MI_BOOL g_bframe;

#define  DISP_CHN_EXTRA     16 //disp的channel16对应的layer0
#define  DISP_LAYER0        0
#define  DISP_LAYER1        1
#define  DISP_DEV           0
#define  DISP_PORT_EXTRA    0 //layer1只有port0

typedef struct ReOrderSlice_s {
    MI_U8 *pos;
    MI_U32 len;
} ReOrderSlice_t;

typedef struct
{
    int startcodeprefix_len;
    unsigned int len;
    unsigned int max_size;
    char *buf;
    unsigned short lost_packets;
} NALU_t;

MI_U64 getOsTime(void);
NALU_t *AllocNALU(int buffersize);
void FreeNALU(NALU_t *n);
int GetAnnexbNALU (NALU_t *nalu, MI_S32 chn, FILE *fp);
int FindStartCode2 (unsigned char *Buf);
int FindStartCode3 (unsigned char *Buf);

/*********************************************audio param*****************************************************/
typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    char chRIFF[4];
    unsigned int  dwRIFFLen;
    char chWAVE[4];
    char chFMT[4];
    unsigned int  dwFMTLen;
    WaveFormat_t wave;
    char chDATA[4];
    unsigned int  dwDATALen;
} WaveFileHeader_t;

MI_S32 s32SoundLayout;
MI_S32 s32SampleRate;

MI_S32 g_audio_volume;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__COMMON__H__

