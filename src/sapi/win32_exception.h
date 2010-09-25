#ifndef WIN32_EXCEPTION_H
#define WIN32_EXCEPTION_h

#include <stdexcept>

class win32_exception : public std::runtime_error
{
 public:
  const unsigned int code;
  explicit win32_exception(unsigned int exception_code): code(exception_code),std::runtime_error("Win32 exception") {}
};

#endif
