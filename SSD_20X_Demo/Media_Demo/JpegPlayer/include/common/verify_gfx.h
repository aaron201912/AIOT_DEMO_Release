#include <mi_gfx.h>
#include <mi_gfx_datatype.h>

#include <mi_sys.h>


// get bytesPerPixel each format
unsigned int getBpp(MI_GFX_ColorFmt_e eFmt);
int _gfx_alloc_surface(MI_GFX_Surface_t *pSurf, char **data,char  *surfName);
void _gfx_free_surface(MI_GFX_Surface_t *pSurf, char *data);
void _gfx_sink_surface(MI_GFX_Surface_t *pSurf, const char *data,const char  *name);

int __test_rotate_ARGB_Normal(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                 MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf);

int __test_rotate_ARGB_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                   MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf);

int __test_rotate_ARGB_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                      MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf);

int __test_rotate_ARGB_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                        MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_YUV420SP_90(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                           MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf);

int __test_rotate_YUV420SP_180(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                              MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf);

int __test_rotate_YUV420SP_270(MI_GFX_Surface_t srcSurf, MI_GFX_Rect_t srcRect,
                               MI_GFX_Surface_t dstSurf, MI_GFX_Rect_t dstRect, char *dstData, MI_BOOL bSinkSurf);


