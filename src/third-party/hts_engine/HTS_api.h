#pragma once
#if defined(_MSC_VER)
    #define HTS_EXPORT_API __declspec(dllexport)
    #define HTS_IMPORT_API __declspec(dllimport)
#else
  #if defined(_WIN32) && _WIN32
    #define HTS_EXPORT_API __attribute__((dllexport))
    #define HTS_IMPORT_API __attribute__((dllimport))
  #else
    #define HTS_EXPORT_API __attribute__((visibility("default")))
    #define HTS_IMPORT_API
  #endif
#endif


#ifdef HTS_EXPORTS
#    define HTS_API HTS_EXPORT_API
#else
#    define HTS_API HTS_IMPORT_API
#endif
