��Demo��Ҫ��ָ��Jpeg��ͼ�������ָ����ʾ�ն���ʾ(Panel),֧��90����ת���ο�����˵��

һ������˵����
    a.�����demo�ŵ���projectͬ��Ŀ¼��Ĭ�Ϸ�ʽ��
      --> make clean;make

    b.demo���õ�����·����ֻ����make��ʱ��ָ��project��Ŀ¼����Ҫ����SDKͷ�ļ���
	  --> declare -x ALKAID_PATH=~/source_code   //����source_code/project
      --> make clean;make 

��������bin�ļ���
    --> out/JpegPlayer

��������˵��:
	--pic_path : ָ����Ҫ���ŵ�JpegͼƬ
	-R         : ��ѡ����[0,1],1����ʹ����ת90�ȣ�DefaultΪ0
	
�ġ�����:
	1.��libĿ¼export������������export LD_LIBRARY_PATH=/lib:$PWD/lib:$LD_LIBRARY_PATH
	2.��������: ./JpegPlayer --pic_path  res/logo_800_480.jpg
	3.��ת90��: ./JpegPlayer --pic_path  res/logo_800_480.jpg -R 1

�塢ע������:
1.�粻ʹ�ù����panel,���滻����
2./config/fbdev.ini�����FB_WIDTH��FB_HEIGHT��Ҫ���ָ���Ļ�ֱ���һ��
	FB_WIDTH = 1024
	FB_HEIGHT = 600
