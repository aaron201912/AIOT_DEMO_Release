使用方法:
指定资料路径：declare -x ALKAID_PATH= project所在的目录。
执行make,生成执行程序.



fb_clrkey:介绍fb的colorkey的使用方法
直接运行fb_clrkey

fb_color：介绍fb的r ，g，b，alpha的色彩标识方法。
直接运行fb_color

fb_cursor：介绍fb光标层的用法。
执行：out/fb_cursor
额外fb配置：
fbdev.ini：
```
# FBDEV 支持的硬件鼠标配置
[FB_CURSOR]
# 鼠标层使用的gop ID
FB_HWLAYER_ID = 0
# 鼠标层使用的gop graphic window ID
FB_HWWIN_ID = 0
# deprecated
FB_HWLAYER_DST = 3
# 鼠标层使用的颜色格式
# RGB565 = 1
# ARGB4444 = 2
# ARGB8888 = 5
# ARGB1555 = 6
# YUV422 = 9
# I8 = 4
# I4 = 13
# I2 = 14
FB_HWWIN_FORMAT = 6
# 修改Output color，0为RGB，1为YUV
FB_HWLAYER_OUTPUTCOLOR = 1
# 如果系统的mmap有layout项目为E_MMAP_ID_FB
# 那么FBDEV的鼠标层 将使用此处的内存
# 如果系统的mmap没有为FBDEV layout一块内存
# 那么FBDEV的鼠标层将自己申请128K内存
FB_MMAP_NAME = E_MMAP_ID_HW_CURSOR

# deprecated,fbdev 设备之间的z order(谁显示在上层或者下层)
[LAYER_ZORDER]
LAYER_ZORDER0 = 0
LAYER_ZORDER1 = 1
LAYER_ZORDER2 = 2
LAYER_ZORDER3 = 3
LAYER_ZORDER4 = 4
```

fb_dbbuf: 开启double buffer。
fb_stable\common\fb_common.c
static unsigned char _bUseDoubleBuffer = 1;
执行：out/fb_dbbuf


fb_dispattr： 测试FB_DisplayLayer
执行：out/fb_dispattr



fb_scale：测试fb scaler scrren
执行：out/fb_scale

fb_total: 基本包括上述范例。
执行：out/fb_total