// QEMU UART RPC transport: CPU executes the unchanged application on Xtensa.
#include "hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static char reply[100];
static void line(void){int n=0;char c;while(n<sizeof(reply)-1){if(uart_read_bytes(UART_NUM_0,&c,1,pdMS_TO_TICKS(10000))!=1){strcpy(reply,"-1");return;}if(c=='\n')break;if(c!='\r')reply[n++]=c;}reply[n]=0;}
static void hex(const uint8_t *p,int n){for(int i=0;i<n;i++)printf("%02x",p[i]);}
static int bus(int a,const uint8_t *tx,int nt,uint8_t *rx,int nr){printf("@I %d %d ",a,nr);hex(tx,nt);printf("\n");fflush(stdout);line();int rc=atoi(reply);char *p=strchr(reply,' ');if(rc==0&&nr&&p){for(int i=0;i<nr;i++){unsigned int v=0;sscanf(p+1+i*2,"%2x",&v);rx[i]=v;}}return rc;}
static int display(const uint8_t *f,int partial){printf("@D %d ",partial);hex(f,5000);printf("\n");fflush(stdout);line();return atoi(reply);}
static void logline(const char *s){printf("@L %s\n",s);fflush(stdout);}
static int get(int p){printf("@G %d\n",p);fflush(stdout);line();return atoi(reply);}
static void put(int p,int v){printf("@W %d %d\n",p,v);fflush(stdout);line();}
static int http(const char*u,char*b,int n){logline("QEMU HTTP adapter unsupported");return -1;}
static int sw(const char*p,const char*s){return -1;}static int sr(const char*p,char*s,int n){return -1;}
extern void wavesim_tick(const WaveHAL*);
void app_main(void){ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,2048,0,0,NULL,0));WaveHAL h={bus,display,logline,get,put,http,sw,sr};printf("@READY\n");fflush(stdout);while(1){line();if(reply[0]=='T'){wavesim_tick(&h);printf("@DONE\n");fflush(stdout);}vTaskDelay(pdMS_TO_TICKS(1));}}
