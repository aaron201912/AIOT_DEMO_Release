#include <signal.h>
#include <pthread.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <stdbool.h>

#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_panel_datatype.h"

#include "common.h"
#include "sstardisp.h"
#include "sstarvdec.h"
#include "sstaradec.h"

#include "UtilS_SPS_PPS.h"

static char g_video_file[256];
static char g_audio_file[256];


static MI_S32 g_audio_enable=false;
static MI_S32 g_video_enable=false;


void display_help(void)
{
    printf("************************* Video usage *************************\n");
    printf("--vpath : Video file path\n");
    printf("-X : Video x coordinate,default value is 0\n");
    printf("-Y : Video y coordinate,default value is 0\n");
    printf("-W : Video with\n");
    printf("-H : Video height\n");
    printf("-R : Video rotate choice:[0-4]0=NONE,1=rotate_90,1=rotate_180,1=rotate_270\n");
    printf("-N : Video Port Num:[0-4]\n");
    printf("-P : PIP ,Enable pip:[0/1]\n");
    printf("eg:./SsPlayer  --vpath 720P25.h264 -X 0 -Y 0 -W 1024 -H 600 -R 0 -N 3 -P 1\n");

    printf("************************* Audio usage *************************\n");
    printf("--apath : Audio file path\n");
    printf("-s : Audio sample rate:\n");
    printf("-c : Audio channel mode:0=MONO,1=STEREO\n");
    printf("-v : Audio volume:[-60db-30db]\n");

    printf("eg:./SsPlayer --apath res/48k_stereo.wav -c 2 -s 48000 -v -10\n");

    return;
}


