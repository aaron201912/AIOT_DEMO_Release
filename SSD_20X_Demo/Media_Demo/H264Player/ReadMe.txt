Demo˵����
	��Demo��Ҫ���H264/H265����Ƶ����������ʾ(Ϊ����ֱ������,Ҳ�����˼򵥵���Ƶ����)

	Ĭ��ʹ��panel��ʾ, ����enable hdmi��ʾ, make ENABLE_HDMI = 1

һ������˵����
    a.�����demo�ŵ���projectͬ��Ŀ¼   ��Ĭ�Ϸ�ʽ��
    --> make clean; make

    b.demo���õ�����·����ֻ����make��ʱ��ָ��ALKAID_PATH�����̵�Ŀ¼����Ҫ����SDKͷ�ļ���
    --> declare -x ALKAID_PATH=~/sdk/TAKOYAKI_DLC00V030/source_code/${mysdkrootpath}
    --> make  clean; make

��������bin�ļ���
    --> out/SsPlayer

����Demo���в���˵��

************************* Video usage *************************

--vpath  : ָ��video�����ļ�·��
-X : ѡ��,ָ�����Ŵ�����ʼX����,Ĭ��ֵΪ0
-Y : ѡ��,ָ�����Ŵ�����ʼY����,Ĭ��ֵΪ0
-W : WIDTH,ָ�����Ŵ��ڿ��
-H : HEIGHT,ָ�����Ŵ��ڸ߶�
-R : Rotate,[0-4]ָ����Ƶ�����Ƿ���ת:0=NONE,1=rotate_90,1=rotate_180,1=rotate_270
-N : Num [0-4] of vdec_channel of videolayer0,��ת���ż�·��Ƶ�����4·(ע���������ֵҲ������pip��·����-N ѡ����·4·ʱ,PIP�ͳ�������)
-P ��PIP,[0/1]���أ�����videolayer1,����pip(PIP�����Զ������������˾��д���)

eg:
	./SsPlayer --vpath ./res/720P25.h264 -X 0 -Y 0 -W 1024 -H 600 -R 0 -N 3 -P 1

************************* Audio usage *************************
--apath : ָ��audio�����ļ�·��
-s : samplerate,ָ��audio���Ų�����
-c : channel,ָ��audio����channel��:0=MONO,1=STEREO
-v : volume:[-60-30],ָ��audio���ŵ�����,��λ��db
eg:
	./Ssplayer --apath res/48k_stereo.wav -c 2 -s 48000 -v -10


*���Խ������Ƶһ�𲥷�:
./SsPlayer --vpath ./res/720P25.h264 -X 0 -Y 0 -W 1024 -H 600 -R 0 -N 3 -P 1 --apath res/48k_stereo.wav -c 2 -s 48000 -v -10
