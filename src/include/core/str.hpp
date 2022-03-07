/* Copyright (C) 2012, 2014, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */
/* Copyright (C) 2022 Non-Routine LLC.  <lp@louderpages.org> */

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

#ifndef RHVOICE_STR_HPP
#define RHVOICE_STR_HPP

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>
#include <string>
#include <sstream>
#include <locale>
#include "stdexcept"
#include "unicode.hpp"
#include "utf8.h"
#include "utf.hpp"

namespace RHVoice
{
  namespace str
  {
    using unicode::tolower;
    using unicode::toupper;

    typedef utf::text_iterator<std::string::const_iterator> utf8_string_iterator;

    inline utf8_string_iterator utf8_string_begin(const std::string& s)
    {
      return utf8_string_iterator(s.begin(),s.begin(),s.end());
    }

    inline utf8_string_iterator utf8_string_end(const std::string& s)
    {
      return utf8_string_iterator(s.end(),s.begin(),s.end());
    }

    template<typename output_iterator>
    class utf8_insert_iterator: public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    public:
      utf8_insert_iterator() = default;

      explicit utf8_insert_iterator(output_iterator pos_):
      pos(pos_)
      {
      }

      utf8_insert_iterator(const utf8_insert_iterator& other):
        pos(other.pos)
      {
      }

      utf8_insert_iterator& operator=(const utf8_insert_iterator& other)
      {
        pos=other.pos;
        return *this;
      }

      utf8_insert_iterator& operator=(utf8::uint32_t c)
      {
        utf8::append(c,pos);
        return *this;
      }

      utf8_insert_iterator& operator*()
      {
        return *this;
      }

      utf8_insert_iterator& operator++()
      {
        return *this;
      }

      utf8_insert_iterator& operator++(int)
      {
        return *this;
      }

    private:
      output_iterator pos;
    };

    template<typename output_iterator>
    inline utf8_insert_iterator<output_iterator> utf8_inserter(output_iterator pos)
    {
      return utf8_insert_iterator<output_iterator>(pos);
    }

    class append_string_iterator: public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    public:
      append_string_iterator():
        destination_string(0)
      {
      }

      append_string_iterator(const append_string_iterator& other):
      destination_string(other.destination_string)
      {
      }

      explicit append_string_iterator(std::string& dest_str):
        destination_string(&dest_str)
      {
      }

      append_string_iterator& operator=(const append_string_iterator& other)
      {
        destination_string=other.destination_string;
        return *this;
      }

      append_string_iterator& operator=(const std::string& s)
      {
        destination_string->append(s);
        return *this;
      }

      append_string_iterator& operator*()
      {
        return *this;
      }

      append_string_iterator& operator++()
      {
        return *this;
      }

      append_string_iterator& operator++(int)
      {
        return *this;
      }

