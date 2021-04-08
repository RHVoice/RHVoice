/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_TONE_HPP
#define RHVOICE_TONE_HPP

#include <cmath>
#include <vector>

namespace RHVoice
{
  class tone
  {
  public:
    tone(double sample_rate,double freq,double dur)
    {
      double pi=std::acos(static_cast<double>(-1));
      int n=dur*sample_rate;
      samples.reserve(n);
      for(int i=0;i<n;++i)
        samples.push_back(0.5*std::sin(2*pi*freq*(static_cast<double>(i)/sample_rate)));
    }

    const std::vector<double>& operator()() const
    {
      return samples;
    }

  private:
    std::vector<double> samples;
  };
}

#endif
