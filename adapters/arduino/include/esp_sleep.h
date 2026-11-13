#pragma once
#include "Arduino.h"
using esp_sleep_wakeup_cause_t=int;
constexpr int ESP_SLEEP_WAKEUP_UNDEFINED=0,ESP_SLEEP_WAKEUP_TIMER=1,ESP_SLEEP_WAKEUP_EXT1=2,ESP_SLEEP_WAKEUP_ALL=3,ESP_EXT1_WAKEUP_ANY_LOW=0;
inline int esp_sleep_get_wakeup_cause(){return wave::cause;}
inline uint64_t esp_sleep_get_ext1_wakeup_status(){return wave::wakeMask;}
inline void esp_sleep_disable_wakeup_source(int){wave::timer=0;}
inline void esp_sleep_enable_timer_wakeup(uint64_t t){wave::timer=t/1000;}
inline void esp_sleep_enable_ext1_wakeup_io(uint64_t,int){}
inline void esp_deep_sleep_start(){wave::asleep=true;wave::log("SIMULATED deep sleep: retained application state, not CPU reset");throw wave::Sleep{};}
