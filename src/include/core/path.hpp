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

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif
#include <string>


namespace RHVoice
{
  namespace path
  {
    std::string join(const std::string& path1,const std::string& path2);
    bool isdir(const std::string& path);
    bool isfile(const std::string& path);

    class directory
    {
    public:
      explicit directory(const std::string& path);

      ~directory()
      {
        release_dir_handle();
      }

      const std::string& get() const
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
      std::string value;
    };
  }
}
#endif
