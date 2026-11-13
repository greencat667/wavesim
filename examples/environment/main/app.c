#include "hal.h"
#include "drawing.h"
#include <stdio.h>
void wavesim_tick(const WaveHAL *h){
 uint8_t f[5000];memset(f,255,sizeof f);uint8_t cmd[]={0x35,0x17},raw[6],reg=4,rtc[7];
 h->i2c(0x70,cmd,2,0,0);cmd[0]=0x78;cmd[1]=0x66;
 int err=h->i2c(0x70,cmd,2,raw,6);int re=h->i2c(0x51,&reg,1,rtc,7);char s[100];if(!re&&(rtc[0]&0x80))re=-1;
 text(f,16,12,"ENVIRONMENT",2);
 if(!re){snprintf(s,sizeof s,"%02d:%02d",bcd(rtc[2]&63),bcd(rtc[1]&127));text(f,24,43,s,5);snprintf(s,sizeof s,"20%02d-%02d-%02d",bcd(rtc[6]),bcd(rtc[5]&31),bcd(rtc[3]&63));text(f,40,88,s,2);}else text(f,16,44,"RTC INVALID",2);
 if(!err&&crc8(raw,2)==raw[2]&&crc8(raw+3,2)==raw[5]){double t=-45+175.0*((raw[0]<<8)|raw[1])/65536.0,r=100.0*((raw[3]<<8)|raw[4])/65536.0;
 snprintf(s,sizeof s,"%.1f C",t);text(f,20,116,s,4); // degree glyph
 for(int x=119;x<124;x++){pixel(f,x,114);pixel(f,x,119);}for(int y=114;y<120;y++){pixel(f,119,y);pixel(f,124,y);}
 snprintf(s,sizeof s,"%.0f%% RH",r);text(f,30,160,s,3);snprintf(s,sizeof s,"SHTC3 MEASURE temperature=%.1f humidity=%.0f",t,r);h->log(s);
 }else{h->log("ERROR SHTC3 unavailable / CRC");text(f,16,125,"SENSOR ERROR",2);}
 if(h->gpio_read(0)==0)text(f,160,188,"BOOT",1);
 h->display(f,0);
}
