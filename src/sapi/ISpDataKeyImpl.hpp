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

#ifndef RHVOICE_SAPI_ISPDATAKEYIMPL_HPP
#define RHVOICE_SAPI_ISPDATAKEYIMPL_HPP

#include <string>
#include <map>
#include <functional>
#include <windows.h>
#include <sapi.h>
#include <sapiddk.h>
#include <sperror.h>

#include "core/utf.hpp"
#include "core/str.hpp"
#include "com.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class ISpDataKeyImpl: public ISpDataKey
    {
    public:
      STDMETHOD(SetData)(LPCWSTR pszValueName,ULONG cbData,const BYTE *pData);
      STDMETHOD(GetData)(LPCWSTR pszValueName,ULONG *pcbData,BYTE *pData);
      STDMETHOD(SetStringValue)(LPCWSTR pszValueName,LPCWSTR pszValue);
      STDMETHOD(GetStringValue)(LPCWSTR pszValueName,LPWSTR *ppszValue);
      STDMETHOD(SetDWORD)(LPCWSTR pszValueName,DWORD dwValue);
      STDMETHOD(GetDWORD)(LPCWSTR pszKeyName,DWORD *pdwValue);
      STDMETHOD(OpenKey)(LPCWSTR pszSubKeyName,ISpDataKey **ppSubKey);
      STDMETHOD(CreateKey)(LPCWSTR pszSubKeyName,ISpDataKey **ppSubKey);
      STDMETHOD(DeleteKey)(LPCWSTR pszSubKeyName);
      STDMETHOD(DeleteValue)(LPCWSTR pszValueName);
      STDMETHOD(EnumKeys)(ULONG Index,LPWSTR *ppszSubKeyName);
      STDMETHOD(EnumValues)(ULONG Index,LPWSTR *ppszValueName);

    protected:
      struct str_less: public std::binary_function<const std::wstring&,const std::wstring&,bool>
      {
        bool operator()(const std::wstring& s1,const std::wstring& s2) const
        {
          std::wstring::const_iterator pos1=s1.begin();
          std::wstring::const_iterator pos2=s2.begin();
          utf8::uint32_t cp1,cp2;
          while((pos1!=s1.end())&&(pos2!=s2.end()))
            {
              cp1=str::tolower(utf::next(pos1,s1.end()));
              cp2=str::tolower(utf::next(pos2,s2.end()));
              if(cp1!=cp2)
                return (cp1<cp2);
            }
          return ((pos1==s1.end())&&(pos2!=s2.end()));
        }
      };

    private:
      typedef std::map<std::wstring,std::wstring,str_less> value_map;

      std::wstring default_value;
      value_map values;

    public:
      void set(const std::wstring& name,const std::wstring& value)
      {
        values[name]=value;
      }

      void set(const std::wstring& value)
      {
        default_value=value;
      }

    protected:
      void* get_interface(REFIID riid)
      {
        return com::try_primary_interface<ISpDataKey>(this,riid);
      }
    };
  }
}
#endif
