本Demo主要是指定Jpeg出图并输出到指定显示终端显示(Panel),支持90度旋转，参考参数说明

一、编译说明：
    a.如果将demo放到跟project同级目录（默认方式）
      --> make clean;make

    b.demo放置到任意路径，只需在make的时候指定project的目录（需要链接SDK头文件）
	  --> declare -x ALKAID_PATH=~/source_code   //假设source_code/project
      --> make clean;make 

二、生成bin文件：
    --> out/JpegPlayer

三、参数说明:
	--pic_path : 指定需要播放的Jpeg图片
	-R         : 可选参数[0,1],1代表使能旋转90度，Default为0
	
四、运行:
	1.将lib目录export到环境变量：export LD_LIBRARY_PATH=/lib:$PWD/lib:$LD_LIBRARY_PATH
	2.正常运行: ./JpegPlayer --pic_path  res/logo_800_480.jpg
	3.旋转90度: ./JpegPlayer --pic_path  res/logo_800_480.jpg -R 1

五、注意事项:
1.如不使用公版的panel,请替换屏参
2./config/fbdev.ini里面的FB_WIDTH和FB_HEIGHT需要保持跟屏幕分辨率一致
	FB_WIDTH = 1024
	FB_HEIGHT = 600
