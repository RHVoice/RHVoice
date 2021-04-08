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

#ifndef RHVOICE_SAPI_REGISTRY_HPP
#define RHVOICE_SAPI_REGISTRY_HPP

#include <windows.h>
#include <string>

#include "core/exception.hpp"

namespace RHVoice
{
  namespace registry
  {
    class error: public exception
    {
    public:
      explicit error(const std::string& msg):
        exception(msg)
      {
      }
    };

    class key
    {
    public:
      key(HKEY parent,const std::wstring& name,REGSAM access_mask=KEY_READ,bool create=false)
      {
        LONG result=create?RegCreateKeyEx(parent,name.c_str(),0,0,0,access_mask,0,&handle,0):RegOpenKeyEx(parent,name.c_str(),0,access_mask,&handle);
        if(result!=ERROR_SUCCESS)
          throw error("Unable to open/create a registry key");
      }

      ~key()
      {
        RegCloseKey(handle);
      }

      operator HKEY () const
      {
        return handle;
      }

      void delete_subkey(const std::wstring& name)
      {
        if(RegDeleteKey(handle,name.c_str())!=ERROR_SUCCESS)
          throw error("Unable to delete a registry key");
      }

      std::wstring get(const std::wstring& name) const;

      std::wstring get() const
      {
        return get(L"");
      }

      void set(const std::wstring& name,const std::wstring& value);

      void set(const std::wstring& value)
      {
        set(L"",value);
      }

    private:
      key(const key&);
      key& operator=(const key&);

      HKEY handle;
    };
  }
}
#endif
