��Ҫ���ܣ�
	֧��MJPEG����ͷ����Ҫ���ܣ�
		1.ͨ��uvcЭ���usb cameraץȡmjpeg����
		2.����һ· 720p�ֱ��ʵ�������libjpeg��libyuv֮��תΪΪyuy2��ʽ��yuy2��ʽ����divp����ʽת����nv12����scaling up/down(���ŵ����ĳ���)��Ȼ��divp����disp��ʾ�����ϡ�
���뷽����
	1.��demo�ļ��зŵ���projectͬһ��Ŀ¼���������Ŀ¼�������޸Ķ�Ӧ��make file��make clean;make

���з�����
	1.����usb����ͷ��ȷ�ϰ�������/dev/video0��������demo
	
ע�����
����demoǰ�����Ȳ���uvc����ͷ��ȷ�ϰ�������/dev/video0��������demo

kernel�������config
CONFIG_USB_VIDEO_CLASS
CONFIG_USB  
CONFIG_MEDIA_SUPPORT  
CONFIG_MEDIA_USB_SUPPORT   
CONFIG_MEDIA_CAMERA_SUPPORT   
CONFIG_VIDEO_V4L2 
