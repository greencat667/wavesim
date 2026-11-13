#pragma once
#include "Arduino.h"
constexpr int WIFI_OFF=0,WIFI_STA=1,WL_CONNECTED=3;
class IPAddress{public:String toString(){return "192.0.2.1 (simulated)";}};
class WiFiClass{bool started=false;public:void mode(int){}void begin(const char*,const char*){started=true;wave::log("SIMULATED Wi-Fi status; no radio or credentials used");}int status(){return started&&wave::network=="ONLINE"?WL_CONNECTED:0;}void disconnect(bool=false){started=false;}IPAddress localIP(){return {};}};inline WiFiClass WiFi;
