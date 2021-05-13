/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_SD_CONFIG_COMMAND_HPP
#define RHVOICE_SD_CONFIG_COMMAND_HPP

#include "core/config.hpp"
#include "command.hpp"

namespace RHVoice
{
  namespace sd
  {
    class config_command: public command
    {
    protected:
      void register_setting(abstract_property& setting)
      {
        conf.register_setting(setting);
      }

      template<class S>
      void register_settings(S& settings)
      {
        settings.register_self(conf);
      }

      void reset_settings()
      {
        conf.reset();
      }

      void read_settings();

    private:
      config conf;
    };
  }
}
#endif
