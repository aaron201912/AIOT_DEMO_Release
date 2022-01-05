#ifndef __SSTARADEC__H__
#define __SSTARADEC__H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus


#define AUDIO_CHN       0
#define MI_AUDIO_SAMPLE_PER_FRAME 1024
#define DMA_BUF_SIZE_8K     (8000)
#define DMA_BUF_SIZE_16K    (16000)
#define DMA_BUF_SIZE_32K    (32000)
#define DMA_BUF_SIZE_48K    (48000)

#if ENABLE_HDMI
#define AUDIO_DEV       3
#else
#define AUDIO_DEV       0
#endif



int sstar_ao_deinit(void);
int sstar_ao_init(void);
void * sstar_audio_thread(void* arg);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__SSTARADEC__H__

