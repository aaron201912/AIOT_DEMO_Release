/**
 * @file ss_fbdev.h
 *
 */

#ifndef __SS_FBDEV_H__
#define __SS_FBDEV_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_SS_FBDEV

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void ss_fbdev_init(void);
void ss_fbdev_exit(void);
int ss_fbdev_get_draw_buf(void **draw_buf1, void **draw_buf2, lv_coord_t hor_res, lv_coord_t ver_res, int bpp);
int ss_fbdev_get_fb_buf(void **buf1, void **buf2);
void ss_fbdev_fill(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color);
void ss_fbdev_fill_ex(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color);
void ss_fbdev_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);
void ss_fbdev_flush_ex(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);

// one small buffer refresh
void ss_fbdev_fill_onebuf(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color);
void ss_fbdev_flush_onebuf(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);

// one full buffer refresh
void ss_fbdev_fill_full_onebuf(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color);
void ss_fbdev_flush_full_onebuf(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);

// directly draw on fb buffer
void ss_fbdev_fill_directly(struct _lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color);
void ss_fbdev_flush_directly(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);


void ss_fbdev_get_sizes(uint32_t *width, uint32_t *height);

/**********************
 *      MACROS
 **********************/

#endif /*USE_SS_FBDEV*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__SS_FBDEV_H__*/

