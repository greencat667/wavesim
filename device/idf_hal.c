#include "hal.h"
#include "waveshare_lut.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_crt_bundle.h"
#include "sdkconfig.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
static spi_device_handle_t spi;
static int transfer(int dc,const uint8_t *data,int count){gpio_set_level(10,dc);spi_transaction_t t={.length=count*8,.tx_buffer=data};return spi_device_polling_transmit(spi,&t);}
static void cmd(uint8_t c){ESP_ERROR_CHECK(transfer(0,&c,1));}
static void dat(uint8_t d){ESP_ERROR_CHECK(transfer(1,&d,1));}
static int wait_busy(void){int64_t deadline=esp_timer_get_time()+10000000;while(gpio_get_level(8)){if(esp_timer_get_time()>deadline){ESP_LOGE("epaper","BUSY timeout");return -1;}vTaskDelay(pdMS_TO_TICKS(5));}return 0;}
static int bus(int a,const uint8_t *tx,int nt,uint8_t *rx,int nr){
 esp_err_t e;if(a==0x70){e=nt?i2c_master_write_to_device(I2C_NUM_0,a,tx,nt,pdMS_TO_TICKS(100)):ESP_OK;if(e)return -1;vTaskDelay(pdMS_TO_TICKS(nr?20:2));return nr?i2c_master_read_from_device(I2C_NUM_0,a,rx,nr,pdMS_TO_TICKS(100)):0;}
 return i2c_master_write_read_device(I2C_NUM_0,a,tx,nt,rx,nr,pdMS_TO_TICKS(100));
}
static int refresh(const uint8_t *f,int partial){if(partial){ESP_LOGE("epaper","Physical partial refresh not implemented");return -1;}if(wait_busy())return -1;cmd(0x4e);dat(0);cmd(0x4f);dat(199);dat(0);cmd(0x24);if(transfer(1,f,5000))return -1;cmd(0x22);dat(0xc7);cmd(0x20);int e=wait_busy();ESP_LOGI("epaper","FULL_REFRESH 5000 bytes result=%d",e);return e;}
static void serial(const char *s){ESP_LOGI("app","%s",s);}
static int get(int p){return gpio_get_level(p);}static void put(int p,int v){gpio_set_level(p,v);}
static bool wifi_started=false;
static void wifi_event(void *arg,esp_event_base_t base,int32_t id,void *data){if(base==WIFI_EVENT&&(id==WIFI_EVENT_STA_START||id==WIFI_EVENT_STA_DISCONNECTED))esp_wifi_connect();}
static void ensure_wifi(void){if(wifi_started||strlen(CONFIG_WAVE_WIFI_SSID)==0)return;wifi_started=true;esp_err_t e=nvs_flash_init();if(e!=ESP_OK){ESP_LOGE("wifi","NVS init failed: %d",e);return;}ESP_ERROR_CHECK(esp_netif_init());ESP_ERROR_CHECK(esp_event_loop_create_default());esp_netif_create_default_wifi_sta();wifi_init_config_t conf=WIFI_INIT_CONFIG_DEFAULT();ESP_ERROR_CHECK(esp_wifi_init(&conf));ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event,NULL));wifi_config_t wc={0};snprintf((char*)wc.sta.ssid,sizeof wc.sta.ssid,"%s",CONFIG_WAVE_WIFI_SSID);snprintf((char*)wc.sta.password,sizeof wc.sta.password,"%s",CONFIG_WAVE_WIFI_PASSWORD);ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&wc));ESP_ERROR_CHECK(esp_wifi_start());vTaskDelay(pdMS_TO_TICKS(3000));}
static int http_get(const char *url,char *body,int cap){ensure_wifi();esp_http_client_config_t conf={.url=url,.timeout_ms=3000,.crt_bundle_attach=esp_crt_bundle_attach};esp_http_client_handle_t c=esp_http_client_init(&conf);if(!c)return -1;int code=-1;if(esp_http_client_open(c,0)==ESP_OK){esp_http_client_fetch_headers(c);code=esp_http_client_get_status_code(c);int n=esp_http_client_read(c,body,cap-1);if(n>=0)body[n]=0;}esp_http_client_cleanup(c);return code;}
static int sd_write(const char*p,const char*s){ESP_LOGW("sd","SD mounting adapter not implemented");return -1;}
static int sd_read(const char*p,char*s,int n){return -1;}
extern void wavesim_tick(const WaveHAL*);
void app_main(void){
 gpio_config_t out={.pin_bit_mask=(1ULL<<6)|(1ULL<<9)|(1ULL<<10)|(1ULL<<17)|(1ULL<<42),.mode=GPIO_MODE_OUTPUT};ESP_ERROR_CHECK(gpio_config(&out));gpio_set_level(17,1);gpio_set_level(6,1);gpio_set_level(42,0);
 gpio_config_t in={.pin_bit_mask=(1ULL<<0)|(1ULL<<18)|(1ULL<<8),.mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_ENABLE};ESP_ERROR_CHECK(gpio_config(&in));
 i2c_config_t ic={.mode=I2C_MODE_MASTER,.sda_io_num=47,.scl_io_num=48,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master.clk_speed=100000};ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0,&ic));ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0,ic.mode,0,0,0));
 spi_bus_config_t sc={.mosi_io_num=13,.miso_io_num=-1,.sclk_io_num=12,.quadwp_io_num=-1,.quadhd_io_num=-1,.max_transfer_sz=5000};ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST,&sc,SPI_DMA_CH_AUTO));spi_device_interface_config_t dc={.clock_speed_hz=4000000,.mode=0,.spics_io_num=11,.queue_size=1};ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST,&dc,&spi));
 gpio_set_level(9,1);vTaskDelay(pdMS_TO_TICKS(50));gpio_set_level(9,0);vTaskDelay(pdMS_TO_TICKS(20));gpio_set_level(9,1);vTaskDelay(pdMS_TO_TICKS(50));wait_busy();cmd(0x12);wait_busy();cmd(1);dat(0xc7);dat(0);dat(1);cmd(0x11);dat(1);cmd(0x44);dat(0);dat(24);cmd(0x45);dat(199);dat(0);dat(0);dat(0);cmd(0x3c);dat(1);cmd(0x18);dat(0x80);cmd(0x22);dat(0xb1);cmd(0x20);wait_busy();cmd(0x32);transfer(1,WF_Full_1IN54,153);wait_busy();cmd(0x3f);dat(WF_Full_1IN54[153]);cmd(3);dat(WF_Full_1IN54[154]);cmd(4);dat(WF_Full_1IN54[155]);dat(WF_Full_1IN54[156]);dat(WF_Full_1IN54[157]);cmd(0x2c);dat(WF_Full_1IN54[158]);
 WaveHAL hal={bus,refresh,serial,get,put,http_get,sd_write,sd_read};ESP_LOGI("WaveSim","Shared application starting on physical V2 HAL");while(1){wavesim_tick(&hal);vTaskDelay(pdMS_TO_TICKS(10000));}
}
