#include "hal.h"
#include "drawing.h"
#include "../third_party/json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <dlfcn.h>
#include <ctime>
#include <cmath>
#include <array>
#include <unistd.h>
#include <sys/wait.h>
using J=nlohmann::json; namespace fs=std::filesystem;
struct Board {
 double temp=21.3,humidity=68,epoch=1897037100,rate=1,elapsed=0,noise=0,ramp=0,battery=84;
 bool powered=true,sleeping=false,realistic=false,displaySleep=false,inserted=true,readonly=false;
 std::string network="ONLINE",fault="",sd="",project="";int rotation=0,frames=0;double busyUntil=0;
 std::array<uint8_t,5000> frame,previous,pending;bool pendingRefresh=false;
 J fixtures=J::array();void (*configure)(const char*)=nullptr;
 J rpc=nullptr;J events=J::array(),record=J::array();bool recording=false;int pins[49]={};
 void *lib=nullptr;WaveTick tick=nullptr;uint16_t sensorCommand=0;uint32_t seed=1;
 Board(){frame.fill(255);previous=frame;pins[0]=pins[18]=1;}
 void event(std::string bus,std::string message){events.push_back({{"time",elapsed},{"bus",bus},{"message",message}});if(events.size()>1000)events.erase(events.begin());}
} b;
static uint8_t enc(int n){return ((n/10)<<4)|(n%10);}
static int i2c(int a,const uint8_t *tx,int nt,uint8_t *rx,int nr){
 b.event("I2C",std::string(nr?"READ ":"WRITE ")+(a==0x70?"0x70":"0x51")+" tx="+std::to_string(nt)+" rx="+std::to_string(nr));
 if(!b.powered||b.fault=="i2c_timeout"||(a==0x70&&b.fault=="sensor_missing"))return -1;
 if(a==0x70){if(nt==2)b.sensorCommand=(tx[0]<<8)|tx[1];if(nr==0)return 0;if(nr!=6||(b.sensorCommand!=0x7866&&b.sensorCommand!=0x7ca2))return -2;
 b.seed=b.seed*1664525+1013904223;double n=(b.seed/4294967296.0-.5)*2*b.noise;
 int t=std::clamp((int)std::lround((b.temp+n+45)*65536/175),0,65535),h=std::clamp((int)std::lround(b.humidity*65536/100),0,65535);
 rx[0]=t>>8;rx[1]=t;rx[2]=crc8(rx,2);rx[3]=h>>8;rx[4]=h;rx[5]=crc8(rx+3,2);b.event("SHTC3","MEASURE "+std::to_string(b.temp));return 0;}
 if(a==0x51&&nt==1&&tx[0]==4&&nr==7){if(b.fault=="rtc_invalid")return -1;time_t now=(time_t)b.epoch;tm v;gmtime_r(&now,&v);rx[0]=enc(v.tm_sec);rx[1]=enc(v.tm_min);rx[2]=enc(v.tm_hour);rx[3]=enc(v.tm_mday);rx[4]=v.tm_wday;rx[5]=enc(v.tm_mon+1);rx[6]=enc((v.tm_year+1900)%100);b.event("RTC","READ "+std::to_string(now));return 0;}return -2;
}
static int display(const uint8_t *f,int partial){if(!b.powered||b.displaySleep||b.fault=="spi_failure"||b.fault=="display_busy"){b.event("DISPLAY","REFRESH rejected: sleeping or fault");return -1;}if(b.pendingRefresh){std::copy(f,f+5000,b.pending.begin());b.event("DISPLAY","REFRESH coalesced while BUSY");return 0;}
 b.event("SPI","WRITE 5000 bytes CS=11 DC=10");std::copy(f,f+5000,b.pending.begin());b.event("DISPLAY",partial?"PARTIAL_REFRESH":"FULL_REFRESH");b.event("SERIAL",partial?"EPAPER PARTIAL_REFRESH":"EPAPER FULL_REFRESH");
 if(b.realistic){b.pendingRefresh=true;b.busyUntil=b.elapsed+(partial?.3:2);b.pins[8]=1;}else{b.previous=b.frame;b.frame=b.pending;b.frames++;}return 0;}
static void logline(const char *s){b.event("SERIAL",s);}
static int gpioRead(int p){return p>=0&&p<49?b.pins[p]:-1;}
static void gpioWrite(int p,int v){if(p>=0&&p<49){b.pins[p]=!!v;b.event("GPIO",std::to_string(p)+(v?" HIGH":" LOW"));}}
static int http(const char *url,char *body,int cap){b.event("NETWORK",std::string("HTTP GET ")+url+" "+b.network);if(b.network=="OFFLINE"||b.network=="DNS FAILURE")return -1;
 std::string u=url;if(u.rfind("http://",0)!=0&&u.rfind("https://",0)!=0)return -2;
 int fd[2];if(pipe(fd))return -3;pid_t child=fork();if(child==0){dup2(fd[1],1);close(fd[0]);close(fd[1]);execl("/usr/bin/curl","curl","-sS","--max-time","3","--max-filesize","1048576","-w","\n%{http_code}","--",url,(char*)nullptr);_exit(127);}close(fd[1]);std::string result;char chunk[4096];ssize_t n;while((n=read(fd[0],chunk,sizeof chunk))>0){if(result.size()<1048576)result.append(chunk,n);}close(fd[0]);int status;waitpid(child,&status,0);if(status!=0){b.event("NETWORK","request failed");return -1;}auto pos=result.rfind('\n');int code=pos==std::string::npos?0:atoi(result.c_str()+pos+1);snprintf(body,cap,"%s",result.substr(0,pos).c_str());b.event("NETWORK","HTTP response "+std::to_string(code));return code;}
