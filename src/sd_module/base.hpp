/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SD_BASE_HPP
#define RHVOICE_SD_BASE_HPP


#include "core/engine.hpp"
#include "core/voice_profile.hpp"
#include "state.hpp"
#include "audio.hpp"
#include "tts_params.hpp"
#include "audio_params.hpp"

namespace RHVoice
{
  namespace sd
  {
    class base
    {
    protected:
      base()
      {
      }

      ~base()
      {
      }

      bool cancelled() const
      {
        state s;
        return (s.test(state::stopped)||
                s.test(state::paused));
      }

      static tts_params tts_settings;
      static std::shared_ptr<engine> tts_engine;
      static voice_profile current_voice;
      static audio_params audio_settings;
      static audio::playback_stream playback_stream;

    private:
      base(const base&);
      base& operator=(const base&);
    };
  }
}
#endif
