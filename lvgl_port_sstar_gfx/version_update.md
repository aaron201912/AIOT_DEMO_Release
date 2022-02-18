2022/2/18:
先基于当前github维护。code base为：https://gitee.com/tianhuihe/lv_port_sstar_gfx.git

```
原方式下载为：
git clone https://gitee.com/tianhuihe/lv_port_sstar_gfx.git -b porting_gfx_double_buffer
cd lv_port_sstar_gfx
git submodule init && git submodule update

提供的版本主要包含3个部分：
lvgl，lv_drivers，lv_demos，对应github code base版本分别如下：
lvgl：
https://github.com/lvgl/lvgl.git
commit：61b0de3bdc46803ae245cf17b453678fd0b30d73

lv_drivers：
https://github.com/lvgl/lv_drivers.git
commit：ec9ddf93c4e5b1feed9c53c54927e55ff4b1dd89

lv_demos:
https://github.com/lvgl/lv_demos.git
commit：bc191bdee0da25ebba542c3f71c0671b75224ee5
```

lvgl绘制：
lvgl绘制缓冲区有3种配置：
1. 一个缓冲区
将屏幕内容绘制到一个缓冲区中并将其发送到显示器。缓冲区可以小于屏幕。在这种情况下，较大的区域将在多个部分中重新绘制。
如果只有小区域发生变化（例如按下按钮），则只会刷新这些区域。

2. 具有两个缓冲区的两个非屏幕大小的缓冲区LVGL 	// ping-pong buffer
可以将其绘制到一个缓冲区中，而将另一个缓冲区的内容发送到后台显示。应该使用DMA或其他硬件将数据传输到显示器，让CPU同时绘制。
这样，显示的渲染和刷新变得并行。与One buffer类似，如果缓冲区小于要刷新的区域，LVGL 将分块绘制显示内容。

3. 两个屏幕大小的缓冲区
与两个非屏幕大小的缓冲区相比，LVGL 将始终提供整个屏幕的内容，而不仅仅是块。
通过这种方式，驱动程序可以简单地将帧缓冲区的地址更改为从 LVGL 接收到的缓冲区。

性能分析：
1. 原gitee code master分支默认使用cpu方式，使用上述方式的第一种。创建一个小于屏幕大小的缓冲buffer，将刷新区域先绘制到缓冲buffer中，再将缓冲buffer拷贝到frame buffer上。若刷新区域大于设定的缓冲buffer大小，则会分成多次拷贝至frame buffer。
这种方式虽使用cpu绘图，但是绘制缓冲较小，一般情况下，刷新只需要重绘较较小的区域。而且这种方式使用的fb为单buffer，就是说显示和绘制操作同一张buffer，这样界面刷新时会存在显示撕裂的问题。

2. 原gitee code porting_gfx_double_buffer分支分支使用gfx加速，同样是使用上面的第一种方式，但是创建的是全屏的buffer，使用的fb为double buffer。做法是每次刷新都会将完整屏幕大小的数据拷贝到fb的后台buffer中，然后将后台buffer交换到前台显示，后续刷新再继续更新整个屏幕至后台buffer，如此往复。
实测这种方式cpu loading反而比使用cpu绘制局部buffer的方式更高，通过抓取火焰图观测，发现gfx并未占用过多资源，应用部分的整体loading增长明显，推测实际是绘制到缓冲buffer的部分占用cpu过高。
且这种方式使用了gfx加速，但仅在无ui遮罩情况下的fill操作做了加速，其它场景下的填充和混合操作并未实现gfx加速。而这些场景对性能影响较为明显。
#define REFRESH_ONE_BUF 1
#define FULL_ONE_BUF    1

3. 现改为第三种方式绘制，使用fb double buffer，直接map为两张屏幕大小的buffer，然后在映射buffer上绘制。这样相较于第二种方式可以省掉一次全屏buffer拷贝的过程。测试发现cpu loading略有改善，但仍偏高。快速刷新时界面会有闪动。
还需要对绘制到缓冲buffer的这部分做优化，这部分包含颜色的填充，图片的填充，alpha混合等，其中主要是alpha混合对性能影响较为明显。
#define REFRESH_ONE_BUF 0
#define FULL_ONE_BUF    0

现有版本还没有上述部分的处理，这部分的对接工作较大，还在继续处理中。

