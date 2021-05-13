/* Copyright (C) 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SAMPLE_RATE_HPP
#define RHVOICE_SAMPLE_RATE_HPP

#include "property.hpp"

namespace RHVoice
{
  enum sample_rate_t
    {
      sample_rate_16k=16000,
      sample_rate_22k=22050,
      sample_rate_24k=24000,
      sample_rate_32k=32000,
      sample_rate_44k=44100,
      sample_rate_48k=48000,
      sample_rate_96k=96000
    };

  class sample_rate_property: public enum_property<sample_rate_t>
  {
  public:
    sample_rate_property():
      enum_property<sample_rate_t>("sample_rate",sample_rate_24k)
    {
      define("16k",sample_rate_16k);
      define("22k",sample_rate_22k);
      define("32k",sample_rate_32k);
      define("44k",sample_rate_44k);
      define("48k",sample_rate_48k);
      define("96k",sample_rate_96k);
    }

    sample_rate_property& operator=(sample_rate_t val)
    {
      this->set_value(val);
      return *this;
    }
  };
}
#endif
