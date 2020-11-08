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

#include <vector>
#include "registry.hpp"

namespace RHVoice
{
  namespace registry
  {
    key::key(HKEY parent,const std::wstring& name,REGSAM access_mask,bool create)
    {
      LONG result=create?RegCreateKeyEx(parent,name.c_str(),0,nullptr,0,access_mask,0,&handle,0):RegCreateKeyEx(parent,name.c_str(),0,nullptr,0,access_mask,0,&handle,0);
      if(result!=ERROR_SUCCESS)
        throw error("Unable to open/create a registry key");
    }

    key::~key()
    {
      RegCloseKey(handle);
    }

    key::operator HKEY () const
    {
      return handle;
    }

    void key::delete_subkey(const std::wstring& name)
    {
      if(RegDeleteKey(handle,name.c_str())!=ERROR_SUCCESS)
        throw error("Unable to delete a registry key");
    }

    std::wstring key::get() const
    {
      return get(L"");
    }

    void key::set(const std::wstring& value)
    {
      set(L"",value);
    }

    error::error(const std::string& msg): exception(msg){}

    std::wstring key::get(const std::wstring& name) const
    {
      DWORD type,size;
      if(RegQueryValueEx(handle,name.c_str(),0,&type,0,&size)==ERROR_SUCCESS)
        if(type==REG_SZ)
          {
            std::vector<wchar_t> buffer(size/sizeof(wchar_t)+1,0);
            if(RegQueryValueEx(handle,name.c_str(),0,&type,reinterpret_cast<BYTE*>(&buffer[0]),&size)==ERROR_SUCCESS)
              if(type==REG_SZ)
                return std::wstring(&buffer[0]);
              }
      throw error("Unable to read a value from the registry");
    }

    void key::set(const std::wstring& name,const std::wstring& value)
    {
      DWORD size=(value.size()+1)*sizeof(wchar_t);
      if(RegSetValueEx(handle,name.c_str(),0,REG_SZ,reinterpret_cast<const BYTE*>(value.c_str()),size)!=ERROR_SUCCESS)
        throw error("Unable to write a value in the registry");
    }
  }
}
