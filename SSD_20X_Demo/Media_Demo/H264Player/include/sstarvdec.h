#ifndef __SSTARVDEC__H__
#define __SSTARVDEC__H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

//VDEC输入宽高无须用户根据视频宽高设置,最大1080P即可
#define VDEC_INPUT_WIDTH     1920
#define VDEC_INPUT_HEIGHT    1080


int sstar_vdec_init(int VChan_num);
int sstar_vdec_deInit(int VChan_num);
void * sstar_video_thread(void* arg);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__SSTARVDEC__H__

