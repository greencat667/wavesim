#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void){printf("WAVESIM ESP32-S3 BINARY EXECUTION OK\n");while(1){printf("WAVESIM QEMU HEARTBEAT\n");vTaskDelay(pdMS_TO_TICKS(1000));}}
