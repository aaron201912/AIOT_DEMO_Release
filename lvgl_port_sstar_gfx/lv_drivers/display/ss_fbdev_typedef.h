#ifndef __SS_FBDEV_TYPEDEF_H__
#define __SS_FBDEV_TYPEDEF_H__

#if 1
//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------

#define MI_SUCCESS 0

/// data type unsigned char, data length 1 byte
typedef unsigned char MI_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MI_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int MI_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MI_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MI_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MI_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int MI_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MI_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MI_FLOAT; // 4 bytes
/// data type 64bit physical address
typedef unsigned long long MI_PHY; // 8 bytes
/// data type pointer content
typedef unsigned long MI_VIRT; // 4 bytes when 32bit toolchain, 8 bytes when 64bit toolchain.

typedef unsigned char MI_BOOL;

typedef struct
{
    size_t len;
    void *pVa;
    MI_PHY phy;
}st_MI_ADDR;
#endif

#if 1
typedef enum
{
    E_MI_GFX_FMT_I1 = 0, /* MS_ColorFormat */
    E_MI_GFX_FMT_I2,
    E_MI_GFX_FMT_I4,
    E_MI_GFX_FMT_I8,
    E_MI_GFX_FMT_FABAFGBG2266,
    E_MI_GFX_FMT_1ABFGBG12355,
    E_MI_GFX_FMT_RGB565,
    E_MI_GFX_FMT_ARGB1555,
    E_MI_GFX_FMT_ARGB4444,
    E_MI_GFX_FMT_ARGB1555_DST,
    E_MI_GFX_FMT_YUV422,
    E_MI_GFX_FMT_ARGB8888,
    E_MI_GFX_FMT_RGBA5551,
    E_MI_GFX_FMT_RGBA4444,
    E_MI_GFX_FMT_ABGR8888,
    E_MI_GFX_FMT_BGRA5551,
    E_MI_GFX_FMT_ABGR1555,
    E_MI_GFX_FMT_ABGR4444,
    E_MI_GFX_FMT_BGRA4444,
    E_MI_GFX_FMT_BGR565,
    E_MI_GFX_FMT_RGBA8888,
    E_MI_GFX_FMT_BGRA8888,
    E_MI_GFX_FMT_MAX
} MI_GFX_ColorFmt_e;

typedef enum
{
    E_MI_GFX_RGB_OP_EQUAL = 0,
    E_MI_GFX_RGB_OP_NOT_EQUAL,
    E_MI_GFX_ALPHA_OP_EQUAL,
    E_MI_GFX_ALPHA_OP_NOT_EQUAL,
    E_MI_GFX_ARGB_OP_EQUAL,
    E_MI_GFX_ARGB_OP_NOT_EQUAL,
    E_MI_GFX_CKEY_OP_MAX,
} MI_GFX_ColorKeyOp_e;

typedef enum
{
    E_MI_GFX_DFB_BLD_ZERO = 0,
    E_MI_GFX_DFB_BLD_ONE,
    E_MI_GFX_DFB_BLD_SRCCOLOR,
    E_MI_GFX_DFB_BLD_INVSRCCOLOR,
    E_MI_GFX_DFB_BLD_SRCALPHA,
    E_MI_GFX_DFB_BLD_INVSRCALPHA,
    E_MI_GFX_DFB_BLD_DESTALPHA,
    E_MI_GFX_DFB_BLD_INVDESTALPHA,
    E_MI_GFX_DFB_BLD_DESTCOLOR,
    E_MI_GFX_DFB_BLD_INVDESTCOLOR,
    E_MI_GFX_DFB_BLD_SRCALPHASAT,
    E_MI_GFX_DFB_BLD_MAX,
} MI_GFX_DfbBldOp_e;

typedef enum
{
    E_MI_GFX_MIRROR_NONE = 0,
    E_MI_GFX_MIRROR_HORIZONTAL,
    E_MI_GFX_MIRROR_VERTICAL,
    E_MI_GFX_MIRROR_BOTH,
    E_MI_GFX_MIRROR_MAX
} MI_GFX_Mirror_e;

typedef enum
{
    E_MI_GFX_ROTATE_0 = 0,
    E_MI_GFX_ROTATE_90,
    E_MI_GFX_ROTATE_180,
    E_MI_GFX_ROTATE_270,
    E_MI_GFX_ROTATE_MAX
} MI_GFX_Rotate_e;

#define MI_GFX_INITIAL_ERROR_CODE (0x220)
typedef enum
{
    E_MI_GFX_ERR_NOT_INIT = MI_GFX_INITIAL_ERROR_CODE,
    E_MI_GFX_ERR_GFX_DRV_NOT_SUPPORT,
    E_MI_GFX_ERR_GFX_DRV_FAIL_FORMAT,
    E_MI_GFX_ERR_GFX_NON_ALIGN_ADDRESS,
    E_MI_GFX_ERR_GFX_NON_ALIGN_PITCH,
    E_MI_GFX_ERR_GFX_DRV_FAIL_OVERLAP,
    E_MI_GFX_ERR_GFX_DRV_FAIL_STRETCH,
    E_MI_GFX_ERR_GFX_DRV_FAIL_ITALIC,
    E_MI_GFX_ERR_GFX_DRV_FAIL_LOCKED,
    E_MI_GFX_ERR_GFX_DRV_FAIL_BLTADDR,
    E_MI_GFX_ERR_MAX
} MI_GFX_ErrCode_e;

