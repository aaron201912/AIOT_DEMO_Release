如果采用v030版本sdk，需要同步/config/wifi/wlan.json文件



wifiDemo使用说明：
1. 修改sample code，添加wifi热点信息。  
`// 连接热点信息, 可替换为其它指定的热点，如:ssid为"EnglishName"，密码为"12345678"`
```
    static MI_WLAN_ConnectParam_t g_stConnectParam[] = {
    {
       E_MI_WLAN_SECURITY_WPA,`
        "EnglishName",`
        "12345678",
        5000
    },
    {
        E_MI_WLAN_SECURITY_WPA,
        "ABC",
        "abcdefg",
        5000
    }
};
```

2. 修改 Makefile 中的 PROJECT_PATH， 使用本地的 sdk 路径， 然后执行make clean;make 编译 demo code。 
`#赋值对应的路径,此路径需要和本地sdk路径一致`
`PROJECT_PATH=/home/koda.xu/xxx/develop_p3_bringup/project`    


3. 运行testWifi。将生成的testWifi拷贝到目标板中。App运行后会出现下面的提示：
```
    please input option command:
    1. switch STA/AP mode, input 'm'
    2. change wifi hotspot in list, input 'a'
    3. connect wifi hotspot, input 'n'
    4. disconnect wifi hotspot, input 'd'
    5. print wifi hotspot's info, input 'p'
    6. exit, input 'q'

    输入‘m‘， 切换 STA/AP 模式，默认是 STA 模式。用户可在 code 里面预设需要连接的 wifi 热点列表;    
    输入’a’，切换到列表中的下一个连接热点，默认从 0 开始循环；  
    输入‘n’，在 STA 模式下连接指定的 wifi，在 AP 模式下，打开个人热点； 
    输入‘d’，在 STA 模式下断开连接，在 AP 模式下，关闭个人热点；    
    输入‘d’，在 STA 模式下断开连接，在 AP 模式下，关闭个人热点；    
    输入‘q’，退出 app。 
```

4. 测试流程示例：  
    STA模式：   
    n(连接)-> p(打印扫描热点信息) -> d(断开连接) -> a(切换到下一个热点) -> n(连接) ->d(断开连接)    

    AP模式：        
    m(切换STA/AP模式，默认STA模式，输入m切换至AP模式) -> n(开启热点) -> p(当有设备接入时，打印已连接的设备信息) -> d(断开连接) 

