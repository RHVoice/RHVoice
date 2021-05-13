/* Copyright (C) 2012, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SD_SPEECH_SYNTHESIZER_HPP
#define RHVOICE_SD_SPEECH_SYNTHESIZER_HPP

#include "core/threading.hpp"
#include "core/client.hpp"
#include "base.hpp"
#include "state.hpp"
#include "tts_message.hpp"
#include "speech_player.hpp"

namespace RHVoice
{
  namespace sd
  {
    class speech_synthesizer: public base, public threading::thread,public client
    {
    public:
      speech_synthesizer():
        status(status_empty)
      {
      }

      void stop();
      void send_message(const tts_message& msg);

      event_mask get_supported_events() const
      {
        return event_mark;
      }

      unsigned int get_audio_buffer_size() const
      {
        return 20;
      }

      bool process_mark(const std::string& name);
      bool play_speech(const short* samples,std::size_t count);
      bool set_sample_rate(int sample_rate);

    private:
      bool cancelled_or_pausing()
      {
        state s;
        return (s.test(state::stopped)||
                s.test(state::paused)||
                s.test(state::pausing));
      }

      enum status_t
        {
          status_empty,
          status_message,
          status_stopped
        };

      bool get_message(tts_message& msg);
      void synthesize_message(const tts_message& msg);
      void signal_stop();
      void prepare();
      void run();
      bool is_ssml(const tts_message& msg) const;

      status_t status;
      tts_message current_message;
      threading::mutex message_mutex;
      threading::condition_variable message_available;
      speech_player player;
    };
  }
}
#endif
