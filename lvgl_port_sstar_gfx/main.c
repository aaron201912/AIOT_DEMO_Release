#include "lvgl/lvgl.h"
#include "lv_drivers/indev/evdev.h"
#include "lv_drivers/display/ss_fbdev.h"
#include "lv_demos/lv_demo.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define LV_HOR_RES_MAX 800
#define LV_VER_RES_MAX 480
#if LV_COLOR_DEPTH == 1
#define LV_DISP_BPP 1
#else
#define LV_DISP_BPP (LV_COLOR_DEPTH / 8)
#endif

// for test
#define REFRESH_ONE_BUF 1
#define FULL_ONE_BUF    1

#if (REFRESH_ONE_BUF && !FULL_ONE_BUF)
#define DISP_BUF_SIZE (LV_HOR_RES_MAX * LV_VER_RES_MAX / 10)
#else
#define DISP_BUF_SIZE (LV_HOR_RES_MAX * LV_VER_RES_MAX)
#endif

int main(void)
{
    lv_indev_drv_t input_drv;

    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    ss_fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    //static lv_color_t buf[DISP_BUF_SIZE];
#if REFRESH_ONE_BUF
#if FULL_ONE_BUF
    lv_color_t *buf = NULL;
#else
    static lv_color_t buf[DISP_BUF_SIZE];
#endif
#else
    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;
#endif

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;

#if REFRESH_ONE_BUF
#if FULL_ONE_BUF
    ss_fbdev_get_draw_buf((void **)&buf, NULL, LV_HOR_RES_MAX, LV_VER_RES_MAX, sizeof(lv_color_t));
#endif
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);
#else
    ss_fbdev_get_fb_buf((void **)&buf1, (void **)&buf2);
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);
#endif

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;

#if REFRESH_ONE_BUF
#if FULL_ONE_BUF
    disp_drv.direct_mode = 1;
    disp_drv.flush_cb   = ss_fbdev_flush_full_onebuf;
    disp_drv.gpu_fill_cb = ss_fbdev_fill_full_onebuf;
#else
    disp_drv.flush_cb   = ss_fbdev_flush_onebuf;
    // soft fill, not need to implement gpu_fill_cb

    // hard fill
    // disp_drv.gpu_fill_cb = ss_fbdev_fill_onebuf; // not implement
#endif
#else
    disp_drv.direct_mode = 1;
    disp_drv.full_refresh = 1;
    disp_drv.flush_cb   = ss_fbdev_flush_directly;
    disp_drv.gpu_fill_cb = ss_fbdev_fill_directly;
#endif
    
    disp_drv.hor_res    = LV_HOR_RES_MAX;
    disp_drv.ver_res    = LV_VER_RES_MAX;
    lv_disp_drv_register(&disp_drv);

    evdev_init();

    lv_indev_drv_init(&input_drv);
    input_drv.type = LV_INDEV_TYPE_POINTER;
    input_drv.read_cb = evdev_read;
    lv_indev_drv_register(&input_drv);

    /*Create a Demo*/
    lv_demo_widgets();
    //lv_demo_benchmark();
    //lv_demo_stress();
    //lv_demo_keypad_encoder();
    //lv_demo_music();

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
