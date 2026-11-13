#include "hal.h"
#include "drawing.h"
#include <stdio.h>
#ifndef WAVE_URL
#define WAVE_URL "http://127.0.0.1:8765/"
#endif
void wavesim_tick(const WaveHAL *h){char body[256],s[40];int code=h->http_get(WAVE_URL,body,sizeof body);uint8_t f[5000];memset(f,255,5000);text(f,15,22,"NETWORK",3);text(f,16,70,code>0?"ONLINE":"OFFLINE",3);snprintf(s,sizeof s,"HTTP %d",code);text(f,16,110,s,2);h->log(s);h->display(f,0);}
