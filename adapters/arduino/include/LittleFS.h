#pragma once
#include "Arduino.h"
namespace wave{inline std::map<std::string,std::string> files;}
class File{std::string path;size_t pos=0;bool valid=false;public:File(){}File(String p,bool w):path(p),valid(w||wave::files.count(p)){if(w)wave::files[p].clear();}explicit operator bool()const{return valid;}int available(){return valid&&pos<wave::files[path].size();}String readStringUntil(char c){auto&s=wave::files[path];auto end=s.find(c,pos);if(end==std::string::npos)end=s.size();String out=s.substr(pos,end-pos);pos=std::min(end+1,s.size());return out;}void println(const String&s){if(valid)wave::files[path]+=s+"\n";}void close(){}};
class LittleFSClass{public:bool begin(bool=false){wave::log("SIMULATED LittleFS: in-memory, resets when project unloads");return true;}File open(String p,const char*mode){return File(p,mode[0]=='w');}};inline LittleFSClass LittleFS;
