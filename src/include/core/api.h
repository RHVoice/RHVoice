#pragma once
#if defined(_MSC_VER)
    #define RHVOICE_EXPORT_API __declspec(dllexport)
    #define RHVOICE_IMPORT_API __declspec(dllimport)
#else
  #if defined(_WIN32)
    #define RHVOICE_EXPORT_API __attribute__((dllexport))
    #define RHVOICE_IMPORT_API __attribute__((dllimport))
  #else
    #define RHVOICE_EXPORT_API __attribute__((visibility("default")))
    #define RHVOICE_IMPORT_API
  #endif
#endif

#ifdef RHVOICE_CORE_EXPORTS
  #define RHVOICE_CORE_API RHVOICE_EXPORT_API
#else
  #define RHVOICE_CORE_API RHVOICE_IMPORT_API
#endif
