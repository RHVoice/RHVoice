/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include "core/ini_parser.hpp"
#include "core/io.hpp"
#include "core/config.hpp"

namespace RHVoice
{
  void config::load()
  {
    try
      {
        for(ini_parser p(config_path);!p.done();p.next())
          {
            if(p.get_section().empty())
              {
                registration_map::iterator it=registered_settings.find(p.get_key());
                if(it!=registered_settings.end())
                  it->second->set_from_string(p.get_value());
              }
          }
      }
    catch(const io::open_error&)
      {
      }
    catch(...)
      {
        throw;
      }
  }
}
