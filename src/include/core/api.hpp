#pragma once
#if defined(MSC_VER)
    #define RHVOICE_EXPORT_API __declspec(dllexport)
    #define RHVOICE_IMPORT_API __declspec(dllimport)
#else
  #if defined(_WIN32)
    #define RHVOICE_EXPORT_API [[gnu::dllexport]]
    #define RHVOICE_IMPORT_API [[gnu::dllimport]]
  #else
    #define RHVOICE_EXPORT_API [[gnu::visibility("default")]]
    #define RHVOICE_IMPORT_API
  #endif
#endif
