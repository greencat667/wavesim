#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// Stable C boundary. Application code uses the same callbacks on ESP-IDF and macOS.
typedef struct {
 int (*i2c)(int address,const uint8_t *tx,int nt,uint8_t *rx,int nr);
 int (*display)(const uint8_t *frame,int partial);
 void (*log)(const char *text);
 int (*gpio_read)(int pin);
 void (*gpio_write)(int pin,int value);
 int (*http_get)(const char *url,char *body,int capacity);
 int (*sd_write)(const char *path,const char *text);
 int (*sd_read)(const char *path,char *text,int capacity);
} WaveHAL;
typedef void (*WaveTick)(const WaveHAL *);
#ifdef __cplusplus
}
#endif
