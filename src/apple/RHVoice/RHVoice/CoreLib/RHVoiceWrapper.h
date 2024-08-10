/* Copyright (C) 2012, 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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


#ifndef RHVoiceWrapper_h
#define RHVoiceWrapper_h

#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>

#include "core/engine.hpp"
#include "core/document.hpp"
#include "core/client.hpp"
#include "audio.hpp"

using namespace RHVoice;

namespace
{
  class audio_player: public client
  {
  public:
    explicit audio_player(const std::string& path);
    bool play_speech(const short* samples,std::size_t count);
    void finish();
    bool set_sample_rate(int sample_rate);
    bool set_buffer_size(unsigned int buffer_size);

  private:
    audio::playback_stream stream;
  };

  audio_player::audio_player(const std::string& path)
  {
    if(!path.empty())
      {
        stream.set_backend(audio::backend_file);
        stream.set_device(path);
      }
  }

  bool audio_player::set_sample_rate(int sample_rate)
  {
    try
      {
        if(stream.is_open()&&(stream.get_sample_rate()!=sample_rate))
          stream.close();
        stream.set_sample_rate(sample_rate);
        return true;
      }
    catch(...)
      {
        return false;
      }
  }

  bool audio_player::set_buffer_size(unsigned int buffer_size)
  {
    try
      {
        if(stream.is_open()&&(stream.get_buffer_size()!=buffer_size))
          stream.close();
        stream.set_buffer_size(buffer_size);
        return true;
      }
    catch(...)
      {
        return false;
      }
  }

  bool audio_player::play_speech(const short* samples,std::size_t count)
  {
    try
      {
        if(!stream.is_open())
          stream.open();
        stream.write(samples,count);
        return true;
      }
    catch(...)
      {
        stream.close();
        return false;
      }
  }

  void audio_player::finish()
  {
    if(stream.is_open())
      stream.drain();
  }
}

#endif /* RHVoiceWrapper_h */
