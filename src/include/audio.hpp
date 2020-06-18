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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_AUDIO_HPP
#define RHVOICE_AUDIO_HPP

#include <string>
#include <vector>
#include <memory>
#include "core/exception.hpp"
#include "core/smart_ptr.hpp"

namespace RHVoice
{
  namespace audio
  {
    struct error: public exception
    {
      explicit error(const std::string& msg):
        exception(msg)
      {
      }
    };

    struct initialization_error: public error
    {
      initialization_error():
        error("Audio library initialization failed")
      {
      }
    };

    struct is_initialized_error: public error
    {
      is_initialized_error():
        error("The playback stream is already initialized")
      {
      }
    };

    struct opening_error: public error
    {
      opening_error():
        error("Unable to open a playback stream")
      {
      }
    };

    struct is_open_error: public error
    {
      is_open_error():
        error("The playback stream is already open")
      {
      }
    };

    struct is_not_open_error: public error
    {
      is_not_open_error():
        error("The playback stream is not open")
      {
      }
    };

    struct playback_error: public error
    {
      playback_error():
        error("Unable to write to a playback stream")
      {
      }
    };

    struct backend_error: public error
    {
      backend_error():
        error("Unsupported audio backend")
      {
      }
    };

    struct library_error: public error
    {
      library_error():
        error("Unsupported audio library")
      {
      }
    };

    enum lib_id
      {
        lib_default,
        lib_pulse,
        lib_portaudio,
        lib_libao
      };

    enum backend_id
      {
        backend_default,
        backend_pulse,
        backend_alsa,
        backend_oss,
        backend_file
      };

    struct playback_params
    {
      lib_id lib;
      backend_id backend;
      std::string device;
      std::string server;
      std::string client_name;
      unsigned int sample_rate;
      unsigned int buffer_size; // in milliseconds

      playback_params():
        lib(lib_default),
        backend(backend_default),
        sample_rate(24000),
        buffer_size(0)
      {
      }
    };

    class library;
    class playback_stream_impl;

    class playback_stream
    {
    public:
      explicit playback_stream(const playback_params& params_=playback_params());

      ~playback_stream();

      bool is_initialized() const
      {
        return (impl.get()!=0);
      }

      void open();
      bool is_open() const;
      void close();

      void set_lib(lib_id lib)
      {
        if(is_initialized())
          throw is_initialized_error();
        else
          params.lib=lib;
      }

      void set_backend(backend_id backend)
      {
        if(is_initialized())
          throw is_initialized_error();
        else
          params.backend=backend;
      }

      void set_device(const std::string& device)
      {
        if(is_initialized())
          throw is_initialized_error();
        else
          params.device=device;
      }

      void set_server(const std::string& server)
      {
        if(is_initialized())
          throw is_initialized_error();
        else
          params.server=server;
      }

      void set_client_name(const std::string& client_name)
      {
        if(is_initialized())
          throw is_initialized_error();
        else
          params.client_name=client_name;
      }

      void set_buffer_size(unsigned int buffer_size)
      {
        if(is_initialized())
          throw is_initialized_error();
        else
          params.buffer_size=buffer_size;
      }

      int get_sample_rate() const
      {
        return params.sample_rate;
      }

      void set_sample_rate(int sample_rate);

      void reset_params()
      {
        if(is_initialized())
          throw is_initialized_error();
        else
          params=playback_params();
      }

      void write(const short* samples,std::size_t count);
      void drain();
      void flush();
      void idle();

    private:
      playback_stream(const playback_stream&);
      playback_stream& operator=(const playback_stream&);

      template<class LIB_TYPE,typename OUT_TYPE>
      static void init_library(OUT_TYPE o);

      static std::vector<smart_ptr<library> > init_libraries();

      playback_params params;
      std::unique_ptr<playback_stream_impl> impl;
      static std::vector<smart_ptr<library> > libraries;
    };

    template<class LIB_TYPE,typename OUT_TYPE>
    void playback_stream::init_library(OUT_TYPE o)
    {
      try
        {
          *o=smart_ptr<library>(new LIB_TYPE);
          ++o;
        }
      catch(const error& e)
        {
        }
    }
  }
}
#endif
