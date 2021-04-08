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
#include <sys/types.h>
#include <wchar.h>
#endif
#include <sys/stat.h>
#include <stdexcept>
#include "core/path.hpp"
#include "utf8.h"

namespace RHVoice
{
  namespace path
  {
    std::string join(const std::string& path1,const std::string& path2)
    {
      if(path1.empty()||path2.empty())
        throw std::invalid_argument("Empty path component");
      std::string result(path1);
      #ifdef WIN32
      if(*result.rbegin()!='\\')
        result.push_back('\\');
      #else
      if(*result.rbegin()!='/')
        result.push_back('/');
      #endif
      result.append(path2);
      return result;
    }

    bool isfile(const std::string& path)
    {
#ifdef WIN32
      struct _stat s;
      std::wstring wpath;
      utf8::utf8to16(path.begin(),path.end(),std::back_inserter(wpath));
      int res=_wstat(wpath.c_str(),&s);
#else
      struct stat s;
      int res=stat(path.c_str(),&s);
#endif
      return ((res==0)&&((s.st_mode&S_IFMT)==S_IFREG));
      }

    bool isdir(const std::string& path)
    {
#ifdef WIN32
      struct _stat s;
      std::wstring wpath;
      utf8::utf8to16(path.begin(),path.end(),std::back_inserter(wpath));
      int res=_wstat(wpath.c_str(),&s);
#else
      struct stat s;
      int res=stat(path.c_str(),&s);
#endif
      return ((res==0)&&((s.st_mode&S_IFMT)==S_IFDIR));
      }

    directory::directory(const std::string& path):
      dir_handle(0)
    {
      #ifdef WIN32
      std::string mask(join(path,"*"));
      std::wstring wmask;
      utf8::utf8to16(mask.begin(),mask.end(),std::back_inserter(wmask));
      WIN32_FIND_DATAW d;
      HANDLE h=FindFirstFileW(wmask.c_str(),&d);
      if(h==INVALID_HANDLE_VALUE)
        return;
      dir_handle=h;
      utf8::utf16to8(d.cFileName,d.cFileName+wcslen(d.cFileName),std::back_inserter(value));
      #else
      dir_handle=opendir(path.c_str());
      if(dir_handle)
        next();
      #endif
    }

    void directory::next()
    {
      if(!done())
        {
          value.clear();
          #ifdef WIN32
          WIN32_FIND_DATAW d;
          if(FindNextFileW(dir_handle,&d))
            utf8::utf16to8(d.cFileName,d.cFileName+wcslen(d.cFileName),std::back_inserter(value));
          else
            release_dir_handle();
          #else
          struct dirent* e;
          do
            {
              e=readdir(dir_handle);
              if(e)
                value.assign(e->d_name);
              else
                {
                  release_dir_handle();
                  break;
                }
            }
          while((value==".")||(value==".."));
          #endif
        }
    }
  }
}
