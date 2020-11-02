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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef WIN32
#include <wchar.h>
#ifdef MINGW_WSTRING_PATH_WORKAROUND
#include <Windows.h>
#endif
#endif
#include "core/io.hpp"
#include "utf8.h"

namespace RHVoice
{
  namespace io
  {

    #if !defined(WIN32)
      std::string getWorkedAroundFileName(const std::string& path){
        return path;
      }
    #else
      std::wstring getWorkedAroundFileNameW(const std::string& path){
        std::wstring wpath;
        utf8::utf8to16(path.begin(),path.end(),std::back_inserter(wpath));
        return wpath;
      }
      #ifdef __MINGW32__
          #ifdef MINGW_WSTRING_PATH_WORKAROUND
            std::string getWorkedAroundFileName(const std::string& path){
              std::wstring short_pathW;
              std::string short_path;
              std::wstring wpath = getWorkedAroundFileNameW(path);
              auto neededSize = GetShortPathNameW(wpath.data(), NULL, 0) + 1;
              short_pathW.reserve(neededSize);
              short_pathW.resize(neededSize);
              short_path.reserve(neededSize);
              short_path.resize(neededSize);
              GetShortPathNameW(wpath.data(), short_pathW.data(), neededSize); // doesn't work, always returns empty string
              std::copy(begin(short_pathW), end(short_pathW), begin(short_path));
              return short_path;
            }
          #else
              std::string getWorkedAroundFileName(const std::string& path){
                // We usually use relative paths in ASCII on Windows, for them should work fine as it is.
                return path;
              }
          #endif
      #else
          std::wstring getWorkedAroundFileName(const std::string& path){
            return getWorkedAroundFileNameW(path);
          }
      #endif
    #endif

    file_handle open_file(const std::string& path,const std::string& mode)
    {
      #ifdef WIN32
      std::wstring wpath,wmode;
      utf8::utf8to16(path.begin(),path.end(),std::back_inserter(wpath));
      utf8::utf8to16(mode.begin(),mode.end(),std::back_inserter(wmode));
      file_handle result(_wfopen(wpath.c_str(),wmode.c_str()),std::fclose);
      #else
      file_handle result(std::fopen(path.c_str(),mode.c_str()),std::fclose);
      #endif
      if(! result)
        throw open_error(path);
      return result;
    }

    
    void open_ifstream(std::ifstream& stream,const std::string& path,bool binary)
    {
      std::ifstream::openmode mode=std::ifstream::in;
      if(binary)
        mode|=std::ifstream::binary;
      stream.open(getWorkedAroundFileName(path).data(),mode);
      if(!stream.is_open())
        throw open_error(path);
    }

    void open_ofstream(std::ofstream& stream,const std::string& path,bool binary)
    {
      std::ofstream::openmode mode=std::ofstream::out;
      if(binary)
        mode|=std::ofstream::binary;
      stream.open(getWorkedAroundFileName(path).data(),mode);
      if(!stream.is_open())
        throw open_error(path);
    }
  }
}
