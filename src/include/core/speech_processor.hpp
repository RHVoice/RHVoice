/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_SPEECH_PROCESSOR_HPP
#define RHVOICE_SPEECH_PROCESSOR_HPP

#include <vector>
#include "client.hpp"
#include "sample_rate.hpp"

namespace RHVoice
{
  class speech_processor
  {
  public:
    typedef double sample_type;
    typedef const sample_type* sample_ptr;

  protected:
    typedef std::vector<sample_type> buffer_type;

  private:
    speech_processor(const speech_processor&);
    speech_processor& operator=(const speech_processor&);

    virtual void do_initialize()
    {
    }

    virtual void on_input()=0;

    virtual void on_output()
    {
    }

    virtual void on_end_of_input()
    {
    }

    virtual void on_finished()
    {
    }

    virtual std::size_t get_desired_input_size() const
    {
      return 0.005*sample_rate;
    }

    virtual bool fixed_size_input()
    {
      return false;
    }

    virtual bool accepts_insertions() const
    {
      return false;
    }

    bool fill_input_buffer(sample_ptr& start,sample_ptr end);

    speech_processor* next;
    bool* stopped;

  protected:
    sample_rate_t sample_rate;
    buffer_type input,output,insertion;
    client* player;

    speech_processor():
      next(0),
      stopped(0),
      sample_rate(sample_rate_24k),
      player(0)
    {
    }

  public:
    virtual ~speech_processor()
    {
      delete next;
    }

    void initialize(client* player_,sample_rate_t rate,bool* stop_flag)
    {
      player=player_;
      sample_rate=rate;
      stopped=stop_flag;
      do_initialize();
    }

    void set_next(speech_processor* next_)
    {
      next=next_;
    }

    bool is_stopped() const
    {
      return (stopped&&*stopped);
    }

    void stop()
    {
      if(stopped)
        *stopped=true;
    }

    void process(sample_ptr samples,std::size_t count);
    void insert(sample_ptr samples,std::size_t count);
    void finish();
  };
}
#endif
