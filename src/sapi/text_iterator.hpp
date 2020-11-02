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

#ifndef RHVOICE_SAPI_TEXT_ITERATOR_HPP
#define RHVOICE_SAPI_TEXT_ITERATOR_HPP

#include <iterator>
#include "sapiddk.h"
#include "sapi.h"

#include "core/utf.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class text_iterator: public std::iterator<std::forward_iterator_tag,const utf8::uint32_t>
    {
    public:
      text_iterator();

      text_iterator(const SPVTEXTFRAG* frag,std::size_t pos);

      const utf8::uint32_t& operator*() const;

      bool operator==(const text_iterator& other) const;

      bool operator!=(const text_iterator& other) const;

      text_iterator& operator++();

      text_iterator operator++(int);

      std::size_t offset() const;

    private:
      const SPVTEXTFRAG* text_frag;
      const wchar_t *range_end,*start,*end;
      utf8::uint32_t code_point;
    };
  }
}
#endif
