/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_HTS_ENGINE_SETTING_HPP
#define RHVOICE_HTS_ENGINE_SETTING_HPP

#include "property.hpp"

namespace RHVoice
{
  class hts_engine_setting: public enum_string_property
  {
  public:
    hts_engine_setting():
      enum_string_property("hts_engine")
    {
      define("standard");
      set_default_value("standard");
    }
  };
}
#endif
