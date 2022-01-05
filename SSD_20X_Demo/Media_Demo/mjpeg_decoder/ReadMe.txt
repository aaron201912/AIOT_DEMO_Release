主要功能：
	支持MJPEG摄像头的主要功能：
		1.通过uvc协议从usb camera抓取mjpeg流；
		2.出来一路 720p分辨率的流给到libjpeg和libyuv之后转为为yuy2格式，yuy2格式给到divp做格式转换（nv12）和scaling up/down(缩放到屏的长宽)，然后divp给到disp显示到屏上。
编译方法：
	1.将demo文件夹放到跟project同一级目录下面的两级目录，或是修改对应的make file，make clean;make

运行方法；
	1.插入usb摄像头，确认板子生成/dev/video0后再运行demo
	
注意事项：
运行demo前必须先插上uvc摄像头，确认板子生成/dev/video0后再运行demo

kernel需打开如下config
CONFIG_USB_VIDEO_CLASS
CONFIG_USB  
CONFIG_MEDIA_SUPPORT  
CONFIG_MEDIA_USB_SUPPORT   
CONFIG_MEDIA_CAMERA_SUPPORT   
CONFIG_VIDEO_V4L2 
