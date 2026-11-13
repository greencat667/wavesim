#!/usr/bin/env python3
"""Build original sketch/application sources against explicitly simulated peripherals."""
import argparse,pathlib,shutil,subprocess,tempfile,json
ROOT=pathlib.Path(__file__).resolve().parents[1]
def sketch_path(p):
 p=pathlib.Path(p).resolve()
 if p.is_file() and p.suffix=='.ino':return p
 if p.is_dir():
  items=list(p.glob('*.ino'))
  if not items:items=list(p.glob('*/*.ino'))
  if len(items)==1:return items[0]
 raise ValueError('Choose one .ino file or its sketch folder (not the working folder containing all projects).')
def build(project,output):
 ino=sketch_path(project);source=ino.parent
 libs=ROOT/'third_party/arduino'
 if not libs.exists():libs=pathlib.Path.home()/'Documents/Arduino/libraries'
 gfx=libs/'Adafruit_GFX_Library';aj=libs/'ArduinoJson/src'
 if not (gfx/'Adafruit_GFX.cpp').exists():raise RuntimeError('Adafruit GFX library is required in Documents/Arduino/libraries.')
 with tempfile.TemporaryDirectory(prefix='wavesim-arduino-') as tmp:
  stage=pathlib.Path(tmp)
  for p in source.iterdir():
   if p.is_file() and p.suffix in ('.ino','.h','.cpp') and p.name!='secrets.h':shutil.copy2(p,stage/p.name)
  (stage/'secrets.h').write_text('#pragma once\n#define MANUAL_POSTCODE ""\n#define WIFI_SSID "WaveSim-test"\n#define WIFI_PASSWORD "unused"\n#define RELAY_BASE_URL "http://fixture.invalid"\n')
  for name in ['display/epaper_driver_bsp.h','power/board_power_bsp.h','i2c/i2c_bsp.h','i2c/shtc3_sensor.h']:
   dest=stage/'src'/name;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text('#pragma once\n#include "peripherals.h"\n')
  (stage/'src/sound.h').write_text('#pragma once\n#include "Arduino.h"\ninline void sound_init(){wave::log("UNSUPPORTED audio playback");}\ninline void play_startup_chime(){wave::log("UNSUPPORTED startup chime");}\n')
  (stage/'src/audio_capture.h').write_text('#pragma once\n#include "Arduino.h"\ninline void audio_capture_init(){wave::log("SIMULATED microphone: fixed synthetic RMS 20");}\ninline float audio_capture_get_rms(unsigned long ms){delay(ms);return 20;}\n')
  extra=[]
  for name in ['display_gfx.h','display_gfx.cpp']:
   if (source/'src'/name).exists():shutil.copy2(source/'src'/name,stage/'src'/name)
  if (stage/'src/display_gfx.cpp').exists():extra.append(stage/'src/display_gfx.cpp')
  cleanup=''
  text=ino.read_text()
  for name in ['canvas','gfx','driver','shtc3']:
   if f'*{name} = nullptr' in text or f'* {name} = nullptr' in text:cleanup+=f'delete {name};{name}=nullptr;'
  wrapper='''#include "Arduino.h"
#include "esp_sleep.h"
#include "HTTPClient.h"
#include "SKETCH"
extern "C" void wavesim_config(const char *config){auto j=nlohmann::json::parse(config);double t=j.value("elapsed",0.0);wave::ms+=(uint64_t)(std::max(0.0,t-wave::elapsed)*1000);wave::elapsed=t;wave::epoch=j.value("epoch",1897037100.0);wave::network=j.value("network",std::string("OFFLINE"));wave::fixtures=j.value("fixtures",nlohmann::json::array());}
extern "C" void wavesim_tick(const WaveHAL*h){wave::hal=h;static bool started=false;static uint64_t sleepAt=0;static int last0=1,last18=1;int p0=h->gpio_read(0),p18=h->gpio_read(18);uint64_t edge=(p0==0&&last0==1?1ULL:0ULL)|(p18==0&&last18==1?1ULL<<18:0ULL);last0=p0;last18=p18;
if(wave::asleep){if(edge){wave::cause=ESP_SLEEP_WAKEUP_EXT1;wave::wakeMask=edge;}else if(wave::timer&&wave::ms-sleepAt>=wave::timer){wave::cause=ESP_SLEEP_WAKEUP_TIMER;wave::wakeMask=0;}else return;wave::asleep=false;started=false;CLEANUP}
try{if(!started){started=true;wave::log("ARDUINO SOURCE SIMULATION: original application, substituted peripherals");setup();}loop();}catch(const wave::Sleep&){sleepAt=wave::ms;}Serial.flush();}
'''.replace('SKETCH',ino.name).replace('CLEANUP',cleanup)
  (stage/'wrapper.cpp').write_text(wrapper)
  args=['/usr/bin/clang++','-std=c++17','-dynamiclib','-g','-O0','-DARDUINO=10819','-DARDUINOJSON_ENABLE_ARDUINO_STRING=1','-DARDUINOJSON_ENABLE_ARDUINO_STREAM=0','-DARDUINOJSON_ENABLE_ARDUINO_PRINT=0','-DARDUINOJSON_ENABLE_PROGMEM=0','-include',str(ROOT/'adapters/arduino/include/Arduino.h')]
  for p in [ROOT/'adapters/arduino/include',ROOT/'core',ROOT/'third_party',stage,gfx,aj]:args+=['-I',str(p)]
  args += [str(stage/'wrapper.cpp')]+[str(p) for p in stage.glob('*.cpp') if p.name!='wrapper.cpp']+[str(p) for p in extra]+[str(gfx/'Adafruit_GFX.cpp'),'-o',str(pathlib.Path(output).resolve())]
  subprocess.run(args,check=True)
 return ino
if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('project');parser.add_argument('output');a=parser.parse_args()
 try:print('Built Arduino source simulation:',build(a.project,a.output))
 except (ValueError,RuntimeError,subprocess.CalledProcessError) as e:parser.exit(1,str(e)+'\n')
