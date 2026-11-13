#pragma once
#include "Arduino.h"
struct custom_lcd_spi_t{int cs,dc,rst,busy,mosi,scl,spi_host,buffer_len;};
constexpr int DRIVER_COLOR_WHITE=1,DRIVER_COLOR_BLACK=0;
class epaper_driver_display{uint8_t frame[5000];public:epaper_driver_display(int,int,custom_lcd_spi_t){EPD_Clear();}void EPD_Init(){}void EPD_Clear(){memset(frame,255,sizeof frame);}void EPD_DrawColorPixel(uint16_t x,uint16_t y,uint8_t c){if(x>=200||y>=200)return;uint8_t bit=128>>(x%8);if(c)frame[y*25+x/8]|=bit;else frame[y*25+x/8]&=~bit;}void EPD_Display(){wave::hal->display(frame,0);}void EPD_Sleep(){} };
class board_power_bsp_t{int e,a,v;public:board_power_bsp_t(int ep,int au,int vb):e(ep),a(au),v(vb){}void POWEER_EPD_ON(){digitalWrite(e,1);}void POWEER_EPD_OFF(){digitalWrite(e,0);}void POWEER_Audio_ON(){digitalWrite(a,1);}void POWEER_Audio_OFF(){digitalWrite(a,0);}void VBAT_POWER_ON(){digitalWrite(v,1);}};
inline void i2c_master_Init(){}
struct shtc3_data_t{float Temp,RH;};
class Shtc3Sensor{public:shtc3_data_t readTempHumi(){uint8_t wake[]={0x35,0x17},cmd[]={0x78,0x66},rx[6]={};wave::hal->i2c(0x70,wake,2,nullptr,0);if(wave::hal->i2c(0x70,cmd,2,rx,6))return {NAN,NAN};return {float(-45+175.0*((rx[0]<<8)|rx[1])/65535),float(100.0*((rx[3]<<8)|rx[4])/65535)};}};
