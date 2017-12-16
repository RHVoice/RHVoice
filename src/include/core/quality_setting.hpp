/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_QUALITY_SETTING_HPP
#define RHVOICE_QUALITY_SETTING_HPP

#include "property.hpp"

namespace RHVoice
{
  class quality_setting: public numeric_property<int>
  {
  public:
    quality_setting():
      numeric_property("quality",50,0,100)
    {
    }
  };
}
#endif
