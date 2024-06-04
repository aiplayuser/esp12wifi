# esp12wifi

目前物联网iot设备都有各自的app，互不兼容，即使同一品牌，还要适配不同平台单独开发app，

使用homeassistant貌似要搭建服务器，门槛又太高。

该项目基于arduino和esp8266，以通用的web方式订阅发送MQTT消息，

我是新手，写不了复杂的代码，该项目没有考虑性能和安全问题，

功能还比较简单，这里只是个简单例子说明我的意图，希望抛砖引玉集思广益，

不知道能不能加入摄像头管理功能，毕竟ipcam本来就是web管理的。

感觉web接口更容易接入gpt管理iot设备，人工智能时代，那一堆app大概率会淘汰。

这不是一个库，不过作为库来使用也没啥问题，这里只是集成打包了一些常用功能。

克隆该项目到你的arduino项目文件夹Documents\Arduino\libraries就可以使用了。

config.json是一个基本配置文件，有几个免费的mqtt服务器，按需修改上传至开发板即可。

mqtt.html是一个简单的web页面，可以收发mqtt消息。

基本使用：

#include "Esp12wifi.h"

void setup(){ setup1(); }

void loop(){ loop1(); }

上传程序首次启动会以开发板id建立一个wifi信号，手机连接该信号会弹出配置页面，

选择重新连接路由器搜索附近可上网的wifi信号，输入密码连接。

返回首页查看开发板信息，mqtt服务器后面数字为0连接成功，其他数字连接失败。

重新修改开发板ssid和pass以确保安全性，

设置一个足够复杂的mqtt主题，比如20位以上，防止和别人的主题相互干扰。

把mqtt.html上传至手机，用浏览器打开并收藏或者添加到桌面，方便下次使用。

把mqtt主题复制到这里，点连接会自动订阅这个主题，并搜索该主题下的所有设备。

![alt text](https://github.com/aiplayuser/esp12wifi/blob/main/image1.PNG)
![alt text](https://github.com/aiplayuser/esp12wifi/blob/main/image2.PNG)