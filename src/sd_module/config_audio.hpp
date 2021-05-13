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

#ifndef RHVOICE_SD_CONFIG_AUDIO_HPP
#define RHVOICE_SD_CONFIG_AUDIO_HPP

#include "config_command.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      class config_audio: public config_command
      {
      public:
        config_audio();

      private:
        bool is_valid() const;
        action_t execute();
        bool try_output_method(const std::string& method);
        bool initialize_audio();
      };
    }
  }
}
#endif
