#include <unistd.h>
#include "sstaradec.h"
#include "common.h"
#include "mi_ao.h"


static MI_U32 u32DmaBufSize;
static MI_S32 g_s32NeedSize;


static int find_wav_data(char *url)
{
    int fd, find_data = 0;
    int head_len, data_len;
    char data[4] = {0};

    fd = open((const char *)url, O_RDONLY, 0666);
    if (fd < 0) {
        printf("open input file failed!\n");
        return -1;
    }

    while (!find_data) {
        read(fd, data, 4);
        if (!strcmp(data, "data")) {
            find_data = 1;
        } else {
            lseek(fd, -3, SEEK_CUR);
        }
    }
    read(fd, data, 4);
    data_len = ((int)data[3] << 24) + ((int)data[2] << 16) + ((int)data[1] << 8) + data[0];
    head_len = lseek(fd, 0, SEEK_CUR);
    printf("wave file size = %d, head info length = %d\n", data_len, head_len);

    close(fd);

    return head_len;
}

int sstar_ao_init(void)
{
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;
    MI_S32 s32GetVolumeDb;

    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;

    if(s32SoundLayout == 2) {
        stSetAttr.u32ChnCnt = 2;
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;  // 立体声
    }
    else if(s32SoundLayout == 1) {
        stSetAttr.u32ChnCnt = 1;
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;    // 单声道
    }

    //stSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_48000;   // 采样率
    stSetAttr.eSamplerate = s32SampleRate;
    printf("set ao sound layout [%d], sample rate [%d]\n", s32SoundLayout, s32SampleRate);

    g_s32NeedSize = MI_AUDIO_SAMPLE_PER_FRAME * 2 * (stSetAttr.u32ChnCnt);
    if (E_MI_AUDIO_SAMPLE_RATE_8000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_8K;;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_16000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_16K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_32000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_32K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_48000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_48K;
    }

    if (stSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_STEREO) {
        if (g_s32NeedSize > (u32DmaBufSize / 4)) {
            g_s32NeedSize = u32DmaBufSize / 4;
        }
    }
    else if (stSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_MONO) {
        if (g_s32NeedSize > (u32DmaBufSize / 8)) {
            g_s32NeedSize = u32DmaBufSize / 8;
        }
    }

    /* set ao public attr*/
    MI_AO_SetPubAttr(AoDevId, &stSetAttr);

    /* get ao device*/
    MI_AO_GetPubAttr(AoDevId, &stGetAttr);

    /* enable ao device */
    MI_AO_Enable(AoDevId);

    /* enable ao channel of device*/
    MI_AO_EnableChn(AoDevId, AoChn);

    /* if test AO Volume */
    MI_AO_SetVolume(AoDevId, g_audio_volume);   // 音量[-60dB ~ 30dB]

    /* get AO volume */
    MI_AO_GetVolume(AoDevId, &s32GetVolumeDb);

    return MI_SUCCESS;
}


int sstar_ao_deinit(void)
{
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

    /* disable ao channel of */
    MI_AO_DisableChn(AoDevId, AoChn);

    /* disable ao device */
    MI_AO_Disable(AoDevId);

    return MI_SUCCESS;
}

void * sstar_audio_thread(void* arg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;
    MI_U8 u8TempBuf[MI_AUDIO_SAMPLE_PER_FRAME * 4];
    WaveFileHeader_t g_stWavHeaderInput;
    int wav_eof = 0, wav_head_len = 0;

    char *wave_file = (char *)arg;
    printf("try to open wave file : %s\n", wave_file);

    wav_head_len = find_wav_data(wave_file);
    if (wav_head_len < 0) {
        printf("find_wav_data error\n");
        return NULL;
    }

    int g_AoReadFd = open((const char *)wave_file, O_RDONLY, 0666);
    if(g_AoReadFd <= 0) {
        printf("open input file failed!!!\n");
        return NULL;
    }

    s32Ret = read(g_AoReadFd, &g_stWavHeaderInput, sizeof(WaveFileHeader_t));
    if (s32Ret < 0) {
        printf("read wav header failed!!!\n");
        return NULL;
    }
    printf("wave file's audio channel layout = %d\n", g_stWavHeaderInput.wave.wChannels);
    printf("wave file's sample rate = %d\n", g_stWavHeaderInput.wave.dwSamplesPerSec);

    lseek(g_AoReadFd, wav_head_len, SEEK_SET);

    while(!bExit)
    {
        s32Ret = read(g_AoReadFd, &u8TempBuf, g_s32NeedSize);
        if(s32Ret != g_s32NeedSize) {
            if (s32Ret >= 0) {
                //printf("Read Num = %d, g_s32NeedSize : %d\n", s32Ret, g_s32NeedSize);
                wav_eof = 1;
            } else {
                lseek(g_AoReadFd, wav_head_len, SEEK_SET);
                s32Ret = read(g_AoReadFd, &u8TempBuf, g_s32NeedSize);
                if (s32Ret < 0) {
                    printf("input file does not has enough data!!!\n");
                    break;
                }
            }
        }
        //printf("Read Num = %d, g_s32NeedSize : %d\n", s32Ret, g_s32NeedSize);
        // 送数据到AO
        memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
        stAoSendFrame.u32Len = s32Ret;          // pcm 数据长度
        stAoSendFrame.apVirAddr[0] = u8TempBuf; // pcm 数据地址
        stAoSendFrame.apVirAddr[1] = NULL;

        do{
            s32Ret = MI_AO_SendFrame(AUDIO_DEV, AUDIO_CHN, &stAoSendFrame, -1);
        }while(s32Ret == MI_AO_ERR_NOBUF);

        if(s32Ret != MI_SUCCESS)
        {
            printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
        }
        if (wav_eof) {
            lseek(g_AoReadFd, wav_head_len, SEEK_SET);
            wav_eof = 0;
        }
    }

    if (g_AoReadFd > 0) {
        close(g_AoReadFd);
    }

    return NULL;
}



