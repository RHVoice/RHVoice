#pragma once
#if defined(_MSC_VER)
  #define MAGE_EXPORT_API __declspec(dllexport)
  #define MAGE_IMPORT_API __declspec(dllimport)
#else
  #if defined(_WIN32) && _WIN32
    #define MAGE_EXPORT_API __attribute__((dllexport))
    #define MAGE_IMPORT_API __attribute__((dllimport))
  #else
    #define MAGE_EXPORT_API __attribute__((visibility("default")))
    #define MAGE_IMPORT_API
  #endif
#endif


#ifdef MAGE_EXPORTS
#  define MAGE_API MAGE_EXPORT_API
#else
#  define MAGE_API MAGE_IMPORT_API
#endif
