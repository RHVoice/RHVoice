/* Copyright (C) 2012, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_CLIENT_HPP
#define RHVOICE_CLIENT_HPP

#include <string>

namespace RHVoice
{
  enum event_flag
    {
      event_mark=1,
      event_word_starts=2,
      event_word_ends=4,
      event_sentence_starts=8,
      event_sentence_ends=16,
      event_audio=32,
event_done=64
    };
  typedef unsigned int event_mask;

  class client
  {
  public:
    virtual ~client()
    {
    }

    virtual unsigned int get_audio_buffer_size() const // in milliseconds
    {
      return 100;
    }

    virtual bool play_speech(const short* samples,std::size_t count)
    {
      return true;
    }

    virtual event_mask get_supported_events() const
    {
      return 0;
    }

    virtual bool process_mark(const std::string& name)
    {
      return true;
    }

    virtual bool play_audio(const std::string& src)
    {
      return true;
    }

    virtual bool set_sample_rate(int sample_rate)
    {
      return true;
    }

    virtual bool sentence_starts(std::size_t position,std::size_t length)
    {
      return true;
    }

    virtual bool sentence_ends(std::size_t position,std::size_t length)
    {
      return true;
    }

    virtual bool word_starts(std::size_t position,std::size_t length)
    {
      return true;
    }

    virtual bool word_ends(std::size_t position,std::size_t length)
    {
      return true;
    }

    virtual void done()
    {
}

    bool configure(int sr)
    {
      if(sr==sample_rate)
        return true;
      bool res=set_sample_rate(sr);
      if(res)
        sample_rate=sr;
      return res;
}

  protected:
    client():
      sample_rate(0)
    {
    }

  private:
    int sample_rate;
  };
}
#endif
