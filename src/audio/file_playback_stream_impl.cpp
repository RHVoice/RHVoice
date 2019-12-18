/* Copyright (C) 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdint.h>
#include "core/io.hpp"
#include "file_playback_stream_impl.hpp"

namespace RHVoice
{
  namespace audio
  {
    file_playback_stream_impl::file_playback_stream_impl(const playback_params& params):
      file_path(params.device),
      num_samples(0)
  {
  }

    void file_playback_stream_impl::write(const short* samples,std::size_t count)
    {
      stream.write(reinterpret_cast<const char*>(samples),count*sizeof(short));
      if(!stream)
        throw playback_error();
      num_samples+=count;
    }

    void file_playback_stream_impl::open(uint32_t sample_rate)
    {
      io::open_ofstream(stream,file_path,true);
      stream.write("RIFF",4);
      write_number<uint32_t>(36);
      stream.write("WAVE",4);
      stream.write("fmt ",4);
      write_number<uint32_t>(16);
      write_number<uint16_t>(1);
      write_number<uint16_t>(1);
      write_number<uint32_t>(sample_rate);
      write_number<uint32_t>(sample_rate*2);
      write_number<uint16_t>(2);
      write_number<uint16_t>(16);
      stream.write("data",4);
      write_number<uint32_t>(0);
      if(!stream)
        throw opening_error();
    }

    bool file_playback_stream_impl::is_open() const
    {
      return stream.is_open();
    }

    void file_playback_stream_impl::close()
    {
      if(!is_open())
        return;
      stream.seekp(4);
      std::size_t count=num_samples*sizeof(short);
      write_number<uint32_t>(count+36);
      stream.seekp(40);
      write_number<uint32_t>(count);
      num_samples=0;
      stream.close();
    }
  }
}

