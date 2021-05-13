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

#ifndef RHVOICE_SD_AUDIO_PARAMS_HPP
#define RHVOICE_SD_AUDIO_PARAMS_HPP

#include "core/config.hpp"

namespace RHVoice
{
  namespace sd
  {
    class audio_params
    {
    public:
      string_property output_method;
      string_property pulse_server;
      string_property alsa_device;
      string_property oss_device;

      audio_params():
        output_method("audio_output_method"),
        pulse_server("audio_pulse_server"),
        alsa_device("audio_alsa_device"),
        oss_device("audio_oss_device")
      {
      }

      void register_self(config& conf)
      {
        conf.register_setting(output_method);
        conf.register_setting(pulse_server);
        conf.register_setting(alsa_device);
        conf.register_setting(oss_device);
      }

    private:
      audio_params(const audio_params&);
      audio_params& operator=(const audio_params&);
    };
  }
}
#endif
