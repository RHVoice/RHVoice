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

#include"speech_player.hpp"
#include "io.hpp"

namespace RHVoice
{
  namespace sd
  {
    const synth_result::event synth_result::started_speaking(event_begin,"BEGIN");
    const synth_result::event synth_result::stopped_speaking(event_stop,"STOP");
    const synth_result::event synth_result::paused_speaking(event_pause,"PAUSE");
    const synth_result::event synth_result::finished_speaking(event_end,"END");

    void synth_result::report_event(const event& e)
    {
      if(e.id==event_index_mark)
        logger::log(5,"Reporting index mark ",e.content);
      else
        logger::log(5,"Reporting event ",e.name);
      message m;
      if(e.id==event_index_mark)
        m(e.id,"-",e.content);
      m(e.id," ",e.name);
    }

    void start_of_speech::output()
    {
      report_event(started_speaking);
    }

    synth_result::event end_of_speech::which_event()
    {
      state s;
      if(s.test(state::stopped))
        return stopped_speaking;
      else if(s.test(state::paused))
        return paused_speaking;
      else
        return finished_speaking;
    }

    void end_of_speech::clear_state()
    {
      state s;
      s.clear(state::pausing);
      s.clear(state::paused);
      s.clear(state::stopped);
      s.clear(state::speaking);
    }

    void end_of_speech::output()
    {
      event final_event(which_event());
      report_event(final_event);
      clear_state();
      if(playback_stream.is_open())
        {
          try
            {
              playback_stream.idle();
            }
          catch(const audio::error& e)
            {
              logger::log(2,e.what(),"\nClosing playback stream");
              playback_stream.close();
            }
        }
    }

    void speech_chunk::output()
    {
      if(cancelled())
        return;
      try
        {
          if(!playback_stream.is_open())
            {
              logger::log(2,"Reopening playback stream");
              playback_stream.open();
            }
          logger::log(5,"Writing ",data.size()," samples to the playback stream");
          playback_stream.write(&data[0],data.size());
        }
      catch(const audio::error& e)
        {
          state::Set(state::stopped);
          logger::log(2,e.what());
          if(playback_stream.is_open())
            {
              logger::log(2,"Closing playback stream");
              playback_stream.close();
            }
        }
    }

    void sample_rate_setting::output()
    {
      if(cancelled())
        return;
      logger::log(2,"Setting sample rate to ",sample_rate,", current sample rate is ",playback_stream.get_sample_rate());
      if(playback_stream.get_sample_rate()==sample_rate)
        return;
      try
        {
          if(playback_stream.is_open())
            {
              logger::log(2,"Playback stream is already open, draining");
              playback_stream.drain();
              logger::log(2,"Closing playback stream");
              playback_stream.close();
            }
          playback_stream.set_sample_rate(sample_rate);
        }
      catch(const audio::error& e)
        {
          state::Set(state::stopped);
          logger::log(2,e.what());
          if(playback_stream.is_open())
            {
              logger::log(2,"Closing playback stream");
              playback_stream.close();
            }
        }
    }

    void index_mark::output()
    {
      if(check_state())
        report_event(reached_index_mark(mark));
    }

    void speech_player::run()
    {
      try
        {
          synth_result_ptr ptr;
          while(speech_queue.pop(ptr))
            {
              ptr->output();
            }
        }
      catch(const std::exception& e)
        {
          logger::log(2,"Fatal error in the playback thread: ",e.what());
        }
    }
  }
}
