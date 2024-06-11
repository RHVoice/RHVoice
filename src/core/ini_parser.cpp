/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <sstream>
#include <iterator>
#include "core/io.hpp"
#include "core/str.hpp"
#include "core/ini_parser.hpp"

namespace RHVoice
{
  ini_parser::ini_parser(const std::string& file_path,bool standard):
    instream(new std::ifstream),
    standard_format(standard)
  {
    io::open_ifstream(*instream,file_path);
    io::skip_bom(*instream);
    next();
  }

  void ini_parser::next()
  {
    if(instream.get()==0)
      return;
    std::string line;
    while(std::getline(*instream,line))
      {
        if(line.empty())
          continue;
        if(!utf8::is_valid(line.begin(),line.end()))
          continue;
        line=str::trim(line);
        if (line.empty())
          continue;
        if(str::startswith(line,";"))
          continue;
        if(str::startswith(line,"[")&&str::endswith(line,"]"))
          {
            section=str::trim(line.substr(1,line.length()-2));
            if(section.empty())
              break;
          }
        else
          {
            if(standard_format)
              {
                std::size_t pos=line.find('=');
                if((pos==std::string::npos)||(pos==0)||(pos==(line.length()-1)))
                  continue;
                key=str::trim(line.substr(0,pos));
                if(key.empty())
                  continue;
                value=unescape(str::trim(line.substr(pos+1,line.length()-pos-1)));
                if(value.empty())
                  continue;
              }
            else
              {
                key.clear();
                value=line;
              }
            return;
          }
      }
    instream.reset();
  }

  std::string ini_parser::unescape(const std::string& s) const
  {
    if(s.empty())
      return s;
    std::string result;
    result.reserve(s.size());
    std::istringstream strm;
    utf8::uint32_t c;
    std::string::size_type pos=0;
    std::string::size_type next_pos;
    while((pos!=std::string::npos)&&(pos!=s.size()))
      {
        next_pos=s.find('#',pos);
        if(next_pos!=pos)
          result.append(s,pos,(next_pos==std::string::npos)?std::string::npos:(next_pos-pos));
        pos=next_pos;
        if(pos!=std::string::npos)
          {
            strm.clear();
            ++pos;
            next_pos=s.find(';',pos);
            if((next_pos==pos)||(next_pos==std::string::npos))
              return std::string();
            if(s[pos]=='x')
              {
                ++pos;
                if(pos==next_pos)
                  return std::string();
                strm.setf(std::ios::hex,std::ios::basefield);
              }
            else
              strm.setf(std::ios::dec,std::ios::basefield);
            strm.str(s.substr(pos,next_pos-pos));
            if(!(strm >> c))
              return std::string();
            if(!utf::is_valid(c))
              return std::string();
            utf8::append(c,std::back_inserter(result));
            pos=next_pos+1;
          }
      }
    return result;
  }
}