//=============================================================================
// GFX point
//=============================================================================
typedef struct MI_GFX_Point_s
{
    /// x coordinate
    MI_S16 s16x;
    /// y coordinate
    MI_S16 s16y;
} MI_GFX_Point_t;

typedef struct MI_GFX_Rect_s
{
    MI_S32 s32Xpos;
    MI_S32 s32Ypos;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_GFX_Rect_t;

//=============================================================================
// GFX line pattern infomation struct
//=============================================================================
typedef struct MI_GFX_Line_s
{
    /// start point of line
    MI_GFX_Point_t stPointFrom;
    /// end point of line
    MI_GFX_Point_t stPointTo;
    /// line width in pixel
    MI_U16 u16Width;
    /// Constant color or  Gradient color
    MI_BOOL bColorGradient;
    /// color range from start to end
    MI_U32 u32ColorFrom;
    ///
    MI_U32 u32ColorTo;
} MI_GFX_Line_t;

//=============================================================================
// GFX palette information
//=============================================================================
typedef union
{
    /// ARGB8888 byte order
    struct
    {
        MI_U8 u8A;
        MI_U8 u8R;
        MI_U8 u8G;
        MI_U8 u8B;
    } RGB;
    // u8Data[0] = u8A
    // u8Data[1] = u8R
    // u8Data[2] = u8G
    // u8Data[3] = u8B
    MI_U8 u8Data[4];
} MI_GFX_PaletteEntry_t;

typedef struct MI_GFX_Palette_s
{
    /// array subscripts are indentical to value of Index Color
    MI_GFX_PaletteEntry_t aunPalette[256];
    /// Starting Index in palette to config
    MI_U16 u16PalStart;
    /// Ending Index in palette to config
    MI_U16 u16PalEnd;
}MI_GFX_Palette_t;

typedef struct MI_GFX_ColorKey_s
{
    MI_U32 u32ColorStart;
    MI_U32 u32ColorEnd;
} MI_GFX_ColorKeyValue_t;

typedef struct MI_GFX_ColorKeyInfo_s
{
    MI_BOOL bEnColorKey;
    MI_GFX_ColorKeyOp_e eCKeyOp;
    MI_GFX_ColorFmt_e eCKeyFmt;
    MI_GFX_ColorKeyValue_t stCKeyVal;
} MI_GFX_ColorKeyInfo_t;

typedef struct MI_GFX_Surface_s
{
    MI_PHY phyAddr;
    MI_GFX_ColorFmt_e eColorFmt;
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32Stride;
} MI_GFX_Surface_t;

typedef enum
{
    E_MI_GFX_DFB_BLEND_NOFX = 0x00000000,
    E_MI_GFX_DFB_BLEND_COLORALPHA = 0x00000001,
    E_MI_GFX_DFB_BLEND_ALPHACHANNEL = 0x00000002,
    E_MI_GFX_DFB_BLEND_COLORIZE = 0x00000004,
    E_MI_GFX_DFB_BLEND_SRC_PREMULTIPLY = 0x00000008,
    E_MI_GFX_DFB_BLEND_SRC_PREMULTCOLOR = 0x00000010,
    E_MI_GFX_DFB_BLEND_DST_PREMULTIPLY = 0x00000020,
    E_MI_GFX_DFB_BLEND_XOR = 0x00000040,
    E_MI_GFX_DFB_BLEND_DEMULTIPLY = 0x00000080,
    E_MI_GFX_DFB_BLEND_SRC_COLORKEY = 0x00000100,
    E_MI_GFX_DFB_BLEND_DST_COLORKEY = 0x00000200,
    E_MI_GFX_DFB_BLEND_MAX = 0x3FF
} MI_Gfx_DfbBlendFlags_e;

typedef struct MI_GFX_Opt_s
{
    MI_GFX_Rect_t stClipRect;
    MI_GFX_ColorKeyInfo_t stSrcColorKeyInfo;
    MI_GFX_ColorKeyInfo_t stDstColorKeyInfo;
    MI_GFX_DfbBldOp_e eSrcDfbBldOp;
    MI_GFX_DfbBldOp_e eDstDfbBldOp;
    MI_GFX_Mirror_e eMirror;
    MI_GFX_Rotate_e eRotate;
    MI_Gfx_DfbBlendFlags_e eDFBBlendFlag;
    MI_U32 u32GlobalSrcConstColor;
    MI_U32 u32GlobalDstConstColor;
} MI_GFX_Opt_t;

typedef struct MI_GFX_DevAttr_s
{
    MI_U32 u32DevId;
    MI_U8 *u8Data;
} MI_GFX_DevAttr_t;

typedef MI_S32 MI_GFX_DEV;
#endif

#endif // __SS_FBDEV_TYPEDEF_H__

