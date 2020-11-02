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

#include "text_iterator.hpp"


namespace RHVoice
{
  namespace sapi
  {
      text_iterator::text_iterator():
        text_frag(0),
        range_end(0),
        start(0),
        end(0),
        code_point('\0')
      {
      }

      text_iterator::text_iterator(const SPVTEXTFRAG* frag,std::size_t pos):
        text_frag(frag),
        range_end(frag->pTextStart+frag->ulTextLen),
        start(frag->pTextStart+pos),
        end(start),
        code_point('\0')
      {
        ++(*this);
      }

      const utf8::uint32_t& text_iterator::operator*() const
      {
        return code_point;
      }

      bool text_iterator::operator==(const text_iterator& other) const
      {
        return ((text_frag==other.text_frag)&&(start==other.start));
      }

      bool text_iterator::operator!=(const text_iterator& other) const
      {
        return !(*this==other);
      }

      text_iterator& text_iterator::operator++()
      {
        if(end==range_end)
          start=end;
        else
          {
            const wchar_t* tmp=end;
            code_point=utf::next(tmp,range_end);
            start=end;
            end=tmp;
          }
        return *this;
      }

      text_iterator text_iterator::operator++(int)
      {
        text_iterator tmp=*this;
        ++(*this);
        return tmp;
      }

      std::size_t text_iterator::offset() const
      {
        return (text_frag->ulTextSrcOffset+(start-text_frag->pTextStart));
      }

  }
}
