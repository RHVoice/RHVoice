/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "core/limiter.hpp"

namespace RHVoice
{
  limiter::limiter(double volume):
    threshold(-amp2db(volume)-1),
    ratio(-threshold),
    slope(1.0-(1.0/ratio))
  {
  }

  void limiter::process_sample()
  {
    sample_type s=window.front();
    sample_type v=*(ordering.rbegin());
    window.pop();
    ordering.erase(ordering.find(std::abs(s)));
    output.push_back(s*compute_gain(env_comp(v)));
}

  void limiter::do_initialize()
  {
    window_size=static_cast<std::size_t>(0.005*sample_rate);
    env_comp.init(sample_rate);
}

  void limiter::on_input()
  {
    sample_type s;
    for(buffer_type::const_iterator it=input.begin();it!=input.end();++it)
      {
        s=*it;
        window.push(s);
        ordering.insert(std::abs(s));
        if(window.size()<window_size)
          continue;
        process_sample();
}
}

  void limiter::on_end_of_input()
  {
    while(!window.empty())
      {
        process_sample();
}
  }
}
