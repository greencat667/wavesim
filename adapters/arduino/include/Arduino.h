#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstdarg>
#include <ctime>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <limits.h>
#include "hal.h"
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define pgm_read_word(p) (*(const uint16_t*)(p))
#define pgm_read_dword(p) (*(const unsigned long*)(p))
class __FlashStringHelper;
#define PROGMEM
#define RTC_DATA_ATTR
#define F(x) x
#define HIGH 1
#define LOW 0
#define INPUT_PULLUP 2
#define OUTPUT 3
#define SPI2_HOST 2
#define I2C_NUM_0 0
#define PI 3.14159265358979323846
using std::min; using std::max;
using byte=uint8_t;using gpio_num_t=int;
class String:public std::string {
 using const_iterator=std::string::const_iterator;
public:
 using std::string::string;using std::string::operator=;
 String& operator=(const char*s){std::string::operator=(s?s:"");return *this;}
 String(const char*s):std::string(s?s:""){}
 String():std::string(){} String(const std::string&s):std::string(s){} String(char c):std::string(1,c){}
 String(int v):std::string(std::to_string(v)){} String(unsigned v):std::string(std::to_string(v)){} String(long v):std::string(std::to_string(v)){} String(unsigned long v):std::string(std::to_string(v)){}
 String(double v,int digits=2){char b[96];snprintf(b,sizeof b,"%.*f",digits,v);assign(b);}
 unsigned length()const{return size();}int indexOf(const String&s,unsigned from=0)const{auto i=find(s,from);return i==npos?-1:(int)i;} int indexOf(char c,unsigned from=0)const{auto i=find(c,from);return i==npos?-1:(int)i;}
 String substring(unsigned a,unsigned b=UINT_MAX)const{return a>=size()?String():String(substr(a,b==UINT_MAX?npos:b-a));}
 void trim(){auto a=find_first_not_of(" \t\r\n"),b=find_last_not_of(" \t\r\n");assign(a==npos?"":substr(a,b-a+1));}
 void toLowerCase(){for(char&c:*this)c=tolower((unsigned char)c);}void toUpperCase(){for(char&c:*this)c=toupper((unsigned char)c);}
 bool equalsIgnoreCase(String o)const{String s=*this;s.toLowerCase();o.toLowerCase();return s==o;}
 void replace(const String&a,const String&b){if(a.empty())return;size_t i=0;while((i=find(a,i))!=npos){std::string::replace(i,a.size(),b);i+=b.size();}}
 void toCharArray(char*b,unsigned n)const{if(n){strncpy(b,c_str(),n-1);b[n-1]=0;}}
 double toDouble()const{return strtod(c_str(),nullptr);}float toFloat()const{return toDouble();}long toInt()const{return strtol(c_str(),nullptr,10);}
 bool startsWith(const String&s)const{return rfind(s,0)==0;}bool endsWith(const String&s)const{return size()>=s.size()&&compare(size()-s.size(),s.size(),s)==0;}
 bool concat(const char*s){append(s);return true;}bool concat(const char*s,size_t n){append(s,n);return true;}bool concat(char c){push_back(c);return true;}
};
class Print{public:virtual ~Print()=default;virtual size_t write(uint8_t)=0;size_t write(const uint8_t*b,size_t n){for(size_t i=0;i<n;i++)write(b[i]);return n;}size_t write(const char*s){return write((const uint8_t*)s,strlen(s));}size_t print(const char*s){return write(s);}size_t print(const String&s){return write(s.c_str());}size_t print(char c){return write(c);}template<class T>size_t print(T v){return print(String(v));}size_t print(double v,int n){return print(String(v,n));}size_t println(){return write('\n');}template<class T>size_t println(T v){return print(v)+println();}size_t printf(const char*fmt,...){char b[4096];va_list a;va_start(a,fmt);vsnprintf(b,sizeof b,fmt,a);va_end(a);return write(b);} };
namespace wave {inline const WaveHAL*hal=nullptr;inline uint64_t ms=0, timer=0,wakeMask=0;inline double elapsed=0,epoch=1897037100;inline bool asleep=false;inline int cause=0;inline std::string network="OFFLINE";struct Sleep{};inline void log(const char*s){if(hal)hal->log(s);}}
class SerialClass:public Print{String line;public:using Print::write;void begin(int){}void flush(){if(!line.empty()){wave::log(line.c_str());line.clear();}}size_t write(uint8_t c)override{if(c=='\n')flush();else line+=char(c);return 1;}explicit operator bool()const{return true;}};inline SerialClass Serial;
inline unsigned long millis(){return wave::ms;}inline void delay(unsigned long ms){wave::ms+=ms;}inline void pinMode(int,int){}inline int digitalRead(int p){return wave::hal->gpio_read(p);}inline void digitalWrite(int p,int v){wave::hal->gpio_write(p,v);}
inline uint32_t esp_random(){static uint32_t v=1;v=v*1664525+1013904223;return v;}inline long random(long n){return n?esp_random()%n:0;}inline long random(long a,long b){return a+random(b-a);}inline void randomSeed(unsigned long){}
inline float radians(float x){return x*PI/180;}inline float degrees(float x){return x*180/PI;}template<class T>T constrain(T v,T a,T b){return std::clamp(v,a,b);}
inline void configTime(long,int,const char*,const char* =nullptr){wave::log("SIMULATED time: board UTC; no NTP request");}inline bool getLocalTime(tm*t,unsigned=5000){time_t e=wave::epoch;gmtime_r(&e,t);return true;}
constexpr int GPIO_NUM_0=0;
constexpr int GPIO_NUM_1=1;
constexpr int GPIO_NUM_2=2;
constexpr int GPIO_NUM_3=3;
constexpr int GPIO_NUM_4=4;
constexpr int GPIO_NUM_5=5;
constexpr int GPIO_NUM_6=6;
constexpr int GPIO_NUM_7=7;
constexpr int GPIO_NUM_8=8;
constexpr int GPIO_NUM_9=9;
constexpr int GPIO_NUM_10=10;
constexpr int GPIO_NUM_11=11;
constexpr int GPIO_NUM_12=12;
constexpr int GPIO_NUM_13=13;
constexpr int GPIO_NUM_14=14;
constexpr int GPIO_NUM_15=15;
constexpr int GPIO_NUM_16=16;
constexpr int GPIO_NUM_17=17;
constexpr int GPIO_NUM_18=18;
constexpr int GPIO_NUM_19=19;
constexpr int GPIO_NUM_20=20;
constexpr int GPIO_NUM_21=21;
constexpr int GPIO_NUM_22=22;
constexpr int GPIO_NUM_23=23;
constexpr int GPIO_NUM_24=24;
constexpr int GPIO_NUM_25=25;
constexpr int GPIO_NUM_26=26;
constexpr int GPIO_NUM_27=27;
constexpr int GPIO_NUM_28=28;
constexpr int GPIO_NUM_29=29;
constexpr int GPIO_NUM_30=30;
constexpr int GPIO_NUM_31=31;
constexpr int GPIO_NUM_32=32;
constexpr int GPIO_NUM_33=33;
constexpr int GPIO_NUM_34=34;
constexpr int GPIO_NUM_35=35;
constexpr int GPIO_NUM_36=36;
constexpr int GPIO_NUM_37=37;
constexpr int GPIO_NUM_38=38;
constexpr int GPIO_NUM_39=39;
constexpr int GPIO_NUM_40=40;
constexpr int GPIO_NUM_41=41;
constexpr int GPIO_NUM_42=42;
constexpr int GPIO_NUM_43=43;
constexpr int GPIO_NUM_44=44;
constexpr int GPIO_NUM_45=45;
constexpr int GPIO_NUM_46=46;
constexpr int GPIO_NUM_47=47;
constexpr int GPIO_NUM_48=48;

inline bool isDigit(char c){return c>='0'&&c<='9';}
