#include "core/abstraction_layer.h"

#ifdef __WIN32
#include <Windows.h>

std::wstring string2wstring(const char * str){
    DWORD size = MultiByteToWideChar(CP_UTF8, 0, str, -1, 0, 0);
    std::wstring res;
    res.reserve(size);
    res.resize(size);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, res.data(), size);
    return res;
}
std::wstring string2wstring(const std::string &str){
    return string2wstring(str.data());
}

std::string wstring2string(const std::wstring &wide){
    DWORD size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, 0, 0, 0, 0);
    std::string res;
    res.reserve(size);
    res.resize(size);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, res.data(), size, 0, 0);
    return res;
}
std::string getShortPathIfNeeded(wchar_t *wide){ //Needed
    DWORD size = GetShortPathNameW(wide, nullptr, 0);
    std::wstring wres;
    wres.reserve(size);
    wres.resize(size);
    GetShortPathNameW(wide, wres.data(), size);
    return wstring2string(wres);
}
wchar_t* ourPathGetenv(char* varName){
    std::string varNameS{varName};
  auto wstr = string2wstring(varNameS);
  return _wgetenv(wstr.data());
}
#else
std::string getShortPathIfNeeded(char *wide){ //Not needed
    return wide;
}
char* ourPathGetenv(char* varName){
  return std::getenv(varName);
}
#endif
