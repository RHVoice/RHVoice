/* Copyright (C) 2012, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <iterator>
#include "audio.hpp"
#include "library.hpp"
#include "playback_stream_impl.hpp"
#ifdef WITH_LIBAO
#include "libao.hpp"
#endif
#ifdef WITH_PULSE
#include "pulse.hpp"
#endif
#ifdef WITH_PORTAUDIO
#include "portaudio.hpp"
#endif

#include "file_playback_stream_impl.hpp"

namespace RHVoice
{
  namespace audio
  {
    std::vector<std::shared_ptr<library> > playback_stream::libraries(playback_stream::init_libraries());

    std::vector<std::shared_ptr<library> > playback_stream::init_libraries()
    {
      std::vector<std::shared_ptr<library> > result;
      std::back_insert_iterator<std::vector<std::shared_ptr<library> > > out(result);
      #ifdef WITH_PULSE
      init_library<pulse_library>(out);
      #endif
      #ifdef WITH_PORTAUDIO
      init_library<portaudio_library>(out);
      #endif
      #ifdef WITH_LIBAO
      init_library<libao_library>(out);
      #endif
      return result;
    }

    void playback_stream::set_sample_rate(int sample_rate)
    {
      if(sample_rate!=params.sample_rate)
        {
          if(is_open())
            throw is_open_error();
          else
            params.sample_rate=sample_rate;
        }
    }

    bool playback_stream::is_open() const
    {
      return ((impl.get()!=0)&&impl->is_open());
    }

    void playback_stream::close()
    {
      if(impl.get()!=0)
        impl->close();
    }

    void playback_stream::open()
    {
      if(impl.get()==0)
        {
          std::unique_ptr<playback_stream_impl> new_impl;
          if(params.backend==backend_file)
            {
              new_impl.reset(new file_playback_stream_impl(params));
              new_impl->open(params.sample_rate);
              impl = std::move(new_impl);
              return;
            }
          for(std::vector<std::shared_ptr<library> >::iterator it=libraries.begin();it!=libraries.end();++it)
            {
              if(!(*it)->supports_backend(params.backend))
                continue;
              if(params.lib==lib_default)
                {
                  try
                    {
                      new_impl.reset((*it)->new_playback_stream_impl(params));
                      new_impl->open(params.sample_rate);
                      impl = std::move(new_impl);
                      return;
                    }
                  catch(const error& e)
                    {
                    }
                }
              else if(params.lib==(*it)->get_id())
                {
                  new_impl.reset((*it)->new_playback_stream_impl(params));
                  new_impl->open(params.sample_rate);
                  impl = std::move(new_impl);
                  return;
                }
            }
          if(params.lib==lib_default)
            throw opening_error();
          else
            throw library_error();
        }
      else
        {
          if(impl->is_open())
            throw is_open_error();
          impl->open(params.sample_rate);
        }
    }

    void playback_stream::write(const short* samples,std::size_t count)
    {
      if(is_open())
        impl->write(samples,count);
      else
        throw is_not_open_error();
    }

    void playback_stream::drain()
    {
      if(is_open())
        impl->drain();
      else
        throw is_not_open_error();
    }

    void playback_stream::flush()
    {
      if(is_open())
        impl->flush();
      else
        throw is_not_open_error();
    }

    void playback_stream::idle()
    {
      if(is_open())
        impl->idle();
      else
        throw is_not_open_error();
    }
  }
}
