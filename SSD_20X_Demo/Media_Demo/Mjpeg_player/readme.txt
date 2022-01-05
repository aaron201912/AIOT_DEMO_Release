Demo说明：
		本Demo主要是针对解析Mjpeg文件(不支持解压后YUV444的格式)，通过libjpeg-turbo解压Mjpeg为YUV数据，将YUV数据喂到DIVP/DISP，最终显示画面到panel

一、编译说明：
    a.如果将demo放到跟project同级目录（默认方式）
      --> make clean；make

    b.demo放置到任意路径，只需在make的时候指定project的目录（需要链接SDK头文件）
    	--> declare -x ALKAID_PATH=~/sdk/TAKOYAKI_DLC00V030/source_code/mysdkrootpath
      --> make clean；make 

二、生成bin文件：
    --> out/Mjpeg_Demo

三、Demo运行参数说明：
    a.指定Demo运行依赖环境：
       --> export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib/libjpeg-turbo/lib32/

    b.参数说明： 
		--mpath  : 指定mjpeg播放文件路径
		-E : 选填,指定是否使能DIVP
		-R : 选填,[0,1]指定是否旋转90度，默认值为0不旋转
		
		eg：
			./MjpegPlayer --mpath res/test.mjpeg -E 1 -R 1