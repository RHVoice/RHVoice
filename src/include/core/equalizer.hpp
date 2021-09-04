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

#ifndef RHVOICE_EQUALIZER_HPP
#define RHVOICE_EQUALIZER_HPP

#include <list>
#include <array>
#include <string>
#include <iostream>

namespace RHVoice
{
class equalizer
{
public:

  equalizer(const std::string& path);
  equalizer(const equalizer&)=delete;
  equalizer& operator=(const equalizer&)=delete;
  double apply(double x);
  void reset();

private:
  using coefs_t=std::array<double, 6>;

  bool read_coefs(coefs_t& cs, std::istream& s);
  void skip_comments(std::istream& s);

  struct section_t
  {
    const double b0, b1, b2, a1, a2;
    const double& x0;
    const double& x1;
    const double& x2;
    double y0{0}, y1{0}, y2{0};

    section_t(const coefs_t cs, const double& i0, const double& i1, const double& i2):
      b0(cs[0]),
      b1(cs[1]),
      b2(cs[2]),
      a1(cs[4]),
      a2(cs[5]),
      x0(i0),
      x1(i1),
      x2(i2)
    {
    }
  };

  unsigned int version{0};
  double x0{0};
  double x1{0};
  double x2{0};
  std::list<section_t> sections;
};
}
#endif