static fs::path sdpath(const char *s){fs::path rel(s);if(rel.is_absolute())throw std::runtime_error("SD path must be relative");auto root=fs::weakly_canonical(b.sd),p=fs::weakly_canonical(root/rel);auto r=p.lexically_relative(root);if(r.empty()||*r.begin()=="..")throw std::runtime_error("SD path escapes card");return p;}
static int sdwrite(const char *path,const char *s){try{if(!b.inserted||b.readonly||b.fault=="sd_full"||b.sd.empty())return -1;auto p=sdpath(path);std::ofstream o(p);o<<s;b.event("SD",std::string("WRITE ")+path);return o?0:-1;}catch(...){return -1;}}
static int sdread(const char *path,char *s,int cap){try{if(!b.inserted||b.sd.empty())return -1;std::ifstream f(sdpath(path));if(!f)return -1;f.read(s,cap-1);s[f.gcount()]=0;b.event("SD",std::string("READ ")+path);return f.gcount();}catch(...){return -1;}}
static WaveHAL hal={i2c,display,logline,gpioRead,gpioWrite,http,sdwrite,sdread};
static void load(const std::string &p){if(b.lib){dlclose(b.lib);b.lib=nullptr;b.tick=nullptr;}b.configure=nullptr;b.lib=dlopen(p.c_str(),RTLD_NOW|RTLD_LOCAL);if(!b.lib)throw std::runtime_error(dlerror());b.tick=(WaveTick)dlsym(b.lib,"wavesim_tick");if(!b.tick)throw std::runtime_error("missing wavesim_tick(const WaveHAL*)");b.configure=(void(*)(const char*))dlsym(b.lib,"wavesim_config");b.project=p;b.event("SERIAL","Boot: native compiled firmware loaded "+p);}
static void run(){if(b.tick&&b.powered&&!b.sleeping){if(b.configure){auto config=J{{"elapsed",b.elapsed},{"epoch",b.epoch},{"network",b.network},{"fixtures",b.fixtures}}.dump();b.configure(config.c_str());}b.tick(&hal);}}
static J state(){std::string hex;const char *digits="0123456789abcdef";for(uint8_t c:b.frame){hex+=digits[c>>4];hex+=digits[c&15];}std::string prev;for(uint8_t c:b.previous){prev+=digits[c>>4];prev+=digits[c&15];}return {{"rpc",b.rpc},{"temperature",b.temp},{"humidity",b.humidity},{"epoch",b.epoch},{"rate",b.rate},{"elapsed",b.elapsed},{"powered",b.powered},{"sleeping",b.sleeping},{"network",b.network},{"frame",hex},{"previous",prev},{"frames",b.frames},{"busy",b.pendingRefresh},{"rotation",b.rotation},{"events",b.events},{"project",b.project},{"fault",b.fault},{"gpio",b.pins},{"battery",b.battery},{"record",b.record},{"status",{{"CPU",b.configure?"SIMULATED — Arduino application source; not Xtensa":"SIMULATED — native C source; not Xtensa"},{"ePaper","SIMULATED — framebuffer/API"},{"SHTC3","SIMULATED — command, conversion, CRC"},{"RTC","SIMULATED — BCD reads, UTC"},{"GPIO","SIMULATED — digital"},{"SD",b.configure?"SIMULATED — LittleFS memory; no SD adapter":"BRIDGED — directory; no FAT"},{"Network",b.configure?"SIMULATED — explicit HTTP test fixtures only":"BRIDGED — HTTP/HTTPS via curl"},{"Power",b.configure?"SIMULATED — cooperative sleep; retained globals":"SIMULATED — lifecycle"},{"Audio","UNSUPPORTED"},{"Bluetooth","UNSUPPORTED"},{"Debugger","UNSUPPORTED — use LLDB externally"}}}};}
static void advance(double s){if(!std::isfinite(s)||s<0||s>1e12)throw std::runtime_error("invalid time increment");b.elapsed+=s;b.epoch+=s*b.rate;b.temp=std::clamp(b.temp+s*b.ramp,-20.,60.);if(b.pendingRefresh&&b.elapsed>=b.busyUntil){b.previous=b.frame;b.frame=b.pending;b.pendingRefresh=false;b.pins[8]=0;b.frames++;b.event("DISPLAY","BUSY released");}}
static void command(const J &j){std::string op=j.value("op","state");if(b.recording&&op!="state"&&op!="record"&&op!="io")b.record.push_back(j);
 if(op=="set"){if(j.contains("fixtures")){if(!j["fixtures"].is_array())throw std::runtime_error("fixtures must be an array");b.fixtures=j["fixtures"];}b.temp=std::clamp(j.value("temperature",b.temp),-20.,60.);b.humidity=std::clamp(j.value("humidity",b.humidity),0.,100.);b.epoch=j.value("epoch",b.epoch);b.rate=std::clamp(j.value("rate",b.rate),0.,864000.);b.noise=std::clamp(j.value("noise",b.noise),0.,10.);b.ramp=j.value("ramp",b.ramp);b.realistic=j.value("realistic",b.realistic);b.rotation=j.value("rotation",b.rotation);b.fault=j.value("fault",b.fault);b.inserted=j.value("inserted",b.inserted);b.readonly=j.value("readonly",b.readonly);b.sd=j.value("sd",b.sd);b.battery=std::clamp(j.value("battery",b.battery),0.,100.);if(j.contains("network")){b.network=j["network"];b.event("NETWORK","STATE "+b.network);}}
 else if(op=="io"){
 std::string kind=j.at("kind");
 if(kind=="i2c"){std::vector<uint8_t> tx=j.value("tx",std::vector<uint8_t>{});int nr=j.value("nr",0);if(nr<0||nr>4096)throw std::runtime_error("invalid I2C read size");std::vector<uint8_t> rx(nr);int rc=i2c(j.at("address"),tx.data(),tx.size(),rx.data(),nr);b.rpc={{"rc",rc},{"rx",rx}};}
 else if(kind=="display"){std::string hex=j.at("frame");if(hex.size()!=10000)throw std::runtime_error("expected 5000-byte frame");std::array<uint8_t,5000> f;for(int i=0;i<5000;i++){auto part=hex.substr(i*2,2);if(part.find_first_not_of("0123456789abcdefABCDEF")!=std::string::npos)throw std::runtime_error("invalid hex");f[i]=std::stoi(part,nullptr,16);}b.rpc={{"rc",display(f.data(),j.value("partial",0))}};}
 else if(kind=="log"){logline(j.at("text").get<std::string>().c_str());b.rpc={{"rc",0}};}
 else if(kind=="gpio_read")b.rpc={{"rc",gpioRead(j.at("pin"))}};
 else if(kind=="gpio_write"){gpioWrite(j.at("pin"),j.at("value"));b.rpc={{"rc",0}};}
 else throw std::runtime_error("unsupported bridge operation");
 }
 else if(op=="load")load(j.at("path"));else if(op=="run")run();
 else if(op=="advance"){advance(j.value("seconds",1.));if(j.value("run",true))run();}
 else if(op=="jump"){b.epoch+=j.value("seconds",60.);run();}
 else if(op=="power"){b.powered=j.at("on");b.sleeping=false;b.pendingRefresh=false;b.pins[8]=0;b.event("POWER",b.powered?"ON":"OFF; ePaper retained");if(b.powered)run();}
 else if(op=="sleep"){b.sleeping=j.value("on",true);b.event("POWER",b.sleeping?"DEEP_SLEEP":"WAKE");if(!b.sleeping)run();}
 else if(op=="reset"){b.sleeping=false;b.pendingRefresh=false;b.displaySleep=false;b.pins[8]=0;b.event("POWER","RESET");if(!b.project.empty())load(b.project);run();}
 else if(op=="button"){int p=j.value("pin",0);if(p!=0&&p!=18)throw std::runtime_error("not a board button");gpioWrite(p,j.value("down",true)?0:1);b.event("BUTTON",std::to_string(p)+(j.value("down",true)?" PRESS":" RELEASE"));run();}
 else if(op=="gpio")gpioWrite(j.at("pin"),j.at("value"));
 else if(op=="display_sleep")b.displaySleep=j.value("on",true);
 else if(op=="clear"){b.events=J::array();}
 else if(op=="serial")b.event("SERIAL","INPUT (no firmware RX adapter): "+j.value("text",""));
 else if(op=="record"){b.recording=j.value("on",true);if(b.recording)b.record=J::array();}
 else if(op=="replay"){b.recording=false;for(auto &step:j.at("steps"))command(step);}
 else if(op=="sd_write"){if(sdwrite(j.at("path").get<std::string>().c_str(),j.at("text").get<std::string>().c_str()))throw std::runtime_error("SD write failed");}
 else if(op=="sd_read"){char s[4096];if(sdread(j.at("path").get<std::string>().c_str(),s,sizeof s)<0)throw std::runtime_error("SD read failed");b.event("SD",s);}
 else if(op!="state")throw std::runtime_error("unknown operation: "+op);
}
int main(int argc,char **argv){if(argc>1){try{load(argv[1]);}catch(std::exception &e){std::cerr<<e.what()<<"\n";return 1;}}std::string line;while(std::getline(std::cin,line)){try{auto j=J::parse(line);command(j);std::cout<<J{{"ok",true},{"state",state()}}.dump()<<std::endl;}catch(std::exception &e){std::cout<<J{{"ok",false},{"error",e.what()},{"state",state()}}.dump()<<std::endl;}}}
