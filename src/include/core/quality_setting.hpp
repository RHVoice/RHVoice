/* Copyright (C) 2017, 2018, 2022  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_QUALITY_SETTING_HPP
#define RHVOICE_QUALITY_SETTING_HPP

#include "property.hpp"
namespace RHVoice
{
  enum quality_t
    {
      quality_none=0,
      quality_min=1,
      quality_std,
      quality_max
    };

  class quality_setting: public enum_property<quality_t>
  {
  public:
    quality_setting():
      enum_property<quality_t>("quality",quality_std)
    {
      define("min",quality_min);
      define("minimum",quality_min);
      define("0",quality_min);
      define("standard",quality_std);
      define("std",quality_std);
      define("default",quality_std);
      define("50",quality_std);
      define("max",quality_max);
      define("maximum",quality_max);
      define("100",quality_max);
    }
  };
}
#endif
