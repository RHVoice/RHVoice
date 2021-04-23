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

#ifndef RHVOICE_PATH_HPP
#define RHVOICE_PATH_HPP

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif
#include "abstraction_layer.h"


namespace RHVoice
{
  namespace path
  {
    PathT join(const PathT& path1,const PathT& path2);
    #ifdef _WIN32
    PathT join(const PathT& path1,const std::string& path2);
    #endif
    bool isdir(const PathT& path);
    bool isfile(const PathT& path);

    class directory
    {
    public:
      explicit directory(const PathT& path);

      ~directory()
      {
        release_dir_handle();
      }

      const PathT& get() const
      {
        return value;
      }

      bool done() const
      {
        return (dir_handle==0);
      }

      void next();

    private:
      directory(const directory&);
      directory& operator=(const directory&);

      void release_dir_handle()
      {
        if(dir_handle)
          {
            #ifdef WIN32
            FindClose(dir_handle);
            #else
            closedir(dir_handle);
            #endif
            dir_handle=0;
          }
      }

      #ifdef WIN32
      HANDLE dir_handle;
      #else
      DIR* dir_handle;
      #endif
      PathT value;
    };
  }
}
#endif
