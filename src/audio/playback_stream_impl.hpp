/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_AUDIO_PLAYBACK_STREAM_IMPL_HPP
#define RHVOICE_AUDIO_PLAYBACK_STREAM_IMPL_HPP

#include "audio.hpp"

namespace RHVoice
{
  namespace audio
  {
    class playback_stream_impl
    {
    public:
      virtual ~playback_stream_impl()
      {
      }

      virtual void open(uint32_t sample_rate) =0;
      virtual bool is_open() const=0;
      virtual void close()=0;

      virtual void write(const short* samples,std::size_t count)=0;

      virtual void drain()
      {
      }

      virtual void flush()
      {
      }

      virtual void idle()
      {
      }

    protected:
      playback_stream_impl()
      {
      }

    private:
      playback_stream_impl(const playback_stream_impl&);
      playback_stream_impl& operator=(const playback_stream_impl&);
    };
  }
}
#endif
