/* Copyright (C) 2021  Olga Yakovleva <olga@rhvoice.org> */

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

#include <cmath>
#include <limits>
#include "core/equalizer.hpp"
#include "core/io.hpp"
#include "core/exception.hpp"

namespace RHVoice
{
  void equalizer::skip_comments(std::istream& s)
  {
    s >> std::ws;
    while(s.peek()=='#')
      {
        s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        s >> std::ws;
      }
  }

  bool equalizer::read_coefs(coefs_t& cs, std::istream& s)
  {
    for(auto& c: cs)
      {
        skip_comments(s);
        if(!(s >> c))
          return false;
      }
    const auto a0=cs[3];
    for(auto& c: cs)
      c/=a0;
    return true;
  }

  equalizer::equalizer(const std::string& path)
  {
    std::ifstream f;
    io::open_ifstream(f, path);
    char h;
    if(!(f >> h >> version) || h!='v')
      throw file_format_error("Error reading eq header");
    if(!(version>=1 && version<=1))
      throw file_format_error("Unsupported eq version");
    coefs_t cs;
    while(read_coefs(cs, f))
      {
        if(sections.empty())
          sections.push_back(section_t(cs, x0, x1, x2));
        else
          {
            const auto& s=sections.back();
            sections.push_back(section_t(cs, s.y0, s.y1, s.y2));
          }
      }
    if(sections.empty())
      throw file_format_error("0 eq sections read");
  }

  void equalizer::reset()
  {
    x2=0;
    x1=0;
    x0=0;
    for(auto& s: sections)
      {
        s.y2=0;
        s.y1=0;
        s.y0=0;
      }
  }

  double equalizer::apply(double x) {
    double y=x;
    x0=x;
    for(auto& s: sections)
      {
        s.y0=s.b0*s.x0+s.b1*s.x1+s.b2*s.x2-s.a1*s.y1-s.a2*s.y2;
        y=s.y0;
      }
    x2=x1;
    x1=x0;
    for(auto& s: sections)
      {
        s.y2=s.y1;
        s.y1=s.y0;
      }
    return y;
  }
}

