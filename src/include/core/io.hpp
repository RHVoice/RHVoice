/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_IO_HPP
#define RHVOICE_IO_HPP

#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <vector>
#include <memory>

#include "api.h"
#include "exception.hpp"

#ifdef RHVOICE_IO_EXPORTS
  #define RHVOICE_IO_API RHVOICE_EXPORT_API
#else
  #define RHVOICE_IO_API RHVOICE_IMPORT_API
#endif

#include "abstraction_layer.h"

namespace RHVoice
{
  namespace io
  {
    RHVOICE_IO_API std::string appendChildPathToErrorMessage(const char * msgC, const PathT& path);

    class open_error: public exception
    {
    public:
      open_error():
        exception("Unable to open a file")
      {
      }

      open_error(const PathT& path): exception(appendChildPathToErrorMessage("Unable to open ", path))
      {

      }
    };

    typedef std::shared_ptr<FILE> file_handle;

    RHVOICE_IO_API file_handle open_file(const PathT &path, const std::string& mode);
    RHVOICE_IO_API void open_ifstream(IStreamT& stream, const PathT &path, bool binary=false);
    RHVOICE_IO_API void open_ofstream(OStreamT &stream, const PathT& path, bool binary=false);

    union host_endianness
    {
      int i;
      char c[sizeof(int)];

      host_endianness():
        i(1)
      {
      }

      bool is_big() const
      {
        return (c[0]==0);
      }

      bool is_little() const
      {
        return (c[0]!=0);
      }
    };

    template<typename T>
    inline IStreamT& read_integer(IStreamT& in,T& out)
    {
      IStreamT::char_type b[sizeof(T)];
      if(in.read(b,sizeof(T)))
        {
          if(host_endianness().is_little())
          std::reverse(b,b+sizeof(T));
          out=*reinterpret_cast<T*>(b);
        }
      return in;
    }

    template<typename T>
    struct integer_reader: public std::binary_function<IStreamT&,T&,IStreamT&>
    {
      IStreamT& operator()(IStreamT& in,T& out) const
      {
        return read_integer(in,out);
      }
    };

    inline IStreamT& read_string(IStreamT& in,std::string& out)
    {
      uint8_t len=0;
      if(read_integer(in,len))
        {
          if(len==0)
            out.clear();
          else
            {
              std::vector<IStreamT::char_type> buf(len);
              if(in.read(&buf[0],len))
                out.assign(buf.begin(),buf.end());
            }
        }
      return in;
    }
  }
}
#endif
