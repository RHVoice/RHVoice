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

#ifndef RHVOICE_SAPI_UTILS_HPP
#define RHVOICE_SAPI_UTILS_HPP

#include <cstring>
#include <iterator>
#include <string>

#include "utf8.h"

namespace RHVoice
{
  namespace utils
  {
    inline std::wstring string_to_wstring(const std::string& s)
    {
      std::wstring r;
      utf8::utf8to16(s.begin(),s.end(),std::back_inserter(r));
      return r;
    }

    inline std::wstring string_to_wstring(const char* s)
    {
      std::wstring r;
      utf8::utf8to16(s,s+std::strlen(s),std::back_inserter(r));
      return r;
    }

    inline std::string wstring_to_string(const std::wstring& s)
    {
      std::string r;
      utf8::utf16to8(s.begin(),s.end(),std::back_inserter(r));
      return r;
    }

    inline std::string wstring_to_string(const wchar_t* s)
    {
      std::string r;
      utf8::utf16to8(s,s+wcslen(s),std::back_inserter(r));
      return r;
    }

    inline std::string wstring_to_string(const wchar_t* s,std::size_t n)
    {
      std::string r;
      utf8::utf16to8(s,s+n,std::back_inserter(r));
      return r;
    }

    template<typename T>
    class out_ptr
    {
    public:
      template<typename F>
      explicit out_ptr(F f):
        ptr(0),
        del(new deleter<F>(f))
      {
      }

      ~out_ptr()
      {
        release();
        delete del;
      }

      T* get()
      {
        return ptr;
      }

      T** address()
      {
        release();
        return &ptr;
      }

    private:
      out_ptr(const out_ptr&);
      out_ptr& operator=(const out_ptr&);

      class abstract_deleter
      {
      public:
        virtual ~abstract_deleter()
        {
        }

        virtual void destroy(T* p) const=0;
      };

      template<typename F>
      class deleter: public abstract_deleter
      {
      public:
        explicit deleter(F f):
          func(f)
        {
        }

        void destroy(T* p) const
        {
          func(p);
        }

      private:
        F func;
      };

      void release()
      {
        if(ptr)
          {
            del->destroy(ptr);
            ptr=0;
          }
      }

      T* ptr;
      abstract_deleter* del;
    };
  }
}
#endif
