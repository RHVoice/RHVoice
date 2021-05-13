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

#ifndef RHVOICE_UNICODE_HPP
#define RHVOICE_UNICODE_HPP

#include "utf8.h"

namespace RHVoice
{
  namespace unicode
  {
    struct category_t
    {
      char major_class,subclass;
    };

    inline bool operator==(const category_t& c1,const category_t& c2)
    {
      return ((c1.major_class==c2.major_class)&&(c1.subclass==c2.subclass));
    }

    inline bool operator!=(const category_t& c1,const category_t& c2)
    {
      return !(c1==c2);
    }

    extern const category_t category_Cc;
    extern const category_t category_Cf;
    extern const category_t category_Co;
    extern const category_t category_Cs;
    extern const category_t category_Ll;
    extern const category_t category_Lm;
    extern const category_t category_Lo;
    extern const category_t category_Lt;
    extern const category_t category_Lu;
    extern const category_t category_Mc;
    extern const category_t category_Me;
    extern const category_t category_Mn;
    extern const category_t category_Nd;
    extern const category_t category_Nl;
    extern const category_t category_No;
    extern const category_t category_Pc;
    extern const category_t category_Pd;
    extern const category_t category_Pe;
    extern const category_t category_Pf;
    extern const category_t category_Pi;
    extern const category_t category_Po;
    extern const category_t category_Ps;
    extern const category_t category_Sc;
    extern const category_t category_Sk;
    extern const category_t category_Sm;
    extern const category_t category_So;
    extern const category_t category_Zl;
    extern const category_t category_Zp;
    extern const category_t category_Zs;

    category_t category(utf8::uint32_t c);

    utf8::uint32_t toupper(utf8::uint32_t c);
    utf8::uint32_t tolower(utf8::uint32_t c);

    enum property_t
      {
        property_white_space=1,
        property_dash=2,
        property_quotation_mark=4,
        property_lowercase=8,
        property_uppercase=16,
        property_alphabetic=32,
        property_terminal_punctuation=64,
        property_sterm=128
      };

    unsigned int properties(utf8::uint32_t c);
  }
}
#endif
