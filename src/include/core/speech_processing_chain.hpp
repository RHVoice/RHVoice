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

#ifndef RHVOICE_SPEECH_PROCESSING_CHAIN_HPP
#define RHVOICE_SPEECH_PROCESSING_CHAIN_HPP

#include "speech_processor.hpp"

namespace RHVoice
{
  class speech_processing_chain
  {
  public:
    speech_processing_chain():
      player(0),
    sample_rate(sample_rate_24k),
    first(0),
      last(0),
      stopped(false)
    {
    }

    ~speech_processing_chain()
    {
      delete first;
    }

    void set_client(client& player_)
    {
      player=&player_;
    }

    void set_sample_rate(sample_rate_t rate)
    {
      sample_rate=rate;
    }

    void stop()
    {
      stopped=true;
    }

    bool is_stopped() const
    {
      return stopped;
    }

    void append(speech_processor* p)
    {
      if(first)
        {
          last->set_next(p);
          last=p;
        }
      else
        {
          first=p;
          last=first;
        }
      last->initialize(player,sample_rate,&stopped);
    }

    void process(speech_processor::sample_ptr samples,std::size_t count)
    {
      if(is_stopped())
        return;
      if(first)
        first->process(samples,count);
    }
      
    void finish()
    {
      if(is_stopped())
        return;
      if(first)
        first->finish();
    }

  private:
    speech_processing_chain(const speech_processing_chain&);
    speech_processing_chain& operator=(const speech_processing_chain&);

    client* player;
    sample_rate_t sample_rate;
    speech_processor *first,*last;
    bool stopped;
  };
}
#endif
