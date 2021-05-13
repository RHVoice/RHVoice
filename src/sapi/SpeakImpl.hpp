/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SAPI_SPEAKIMPL_HPP
#define RHVOICE_SAPI_SPEAKIMPL_HPP

#include <algorithm>
#include <windows.h>
#include <sapi.h>
#include <sapiddk.h>

#include "core/client.hpp"
#include "core/voice_profile.hpp"
#include "core/document.hpp"
#include "tts_base.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class SpeakImpl: public client, public tts_base
    {
    public:
      struct init_params
      {
        const SPVTEXTFRAG* input;
        ISpTTSEngineSite* caller;
        voice_profile profile;
      };

      SpeakImpl(const init_params& p);
      void operator()();
      event_mask get_supported_events() const;
      bool play_speech(const short* samples,std::size_t count);
      bool process_mark(const std::string& name);
      bool word_starts(std::size_t position,std::size_t length);
      bool sentence_starts(std::size_t position,std::size_t length);

    private:
      SpeakImpl(const SpeakImpl&);
      SpeakImpl& operator=(const SpeakImpl&);

      bool check_actions();
      bool queue_boundary_event(bool is_word,std::size_t position,std::size_t length);

      double convert_rate(long rate) const
      {
        return rate_table[10+std::max<long>(-10,std::min<long>(rate,10))];
      }

      double convert_pitch(long pitch) const
      {
        return pitch_table[24+std::max<long>(-24,std::min<long>(pitch,24))];
      }

      double convert_volume(unsigned long volume) const
      {
        return (std::min<unsigned long>(volume,100)/100.0);
      }

      const static double rate_table[];
      const static double pitch_table[];

      double get_rate()
      {
        long rate;
          if(SUCCEEDED(caller->GetRate(&rate)))
            return convert_rate(rate);
          else
            return 1;
      }

      double get_volume()
      {
        unsigned short volume;
        if(SUCCEEDED(caller->GetVolume(&volume)))
          return convert_volume(volume);
        else
          return 1;
      }

      ISpTTSEngineSite* caller;
      document doc;
      ULONGLONG bytes_written;
    };
  }
}
#endif
