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

#ifdef WIN32
#include <wchar.h>
#endif
#include "core/io.hpp"
#include "utf8.h"

namespace RHVoice
{
  namespace io
  {

    std::string appendChildPathToErrorMessage(const char * msgC, const PathT& path){
      std::string errorMsg {msgC};
      #if defined(_WIN32)
      errorMsg += wstring2string(path);
      #else
      errorMsg += path;
      #endif
      return errorMsg;
    }

    file_handle open_file(const PathT& path,const std::string& mode)
    {
      #ifdef WIN32
      std::wstring wmode;
      utf8::utf8to16(mode.begin(),mode.end(),std::back_inserter(wmode));
      file_handle result(_wfopen(path.c_str(),wmode.c_str()),std::fclose);
      #else
      file_handle result(std::fopen(path.c_str(),mode.c_str()),std::fclose);
      #endif
      if(! result)
        throw open_error(path);
      return result;
    }

    void open_ifstream(IStreamT &stream, const PathT& path, bool binary)
    {
      IStreamT::openmode mode=IStreamT::in;
      if(binary)
        mode|=IStreamT::binary;
      stream.open(path.data(),mode);
      if(!stream.is_open())
        throw open_error(path);
    }

    void open_ofstream(OStreamT& stream, const PathT &path, bool binary)
    {
      OStreamT::openmode mode=std::ofstream::out;
      if(binary)
        mode|=OStreamT::binary;
      stream.open(path.data(),mode);
      if(!stream.is_open())
        throw open_error(path);
    }
  }
}
