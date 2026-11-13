#include "hal.h"
#include "drawing.h"
void wavesim_tick(const WaveHAL *h){uint8_t f[5000];memset(f,255,5000);text(f,35,62,"Hello",4);text(f,17,108,"WaveSim",4);h->log("Hello WaveSim");h->display(f,0);}
