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

#ifndef RHVOICE_SPEECH_STREAM_HPP
#define RHVOICE_SPEECH_STREAM_HPP

#include <vector>
#include "sonic.h"

namespace RHVoice
{
  class speech_stream
  {
  public:
    typedef std::vector<short> buffer_type;

    speech_stream(int sample_rate,double rate);

    ~speech_stream()
    {
      if(sonic_stream)
        sonicDestroyStream(sonic_stream);
    }

    void set_start_offset(int offset)
    {
      start_offset=offset;
    }

    double get_rate() const
    {
      return (sonic_stream?sonicGetSpeed(sonic_stream):1);
    }

    void write(short sample);
    bool read(buffer_type& buffer,int min_size=0);
    void flush();

    int get_offset() const
    {
      return offset;
    }

  private:
    speech_stream(const speech_stream&);
    speech_stream& operator=(const speech_stream&);

    void flush_input_buffer();

    int buffer_size;
    buffer_type samples;
    int offset,start_offset;
    sonicStream sonic_stream;
  };
}
#endif
