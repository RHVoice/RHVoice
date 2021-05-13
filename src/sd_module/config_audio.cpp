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

#include <exception>
#include "core/str.hpp"
#include "config_audio.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace cmd
    {
      config_audio::config_audio()
      {
        register_settings(audio_settings);
      }

      bool config_audio::is_valid() const
      {
        return !playback_stream.is_initialized();
      }

      action_t config_audio::execute()
      {
        reply r;
        r("207 OK RECEIVING AUDIO SETTINGS");
        reset_settings();
        read_settings();
        if(initialize_audio())
          {
            r("203-Opened a playback stream");
            r("203 OK AUDIO INITIALIZED");
          }
        else
          {
            r("400-Cannot open a playback stream");
            r("400 ERROR AUDIO NOT INITIALIZED");
          }
        return action_continue;
      }

      bool config_audio::try_output_method(const std::string& method)
      {
        try
          {
            logger::log(2,"Trying to initialize audio output using method ",method);
            playback_stream.reset_params();
            if(method=="pulse")
              {
                playback_stream.set_backend(audio::backend_pulse);
                if(audio_settings.pulse_server.is_set()&&audio_settings.pulse_server.get()!="default")
                  playback_stream.set_server(audio_settings.pulse_server);
              }
            else if(method=="alsa")
              {
                playback_stream.set_backend(audio::backend_alsa);
                if(audio_settings.alsa_device.is_set()&&(audio_settings.alsa_device.get()!="default"))
                  playback_stream.set_device(audio_settings.alsa_device);
              }
            else if(method=="oss")
              {
                playback_stream.set_backend(audio::backend_oss);
                if(audio_settings.oss_device.is_set()&&(audio_settings.oss_device.get()!="default"))
                  playback_stream.set_device(audio_settings.oss_device);
              }
            else if(method=="libao")
              playback_stream.set_lib(audio::lib_libao);
            else
              return false;
            playback_stream.set_buffer_size(20);
            playback_stream.open();
            logger::log(2,"Audio output initialized");
            return true;
          }
        catch(const audio::error& e)
          {
            logger::log(2,"Audio output initialization error: ",e.what());
            return false;
          }
      }

      bool config_audio::initialize_audio()
      {
        str::tokenizer<str::is_equal_to> tok(audio_settings.output_method,str::is_equal_to(','));
        for(str::tokenizer<str::is_equal_to>::iterator it=tok.begin();it!=tok.end();++it)
          {
            if(try_output_method(*it))
              return true;
          }
        return false;
      }
    }
  }
}
