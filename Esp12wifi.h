#ifndef _ESP12WIFI_H_
#define _ESP12WIFI_H_

#include "Arduino.h"
#include "DNSServer.h" //自动弹窗
#include "ArduinoJson.h" //json解析
#include <PubSubClient.h> //mqtt服务
#include <FS.h> //闪存系统

#ifdef ESP8266  
    #include "ESP8266WiFi.h" //wifi服务
    #include "ESP8266WebServer.h" //web服务
    ESP8266WebServer webserver(80); //web对象 
    #define espmodel "Esp12" //芯片类型
    #define leddark 1 //这个led太刺眼了，不想让他一直亮着。
#elif defined(ESP32)   
    #include "WiFi.h" //wifi服务
    #include "WebServer.h" //web服务
    WebServer webserver(80); //web对象
    #define espmodel "Esp32" //芯片类型
    #define leddark 0 //这个led太刺眼了，不想让他一直亮着。
#endif

File file; DNSServer dnsserver; WiFiClient wificlient; PubSubClient mqttclient(wificlient); DynamicJsonDocument doc(22222); //json对象占用内存计算麻烦，自己估算吧！
String espid, ssid, pass, mqttserver, mqttmess, topic; //常用变量

String httphead(){ return R"( <!DOCTYPE html><html>
    <head><meta charset=UTF-8><style>a{TEXT-DECORATION:none}</style><meta name=viewport content=\"width=device-width,initial-scale=1.2\"></head>
    <body><p><form action=/again method=POST><input type=submit value=重新启动>&nbsp&nbsp<a href=/>返回首页</a></form><p>
    <form action=/scanap method=POST><input type=submit value=重新连接路由器></form><p>   )" ;   }  //统一页面的顶部

void setbeep(int pin,int f,int n,int d){ int tn=n*d; while(tn--){ tone(pin,f,300/n); delay(1000/n); } }  //自定义蜂鸣器快setbeep(D5,3333,5,1)慢setbeep(D5,2222,2,1)定时器中不能使用

void sendmqtt(String topic,String message){ mqttclient.publish( topic.c_str(), message.c_str() ); } //向主题发布消息

void callback(char* topic, byte* payload, unsigned int length) { char payloadchar[length + 1]; memcpy(payloadchar, payload, length); payloadchar[length] = '\0'; 
    mqttmess=(String)payloadchar; if(mqttmess=="正在查找设备"){ sendmqtt(topic,"<a href=http://" + WiFi.localIP().toString() + " target=_blank>" + ssid + "</a>" ); mqttmess=""; }  }  
    //收到mqtt消息的回调函数，返回一个包含设备ip的链接。 

void connectmqtt(){ mqttclient.setServer(mqttserver.c_str(), 1883); mqttclient.setKeepAlive(60); mqttclient.setCallback(callback); 
    mqttclient.connect(espid.c_str()); mqttclient.subscribe(topic.c_str()); }  //连接mqtt服务器 

void homepage(){ String html = httphead() + String(espmodel) + "-" + espid + "-" + String(ESP.getFlashChipSize()/1024/1024) + "MB<p>APIP: " ;

    html+= WiFi.softAPIP().toString() + "<p>" + WiFi.SSID() + ": " + WiFi.localIP().toString() + "<p>" ; //开发板信息

    html+= "<form action=/upload name=form1 method=POST enctype=multipart/form-data><input type=file name=data onchange=document.form1.submit()></form><p>" ; //上传文件

    #ifdef ESP8266
        FSInfo fsinfo; SPIFFS.info(fsinfo); html+="闪存"+String(fsinfo.usedBytes)+"/"+String(fsinfo.totalBytes)+"字节<p>"; //闪存信息
        Dir dir=SPIFFS.openDir("/"); while(dir.next()){ html+="<form action=/delone method=post><a href="+dir.fileName()+">"+dir.fileName()+"</a>--"+dir.fileSize();
                                                        html+="<input name=mingzi style=width:0px value="+dir.fileName()+"><input type=submit value=删除></form><p>";} 
    #elif defined(ESP32)
        file = SPIFFS.open("/", "r"); while(File entry = file.openNextFile()){  //我的esp32有点问题，没有过多测试。
                                  html+="<form action=/delone method=post><a href="+String(entry.name())+">"+String(entry.name())+"</a>--"+String(entry.size());
                                  html+="<input name=mingzi style=width:0px value="+String(entry.name())+"><input type=submit value=删除></form><p>"; entry.close(); } file.close();
    #endif

    html+= "<form action=/setap method=post><input name=ssid style=width:111px value="+ssid+"><input name=pass style=width:111px value="+pass+"><p>\
                                            <input name=mqttserver style=width:211px value="+mqttserver+">-"+String(mqttclient.state())+"<p>\
                                            <input name=topic style=width:144px value="+topic+"><input type=submit value=设置config></form><p>" ;

    html+= "<form action=/delall method=POST><input type=submit value=删除所有文件></form><p></body></html>"; webserver.send(200,"text/html",html) ;
    }

void cjson(){ doc.clear(); file.close(); } //关闭json对象
void rjson(String jsonfile){ file = SPIFFS.open(jsonfile,"r"); deserializeJson(doc,file); } //读取json对象
void wjson(String jsonfile){ file = SPIFFS.open(jsonfile,"w"); serializeJson(doc,file); cjson(); } //写入json对象

