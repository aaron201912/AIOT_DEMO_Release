Demo说明：
	本Demo主要针对H264/H265的视频流播放做演示(为考虑直观现象,也加入了简单的音频播放)

	默认使用panel显示, 如需enable hdmi显示, make ENABLE_HDMI = 1

一、编译说明：
    a.如果将demo放到跟project同级目录   （默认方式）
    --> make clean; make

    b.demo放置到任意路径，只需在make的时候指定ALKAID_PATH到工程的目录（需要链接SDK头文件）
    --> declare -x ALKAID_PATH=~/sdk/TAKOYAKI_DLC00V030/source_code/${mysdkrootpath}
    --> make  clean; make

二、生成bin文件：
    --> out/SsPlayer

三、Demo运行参数说明

************************* Video usage *************************

--vpath  : 指定video播放文件路径
-X : 选填,指定播放窗口起始X坐标,默认值为0
-Y : 选填,指定播放窗口起始Y坐标,默认值为0
-W : WIDTH,指定播放窗口宽度
-H : HEIGHT,指定播放窗口高度
-R : Rotate,[0-4]指定视频播放是否旋转:0=NONE,1=rotate_90,1=rotate_180,1=rotate_270
-N : Num [0-4] of vdec_channel of videolayer0,旋转播放几路视频，最大4路(注意这里最大值也包含了pip那路，当-N 选择主路4路时,PIP就出不来了)
-P ：PIP,[0/1]开关，启用videolayer1,播放pip(PIP坐标自动根据屏参做了居中处理)

eg:
	./SsPlayer --vpath ./res/720P25.h264 -X 0 -Y 0 -W 1024 -H 600 -R 0 -N 3 -P 1

************************* Audio usage *************************
--apath : 指定audio播放文件路径
-s : samplerate,指定audio播放采样率
-c : channel,指定audio播放channel数:0=MONO,1=STEREO
-v : volume:[-60-30],指定audio播放的音量,单位是db
eg:
	./Ssplayer --apath res/48k_stereo.wav -c 2 -s 48000 -v -10


*可以结合音视频一起播放:
./SsPlayer --vpath ./res/720P25.h264 -X 0 -Y 0 -W 1024 -H 600 -R 0 -N 3 -P 1 --apath res/48k_stereo.wav -c 2 -s 48000 -v -10
