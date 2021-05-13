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

#ifndef RHVOICE_SD_SPEECH_PLAYER_HPP
#define RHVOICE_SD_SPEECH_PLAYER_HPP

#include <vector>

#include "core/threading.hpp"
#include "base.hpp"
#include "state.hpp"

namespace RHVoice
{
  namespace sd
  {
    class synth_result: public base
    {
    public:
      virtual ~synth_result()
      {
      }

      virtual void output()=0;

    protected:
      enum event_id
        {
          event_index_mark=700,
          event_begin=701,
          event_end=702,
          event_stop=703,
          event_pause=704
        };

      struct event
      {
        event_id id;
        std::string name;
        std::string content;

        event(event_id id_,const std::string& name_,const std::string& content_=""):
          id(id_),
          name(name_),
          content(content_)
        {
        }
      };

      static const event started_speaking;
      static const event stopped_speaking;
      static const event paused_speaking;
      static const event finished_speaking;

      struct reached_index_mark: public event
      {
        explicit reached_index_mark(const std::string& mark):
          event(event_index_mark,"INDEX MARK",mark)
        {
        }
      };

      void report_event(const event& e);
    };

    typedef std::shared_ptr<synth_result> synth_result_ptr;

    class start_of_speech: public synth_result
    {
    public:
      void output();
    };

    class end_of_speech: public synth_result
    {
    public:
      void output();

    private:
      event which_event();
      void clear_state();

    };

    class speech_chunk: public synth_result
    {
    public:
      explicit speech_chunk(const short* samples,std::size_t count):
        data(samples,samples+count)
      {
      }

      void output();

    private:
      std::vector<short> data;
    };

    class sample_rate_setting: public synth_result
    {
    public:
      explicit sample_rate_setting(int sr):
        sample_rate(sr)
      {
      }

      void output();

    private:
      int sample_rate;
    };


    class index_mark: public synth_result
    {
    public:
      explicit index_mark(const std::string& name):
        mark(name)
      {
      }

      void output();

    private:
      bool check_state()
      {
        state s;
        if(s.test(state::stopped)||s.test(state::paused))
          return false;
        if(s.test(state::pausing))
          {
            s.clear(state::pausing);
            s.set(state::paused);
          }
        return true;
      }

      std::string mark;
    };

    class speech_player: public base,public threading::thread
    {
    public:
      void enqueue(const synth_result_ptr& ptr)
      {
        speech_queue.push(ptr);
      }

      void stop()
      {
        speech_queue.stop();
      }

    private:
      void run();

      threading::queue<synth_result_ptr> speech_queue;
    };
  }
}
#endif
