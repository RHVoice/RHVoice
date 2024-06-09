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

#ifndef RHVOICE_HPP
#define RHVOICE_HPP

#include <iterator>
#include "utf8.h"

namespace RHVoice
{
  namespace utf
  {
    inline bool is_valid(utf8::uint32_t cp)
    {
      return ((cp<=0x10ffff)&&((cp<=0xd7ff)||(cp>=0xe000)));
    }

    template<std::size_t size>
    struct next_impl;
  
    template<>
    struct next_impl<1>
    {
      template<typename iterator> utf8::uint32_t operator()(iterator& it,iterator end) const
      {
        return utf8::next(it,end);
      }
    };

    template<>
    struct next_impl<4>
    {
      template<typename iterator> utf8::uint32_t operator()(iterator& it,iterator end) const
      {
        utf8::uint32_t cp=*it;
        if(!is_valid(cp))
          throw utf8::invalid_code_point(cp);
        ++it;
        return cp;
      }
    };

    template<>
    struct next_impl<2>
    {
      template<typename iterator> utf8::uint32_t operator()(iterator& it,iterator end) const;
    };

    template<typename iterator> utf8::uint32_t next_impl<2>::operator()(iterator& it,iterator end) const
    {
      const utf8::uint32_t rc{0xfffd};
      utf8::uint32_t c1=*it;
      ++it;
      if((c1<0xd800)||(c1>0xdfff))
        {
          return c1;
        }
      else
        {
          if(c1<0xdc00)
            {
              if(it!=end)
                {
                  utf8::uint32_t c2=*it;
                  if((c2>0xdbff)&&(c2<0xe000))
                    {
                      ++it;
                      return (0x10000+((c1-0xd800)<<10)+(c2-0xdc00));
                    }
                  else
                    return rc;
                }
              else
                return rc;
            }
          else
            return rc;
        }
    }

    template<typename iterator> inline utf8::uint32_t next(iterator& it,iterator end)
    {
      if(it==end)
        throw utf8::not_enough_room();
      #ifdef _MSC_VER
      return next_impl<sizeof(std::iterator_traits<iterator>::value_type)>()(it,end);
      #else
      return next_impl<sizeof(typename std::iterator_traits<iterator>::value_type)>()(it,end);
      #endif
    }

    template<std::size_t size>
    struct prev_impl;
  
    template<>
    struct prev_impl<1>
    {
      template<typename iterator> utf8::uint32_t operator()(iterator& it,iterator start) const
      {
        return utf8::prior(it,start);
      }
    };

    template<>
    struct prev_impl<4>
    {
      template<typename iterator> utf8::uint32_t operator()(iterator& it,iterator start) const
      {
        iterator tmp=it;
        --tmp;
        utf8::uint32_t cp=*tmp;
        if((cp>0x10ffff)||((cp>0xd7ff)&&(cp<0xe000)))
          throw utf8::invalid_code_point(cp);
        it=tmp;
        return cp;
      }
    };

    template<>
    struct prev_impl<2>
    {
      template<typename iterator> utf8::uint32_t operator()(iterator& it,iterator start) const;
    };

    template<typename iterator> utf8::uint32_t prev_impl<2>::operator()(iterator& it,iterator start) const
    {
      iterator tmp=it;
      --tmp;
      utf8::uint32_t c2=*tmp;
      if((c2<0xd800)||(c2>0xdfff))
        {
          it=tmp;
          return c2;
        }
      else
        {
          if(c2>0xdbff)
            {
              if(tmp==start)
                throw utf8::invalid_utf16(c2);
              else
                {
                  --tmp;
                  utf8::uint32_t c1=*tmp;
                  if((c1>0xd7ff)&&(c1<0xdc00))
                    {
                      it=tmp;
                      return (0x10000+((c1-0xd800)<<10)+(c2-0xdc00));
                    }
                  throw utf8::invalid_utf16(c1);
                }
            }
          else
            throw utf8::invalid_utf16(c2);
        }
    }

    template<typename iterator> inline utf8::uint32_t prev(iterator& it,iterator start)
    {
      if(it==start)
        throw utf8::not_enough_room();
      #ifdef _MSC_VER
      return prev_impl<sizeof(std::iterator_traits<iterator>::value_type)>()(it,start);
      #else
      return prev_impl<sizeof(typename std::iterator_traits<iterator>::value_type)>()(it,start);
      #endif
    }

    template<typename forward_iterator>
    class text_iterator: public std::iterator<std::forward_iterator_tag,const utf8::uint32_t>
    {
    public:
      text_iterator():
        code_point('\0')
      {
      }

      text_iterator(const forward_iterator& it,const forward_iterator& range_start_,const forward_iterator& range_end_):
        code_point('\0'),
        start(it),
        end(it),
        range_start(range_start_),
        range_end(range_end_)
      {
        ++(*this);
      }

      const utf8::uint32_t& operator*() const
      {
        return code_point;
      }

      bool operator==(const text_iterator& other) const
      {
        return ((range_start==other.range_start)&&(range_end==other.range_end)&&(start==other.start));
      }

      bool operator!=(const text_iterator& other) const
      {
        return !(*this==other);
      }

      text_iterator& operator++()
      {
        if(end==range_end)
          start=end;
        else
          {
            forward_iterator tmp=end;
            code_point=next(tmp,range_end);
            start=end;
            end=tmp;
          }
        return *this;
      }

      text_iterator operator++(int)
      {
        text_iterator tmp=*this;
        ++(*this);
        return tmp;
      }

      forward_iterator base() const
      {
        return start;
      }

      std::size_t offset() const
      {
        return std::distance(range_start,start);
      }

    private:
      utf8::uint32_t code_point;
      forward_iterator start,end,range_start,range_end;
    };
  }
}
#endif
