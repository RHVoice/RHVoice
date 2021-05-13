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

#include <algorithm>
#include "core/unicode.hpp"

namespace RHVoice
{
  namespace unicode
  {
    namespace
    {
      struct record
      {
        utf8::uint32_t code;
        category_t category;
        utf8::uint32_t upper,lower;
        unsigned int properties;
      };

      #include "unidata.cpp"

      const record* end=&records[0]+(sizeof(records)/sizeof(record));

      const category_t empty_category={0,0};
      const record empty_record={0,empty_category,0,0,0};

      inline bool compare(const record& r1,const record& r2)
      {
        return (r1.code<r2.code);
      }

      const record* find(utf8::uint32_t c)
      {
        record r=empty_record;
        r.code=c;
        const record* res=std::lower_bound(&records[0],end,r,compare);
        return (((res!=end)&&(res->code==c))?res:0);
      }
    }

    extern const category_t category_Cc={'C','c'};
    extern const category_t category_Cf={'C','f'};
    extern const category_t category_Co={'C','o'};
    extern const category_t category_Cs={'C','s'};
    extern const category_t category_Ll={'L','l'};
    extern const category_t category_Lm={'L','m'};
    extern const category_t category_Lo={'L','o'};
    extern const category_t category_Lt={'L','t'};
    extern const category_t category_Lu={'L','u'};
    extern const category_t category_Mc={'M','c'};
    extern const category_t category_Me={'M','e'};
    extern const category_t category_Mn={'M','n'};
    extern const category_t category_Nd={'N','d'};
    extern const category_t category_Nl={'N','l'};
    extern const category_t category_No={'N','o'};
    extern const category_t category_Pc={'P','c'};
    extern const category_t category_Pd={'P','d'};
    extern const category_t category_Pe={'P','e'};
    extern const category_t category_Pf={'P','f'};
    extern const category_t category_Pi={'P','i'};
    extern const category_t category_Po={'P','o'};
    extern const category_t category_Ps={'P','s'};
    extern const category_t category_Sc={'S','c'};
    extern const category_t category_Sk={'S','k'};
    extern const category_t category_Sm={'S','m'};
    extern const category_t category_So={'S','o'};
    extern const category_t category_Zl={'Z','l'};
    extern const category_t category_Zp={'Z','p'};
    extern const category_t category_Zs={'Z','s'};

    category_t category(utf8::uint32_t c)
    {
      const record* r=find(c);
      return (r?(r->category):empty_category);
    }

    utf8::uint32_t toupper(utf8::uint32_t c)
    {
      const record* r=find(c);
      return (r?(r->upper):c);
    }

    utf8::uint32_t tolower(utf8::uint32_t c)
    {
      const record* r=find(c);
      return (r?(r->lower):c);
    }

    unsigned int properties(utf8::uint32_t c)
    {
      const record* r=find(c);
      return (r?(r->properties):0);
    }
  }
}
