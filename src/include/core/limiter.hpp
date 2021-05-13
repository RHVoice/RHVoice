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

#ifndef RHVOICE_LIMITER_HPP
#define RHVOICE_LIMITER_HPP

#include <cmath>
#include <queue>
#include <set>
#include "sample_rate.hpp"
#include "speech_processor.hpp"

namespace RHVoice
{
  template<typename S>
  class envelope_computer
  {
  public:
    envelope_computer():
      value(0)
    {
}

    void init(RHVoice::sample_rate_t sr)
    {
      attack_coef=std::exp(-1.0/(0.001*sr));
      release_coef=std::exp(-1.0/(0.01*sr));
}

    S operator()(S s)
    {
      if(value<s)
        value=s+attack_coef*(value-s);
      else
        value=s+release_coef*(value-s);
      return value;
}

  private:
    double attack_coef;
    double release_coef;
    S value;
  };

  class limiter: public speech_processor
  {
  public:
    limiter(double volume);

  private:
    double amp2db(double a)
    {
      return 20.0*std::log10(a);
}

    double db2amp(double db)
    {
      return std::pow(10.0,db/20.0);
}

    double compute_gain(double v)
    {
      double db=amp2db(v);
      if(db<threshold)
        return 1.0;
      double gain=slope*(threshold-db);
      return db2amp(gain);
}

    void on_input();
    void on_end_of_input();
    void do_initialize();
    void process_sample();

    const double threshold;
    const double ratio;
    const double slope;
    std::size_t window_size;
    std::queue<sample_type> window;
    std::multiset<sample_type> ordering;
    envelope_computer<sample_type> env_comp;
  };
}
#endif