    private:
      std::string* destination_string;
    };

    inline bool startswith(const std::string& s1,const std::string& s2)
    {
      return (s1.size()<s2.size())?false:(s1.substr(0,s2.size())==s2);
    }

    inline bool endswith(const std::string& s1,const std::string& s2)
    {
      return (s1.size()<s2.size())?false:(s1.substr(s1.size()-s2.size(),s2.size())==s2);
    }

    inline bool isspace(utf8::uint32_t c)
    {
      return (unicode::properties(c)&unicode::property_white_space);
    }

    inline bool isupper(utf8::uint32_t c)
    {
      return ((unicode::properties(c)&unicode::property_uppercase)||(unicode::category(c)==unicode::category_Lt));
    }

    inline bool islower(utf8::uint32_t c)
    {
      return (unicode::properties(c)&unicode::property_lowercase);
    }

    inline bool isalpha(utf8::uint32_t c)
    {
      return (unicode::properties(c)&unicode::property_alphabetic);
    }

    inline bool isquote(utf8::uint32_t c)
    {
      return (unicode::properties(c)&unicode::property_quotation_mark);
    }

    inline bool isdash(utf8::uint32_t c)
    {
      return (unicode::properties(c)&unicode::property_dash);
    }

    inline bool ispunct(utf8::uint32_t c)
    {
      return (unicode::category(c).major_class=='P');
    }

    inline bool istermpunct(utf8::uint32_t c)
    {
      return (unicode::properties(c)&unicode::property_terminal_punctuation);
    }

    inline bool issterm(utf8::uint32_t c)
    {
      return (unicode::properties(c)&unicode::property_sterm);
    }

    inline bool isdigit(utf8::uint32_t c)
    {
      return (unicode::category(c)==unicode::category_Nd);
    }

    inline bool isadigit(utf8::uint32_t c)
    {
      return ((c>='0')&&(c<='9'));
    }

    inline bool issym(utf8::uint32_t c)
    {
      return (unicode::category(c).major_class=='S');
    }

    struct is_space: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return isspace(c);
      }
    };

    struct is_upper: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return isupper(c);
      }
    };

    struct is_lower: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return islower(c);
      }
    };

    struct is_alpha: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return isalpha(c);
      }
    };

    struct is_quote: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return isquote(c);
      }
    };

    struct is_dash: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return isdash(c);
      }
    };

    struct is_punct: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return ispunct(c);
      }
    };

    struct is_termpunct: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return istermpunct(c);
      }
    };

    struct is_sterm: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return issterm(c);
      }
    };

    struct is_digit: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return isdigit(c);
      }
    };

    struct is_adigit: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return isadigit(c);
      }
    };

    struct is_equal_to: public std::unary_function<utf8::uint32_t,bool>
    {
      explicit is_equal_to(utf8::uint32_t c):
      chr(c)
      {
      }

      bool operator()(utf8::uint32_t c) const
      {
        return (c==chr);
      }

    private:
      utf8::uint32_t chr;
    };

    inline std::string trim(const std::string& s)
    {
      std::string::const_iterator last=s.end();
      std::string::const_iterator tmp=last;
      while(last!=s.begin())
        {
          if(isspace(utf8::prior(tmp,s.begin())))
            last=tmp;
          else
            break;
        }
      std::string::const_iterator first=s.begin();
      tmp=first;
      while(first!=last)
        {
          if(isspace(utf8::next(tmp,last)))
            first=tmp;
          else
            break;
        }
      std::string result(first,last);
      return result;
    }

    inline bool equal(const std::string& s1,const std::string& s2)
    {
      std::string::const_iterator pos1=s1.begin();
      std::string::const_iterator pos2=s2.begin();
      while((pos1!=s1.end())&&(pos2!=s2.end()))
        {
          if(tolower(utf8::next(pos1,s1.end()))!=tolower(utf8::next(pos2,s2.end())))
            return false;
        }
      return ((pos1==s1.end())&&(pos2==s2.end()));
    }

    struct less: public std::binary_function<const std::string&,const std::string&,bool>
    {
      bool operator()(const std::string& s1,const std::string& s2) const
      {
        std::string::const_iterator pos1=s1.begin();
        std::string::const_iterator pos2=s2.begin();
        utf8::uint32_t cp1,cp2;
        while((pos1!=s1.end())&&(pos2!=s2.end()))
          {
            cp1=tolower(utf8::next(pos1,s1.end()));
            cp2=tolower(utf8::next(pos2,s2.end()));
            if(cp1!=cp2)
              return (cp1<cp2);
          }
        return ((pos1==s1.end())&&(pos2!=s2.end()));
      }
    };

    template<typename delimiter_predicate>
    class tokenizer
    {
    public:
      class iterator: public std::iterator<std::input_iterator_tag,const std::string>
      {
      public:
        iterator(const utf8_string_iterator& first_,const utf8_string_iterator& last_,delimiter_predicate pred):
          first(first_),
          last(first_),
          end(last_),
          is_delimiter(pred)
        {
          ++(*this);
        }

        const std::string& operator*() const
        {
          return value;
        }

        const std::string* operator->() const
        {
          return &value;
        }

        bool operator==(const iterator& other) const
        {
          return ((first==other.first)&&(last==other.last)&&(end==other.end));
        }

        bool operator!=(const iterator& other) const
        {
          return !(*this==other);
        }

        iterator& operator++()
        {
          first=std::find_if(last,end,std::not1(is_delimiter));
          last=std::find_if(first,end,is_delimiter);
          value.assign(first.base(),last.base());
          return *this;
        }

        iterator operator++(int)
        {
          iterator tmp=*this;
          ++(*this);
          return tmp;
        }

      private:
        utf8_string_iterator first,last,end;
        std::string value;
        delimiter_predicate is_delimiter;
      };

      tokenizer(const std::string& s,delimiter_predicate pred=delimiter_predicate()):
        text(s),
        is_delimiter(pred)
      {
      }

      void assign(const std::string& s)
      {
        text=s;
      }

      iterator begin() const
      {
        return iterator(utf8_string_begin(text),utf8_string_end(text),is_delimiter);
      }

      iterator end() const
      {
        return iterator(utf8_string_end(text),utf8_string_end(text),is_delimiter);
      }

    private:
      std::string text;
      delimiter_predicate is_delimiter;
    };

    template<typename input_iterator> std::size_t count_newlines(input_iterator start,input_iterator end)
    {
      utf8::uint32_t chr;
      utf8::uint32_t prev_chr=0;
      std::size_t n=0;
      for(input_iterator it=start;it!=end;++it)
        {
          chr=*it;
          if(((chr=='\n')&&(prev_chr!='\r'))||(chr=='\r')||(chr==0x85)||(chr==0x2028)||(chr==0x2029))
            ++n;
          prev_chr=chr;
        }
      return n;
    }

    struct to_lower
    {
      utf8::uint32_t operator()(utf8::uint32_t c) const
      {
        return tolower(c);
      }

      std::string operator()(const std::string& s) const
      {
        std::string result;
        std::transform(utf8_string_begin(s),utf8_string_end(s),utf8_inserter(std::back_inserter(result)),tolower);
        return result;
      }
    };

    template<typename T>
    std::string to_string(const T& v)
    {
      std::ostringstream s;
      s.imbue(std::locale::classic());
      s << v;
      return s.str();
    }

    template<typename T>
    T from_string(const std::string& s)
    {
      std::istringstream strm(s);
      strm.imbue(std::locale::classic());
      T result;
      if(strm>>result)
        return result;
      else
        throw std::invalid_argument("Invalid type representation as a string");
    }

    inline bool is_single_char(const std::string& s)
    {
      if(s.empty())
        return false;
      utf8_string_iterator it=utf8_string_begin(s);
      ++it;
      return (it==utf8_string_end(s));
    }

    inline std::string cp2str(utf8::uint32_t cp)
    {
      std::string s;
      utf8::append(cp, std::back_inserter(s));
      return s;
    }

    template<typename oit>
    oit utf8explode(const std::string& s, oit out)
    {
      return std::transform(utf8_string_begin(s), utf8_string_end(s), out, [] (utf8::uint32_t cp)-> std::string {return cp2str(cp);});
    }
  }
}
#endif