void again(){ webserver.send(200,"text/html",httphead()+"<p><h2>正在重启设备</h2><p></body></html>"); delay(2222); ESP.restart(); }

String getChip() { uint8_t mac[6]; WiFi.macAddress(mac); 
    uint32_t chipId = (mac[2]<<24) | (mac[3]<<16) | (mac[4]<<8) | mac[5] ; return String(chipId); } //获取主板id

String gettype(String path){ if( path.endsWith(".htm") || path.endsWith(".html") ) return "text/html"; 
    else if(path.endsWith(".json")) return "text/json"; return "application/octet-stream"; } //获取文件类型

void setup1(){ Serial.begin(9600); Serial.println("\nbegin"); pinMode(0,INPUT_PULLUP); pinMode(2,OUTPUT); SPIFFS.begin(); espid=getChip(); //初始化系统

    rjson("/config.json"); ssid=doc["ssid"].as<String>(); pass=doc["pass"].as<String>(); topic=doc["topic"].as<String>(); mqttserver=doc["mqttserver"][0].as<String>(); cjson(); 
    if(ssid=="null")ssid=espid; if(pass=="null")pass=""; if(topic=="null")topic="jkdajdlsfhwdsjahweufsd"; if(mqttserver=="null")mqttserver="broker.hivemq.com";  //读取config

    WiFi.mode(WIFI_AP_STA); WiFi.softAP(ssid,pass); WiFi.begin(); webserver.begin(); dnsserver.start(53,"*",IPAddress(192,168,4,1)); //开始所有服务

    webserver.onNotFound( [](){ String path=webserver.uri(); if(!SPIFFS.exists(path)){ homepage(); }
        else{ file=SPIFFS.open(path,"r"); webserver.streamFile(file,gettype(path)); file.close(); } } ); //处理所有请求  

    webserver.on("/",HTTP_GET,homepage);//进入首页
    
    webserver.on("/upload", HTTP_POST, [](){ webserver.send(200); }, [](){ HTTPUpload& upload = webserver.upload(); bool isbin = upload.filename.endsWith(".bin"); 
        if ( upload.status == UPLOAD_FILE_START ) { isbin?Update.begin((ESP.getFreeSketchSpace()-0x1000)&0xFFFFF000,U_FLASH):file=SPIFFS.open("/"+upload.filename,"w"); } 
        else if ( upload.status == UPLOAD_FILE_WRITE ) { isbin ? Update.write(upload.buf, upload.currentSize) : file.write(upload.buf,upload.currentSize); } 
        else if ( upload.status == UPLOAD_FILE_END ) { String updatastr = Update.end(true)?"系统已更新":"系统未更新"; 
            isbin?webserver.send( 200,"text/html",httphead()+"<p><h2>"+updatastr+"</h2><p></body></html>" ):homepage(); file.close(); }            
        } );  //上传文件更新系统
        
    webserver.on("/again",again); //重新启动系统
    webserver.on("/setap",[](){ ssid=webserver.arg("ssid"); pass=webserver.arg("pass"); topic=webserver.arg("topic"); mqttserver=webserver.arg("mqttserver"); rjson("/config.json"); 
               doc["ssid"]=ssid; doc["pass"]=pass; doc["topic"]=topic; doc["mqttserver"][0]=mqttserver; wjson("/config.json"); WiFi.softAP(ssid,pass); homepage(); } ); //重新配置config

    webserver.on("/setsta",[](){ WiFi.begin(webserver.arg("ssid"),webserver.arg("pass")); WiFi.persistent(1); homepage(); } ); //连接路由器
    webserver.on("/scanap",[](){ String html=httphead(); int n=WiFi.scanNetworks(); for(int i=0;i>-100;i--){ for(int j=0;j<n;j++){if(WiFi.RSSI(j)==i){ html+="<form action=/setsta method=post>";
                                html+="<input name=ssid size=24 value="+WiFi.SSID(j)+">"+WiFi.RSSI(j)+"<br><input name=pass size=15><input type=submit value=连接wifi></form><p>"; } } }
                           html+="</body></html>"; webserver.send(200,"text/html",html); } ); //扫描路由器
                           
    webserver.on("/deloneok",[](){ SPIFFS.remove(webserver.arg("mingzi")); homepage(); } ); //确认删除单个文件
    webserver.on("/delone",[](){ String html=httphead()+"<form action=/deloneok method=post><input name=mingzi size=24 value="+webserver.arg("mingzi");
                                        html+="><p><input type=submit value=确认删除></form></body></html>"; webserver.send(200,"text/html",html); } ); //删除单个文件
    webserver.on("/delallok",[](){ SPIFFS.format(); homepage(); } ); //确认删除所有文件
    webserver.on("/delall",[](){ String html=httphead()+"<form action=/delallok method=post><input type=submit value=全部删除></form></body></html>"; 
                                        webserver.send(200,"text/html",html); } ); //删除所有文件
    }
void loop1(){ if(!digitalRead(0)){ rjson("/config.json"); doc["pass"]=""; doc["ssid"]=espid; wjson("/config.json"); again(); } //监听板载按钮重置ssid和pass
     digitalWrite(2,!leddark); webserver.handleClient(); digitalWrite(2,leddark); dnsserver.processNextRequest();  //监听webserver和dnsserver
     if(!mqttclient.connected())connectmqtt(); mqttclient.loop(); }  //监听mqtt

#endif


    