int parse_args(int argc, char **argv)
{
    int option_index=0;
    MI_S32 s32Opt = 0;

    struct option long_options[] = {
            {"vpath", required_argument, NULL, 'V'},
            {"apath", required_argument, NULL, 'a'},
            {"hdmi", no_argument, NULL, 'M'},
            {"help", no_argument, NULL, 'h'},
            {0, 0, 0, 0}
    };

    while ((s32Opt = getopt_long(argc, argv, "X:Y:W:H:Y:R:N:P:s:c:v:h",long_options, &option_index))!= -1 )
    {
        switch(s32Opt)
        {
            //video
            case 'V':
            {
                if(strlen(optarg) != 0)
                {
                   strcpy(g_video_file,optarg);
                   g_video_enable = true;
                }
                break;
            }
            case 'X':
            {
                OutX= atoi(optarg);
                break;
            }
            case 'Y':
            {
                OutY= atoi(optarg);
                break;
            }
            case 'W':
            {
                OutDispWidth = atoi(optarg);
                inDispWidth  = OutDispWidth;
                break;
            }
            case 'H':
            {
                OutDispHeight= atoi(optarg);
                inDispHeight = OutDispHeight;
                break;
            }
            case 'R':
            {
                bRota = (MI_DISP_RotateMode_e)atoi(optarg);
                break;
            }
            case 'N':
            {
                g_vdec_num = atoi(optarg);
                if(g_vdec_num > 4)
                {
                    printf("param error g_vdec_num=%d,-N param should be less than 4\n",g_vdec_num);
                    return -1;
                }
                break;
            }
            case 'P':
            {
                g_enable_pip = atoi(optarg);
                printf("g_enable_pip=%d \n",g_enable_pip);
                break;
            }

            //audio
            case 'a':
            {
                if(strlen(optarg) != 0)
                {
                   strcpy(g_audio_file,optarg);
                   g_audio_enable = true;
                }
                break;
            }
            case 's':
            {
                s32SampleRate = atoi(optarg);
                printf("s32SampleRate = %d\n",s32SampleRate);
                break;
            }
            case 'c':
            {
                s32SoundLayout = atoi(optarg);
                if(s32SoundLayout > 2)
                {
                    printf("param error s32SoundLayout=%d,-c param should be 1/2\n",s32SoundLayout);
                    return -1;
                }
                break;
            }
            case 'v':
            {
                g_audio_volume = atoi(optarg);
                printf("g_audio_volume = %d\n",g_audio_volume);
                break;
            }
            case '?':
            {
                if(optopt == 'V')
                {
                    printf("Missing Video file path, please --vpath 'video_path' \n");
                }
                if(optopt == 'a')
                {
                    printf("Missing Audio file path, please --apath 'audio_path' \n");
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

int parse_video_info(char *filename, MI_S32 *width, MI_S32 *height, MI_BOOL *type)
{
    int seek_length;
    int find_sps_code = 0, find_pps_code = 0;
    int sps_pos = 0, pps_pos = 0;
    FILE *fp = NULL;
    unsigned char *temp_buf = NULL;

    fp = fopen(filename, "rb");
    if (!fp) {
        printf("open %s error!\n", filename);
        return -1;
    }

    temp_buf = (unsigned char *)malloc(1024 * sizeof(unsigned char));
    if (!temp_buf) {
        printf("parse_video_info: malloc buf error\n");
        goto fail;
    }

    if (3 != fread (temp_buf, 1, 3, fp)) {
        goto fail;
    }
    seek_length = 3;
    while (!find_sps_code || !find_pps_code) {
        temp_buf[seek_length ++] = fgetc(fp);

        if (FindStartCode3(&temp_buf[seek_length - 4]) != 1) {
            if (FindStartCode2(&temp_buf[seek_length - 3]) != 1) {

            } else {
                //start_code_len = 3;
                temp_buf[seek_length] = fgetc(fp);
                if ((temp_buf[seek_length] & 0x1F) == 7) {
                    find_sps_code = 1;
                    sps_pos = seek_length;
                } else if ((temp_buf[seek_length] & 0x1F) == 8) {
                    find_pps_code = 1;
                    pps_pos = seek_length;
                }
                seek_length ++;
            }
        } else {
            //start_code_len = 4;
            temp_buf[seek_length] = fgetc(fp);
            if ((temp_buf[seek_length] & 0x1F) == 7) {
                find_sps_code = 1;
                sps_pos = seek_length;
            } else if ((temp_buf[seek_length] & 0x1F) == 8) {
                find_pps_code = 1;
                pps_pos = seek_length;
            }
            seek_length ++;
        }
        if (seek_length >= 1024) {
            printf("parse_video_info: cant't find start code\n");
            goto fail;
        }
    }
    fclose(fp);
    printf("sps position = [%d], pps length = [%d]\n", sps_pos, (pps_pos - sps_pos - 4));

    SPS sps_buf;
    get_bit_context bitcontext;
    memset(&bitcontext,0x00,sizeof(get_bit_context));
    bitcontext.buf = temp_buf + sps_pos + 1;
    bitcontext.buf_size = (pps_pos - sps_pos) - 4;
    h264dec_seq_parameter_set(&bitcontext, &sps_buf);
    *width  = h264_get_width(&sps_buf);
    *height = h264_get_height(&sps_buf);
    //根据reoder的数量判断视频是否有B帧
    if (sps_buf.vui_parameters.num_reorder_frames > 0) {
        *type = true;
    } else {
        *type = false;
    }
    printf("h264 of sps w/h = [%d %d], has bframe: %d\n", *width, *height, *type);
    free(temp_buf);
    return 0;

fail:
    fclose(fp);
    free(temp_buf);
    return -1;
}


int main (int argc, char **argv)
{
    pthread_t tid_audio;
    pthread_t tid_video;

    if(argc <= 1 )
    {
        display_help();
        return 0;
    }
    if(parse_args(argc, argv) != 0)
    {
        return 0;
    }
    if((bRota == E_MI_DISP_ROTATE_90 || bRota == E_MI_DISP_ROTATE_270) && g_vdec_num > 0)
    {//HW Rotate模式下,每个layer只能inputport0旋转,hw limit(180是gfx做的不受限制)
        if(g_vdec_num != 1 || g_enable_pip != 1)
        {
            printf("Hw limit,each layer only support one port(port0) with hw rotate(E_MI_DISP_ROTATE_90/270)\n");
            return 0;
        }
    }

    if(g_vdec_num > 4)
    {
        printf("Invail Param with -N=%d ，should be less than 4  \n",g_vdec_num);
        return 0;
    }
    else if((g_vdec_num > 3) && (g_enable_pip == 1))//limit
    {
        printf("Disable Pip,VChan_num is:%d,should be less than 4 total\n",g_vdec_num);
        g_enable_pip = 0;
    }
    else if((g_vdec_num == 0) && (g_enable_pip == 0))
    {
        g_vdec_num = 1;
    }

    parse_video_info(g_video_file, &s32VideoWidth, &s32VideoHeight, &g_bframe);
    if (s32VideoWidth <= 0 || s32VideoHeight <= 0)
    {
         printf("parse video info faile!\n");
         return 0;
    }

    MI_DISP_PubAttr_t stDispPubAttr;
#if ENABLE_HDMI
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
#else
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
#endif
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    sstar_disp_init(&stDispPubAttr, g_vdec_num);

    sstar_vdec_init(g_vdec_num);

    sstar_disp_set_rotatemode(bRota);

    if (g_audio_enable) {
        sstar_ao_init();
        pthread_create(&tid_audio, NULL, sstar_audio_thread, (void *)g_audio_file);
    }

    if (g_video_enable) {
        pthread_create(&tid_video, NULL, sstar_video_thread, (void *)g_video_file);
    }

    while (!bExit)
    {
        printf("please input 'q' to exit\n");
        if(getchar() == 'q')
        {
            bExit = TRUE;
            printf("### H264Player Exit ###\n");
        }
    }

    if (g_video_enable) {
        pthread_join(tid_video, NULL);
    }

    if (g_audio_enable) {
        pthread_join(tid_audio, NULL);
        sstar_ao_deinit();
    }

    sstar_disp_Deinit(&stDispPubAttr, g_vdec_num);
    sstar_vdec_deInit(g_vdec_num);

    return 0;
}
