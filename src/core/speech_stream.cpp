/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <new>
#include "core/speech_stream.hpp"

namespace RHVoice
{
  speech_stream::speech_stream(int sample_rate,double speed):
    buffer_size(0.1*sample_rate),
    offset(0),
    start_offset(0),
    sonic_stream(0)
  {
    if(speed!=1)
      {
        sonic_stream=sonicCreateStream(sample_rate,1);
        if(!sonic_stream)
          throw std::bad_alloc();
        sonicSetSpeed(sonic_stream,speed);
      }
  }

  void speech_stream::write(short sample)
  {
    if(offset<start_offset)
      {
        ++offset;
        return;
      }
    samples.push_back(sample);
    ++offset;
    if(!sonic_stream)
      return;
    if(samples.size()<buffer_size)
      return;
        flush_input_buffer();
  }

  void speech_stream::flush_input_buffer()
  {
    if(!sonic_stream)
      return;
    if(samples.empty())
      return;
    if(!sonicWriteShortToStream(sonic_stream,&samples[0],samples.size()))
      throw std::bad_alloc();
    samples.clear();
  }

  bool speech_stream::read(speech_stream::buffer_type& buffer,int min_size)
  {
    if(sonic_stream)
      {
        if(min_size==0)
          flush_input_buffer();
        int nsamples=sonicSamplesAvailable(sonic_stream);
        if((nsamples==0)||(nsamples<min_size))
          return false;
        buffer.resize(nsamples,0);
        sonicReadShortFromStream(sonic_stream,&buffer[0],nsamples);
      }
    else
      {
        if(samples.empty()||(samples.size()<min_size))
          return false;
        buffer=samples;
        samples.clear();
      }
    return true;
  }

  void speech_stream::flush()
  {
    if(!sonic_stream)
      return;
    flush_input_buffer();
    sonicFlushStream(sonic_stream);
  }
}
