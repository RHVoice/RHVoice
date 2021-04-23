#pragma once
#ifndef RHVOICE_ABSTR_LAYER
#define RHVOICE_ABSTR_LAYER

#include "api.h"

#if defined(_WIN32) && defined(UNICODE) && UNICODE
    typedef wchar_t PathCharT;
#else
    typedef char PathCharT;

    #define TEXT(t) t
#endif

PathCharT* ourPathGetenv(char* varName);

#if defined(__cplusplus)
    #include <string>
    typedef std::basic_string<PathCharT> PathT;
    typedef std::basic_ifstream<char> IStreamT;
    typedef std::basic_ofstream<char> OStreamT;
    RHVOICE_CORE_API std::string getShortPathIfNeeded(PathCharT *wide);
    RHVOICE_CORE_API std::wstring string2wstring(char * str);
    RHVOICE_CORE_API std::wstring string2wstring(const std::string &str);
    RHVOICE_CORE_API std::string wstring2string(const std::wstring &wide);
    RHVOICE_CORE_API wchar_t* ourPathGetenv(const char* varName);
#endif

#endif
