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

#ifndef RHVOICE_AUDIO_HPP
#define RHVOICE_AUDIO_HPP

#include <limits>
#include <string>
#include <vector>
#include <memory>
#include "core/exception.hpp"
#include "core/api.hpp"


namespace RHVoice
{
  namespace audio
  {
    struct error: public exception
    {
      explicit error(const std::string& msg);
    };

    struct disallowed_sample_rate: public error
    {
      disallowed_sample_rate();
    };

    struct initialization_error: public error
    {
      initialization_error();
    };

    struct is_initialized_error: public error
    {
      is_initialized_error();
    };

    struct opening_error: public error
    {
      opening_error();
    };

    struct is_open_error: public error
    {
      is_open_error();
    };

    struct is_not_open_error: public error
    {
      is_not_open_error();
    };

    struct playback_error: public error
    {
      playback_error();
    };

    struct backend_error: public error
    {
      backend_error();
    };

    struct library_error: public error
    {
      library_error();
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

      playback_params();
    };

    class library;
    class playback_stream_impl;

    class playback_stream
    {
    public:
      explicit playback_stream(const playback_params& params_=playback_params());

      ~playback_stream();

      bool is_initialized() const;

      void open();
      bool is_open() const;
      void close();

      void set_lib(lib_id lib);

      void set_backend(backend_id backend);

      void set_device(const std::string& device);

      void set_server(const std::string& server);

      void set_client_name(const std::string& client_name);

      unsigned int get_buffer_size() const;

      void set_buffer_size(unsigned int buffer_size);

      int get_sample_rate() const;

      void set_sample_rate(int sample_rate);

      void reset_params();

      void write(const short* samples,std::size_t count);
      void drain();
      void flush();
      void idle();

    private:
      playback_stream(const playback_stream&);
      playback_stream& operator=(const playback_stream&);

      template<class LIB_TYPE,typename OUT_TYPE>
      static void init_library(OUT_TYPE o);

      static std::vector<std::shared_ptr<library> > init_libraries();

      playback_params params;
      std::unique_ptr<playback_stream_impl> impl;
      static std::vector<std::shared_ptr<library> > libraries;
    };

    template<class LIB_TYPE,typename OUT_TYPE>
    void playback_stream::init_library(OUT_TYPE o)
    {
      try
        {
          *o=std::shared_ptr<library>(new LIB_TYPE);
          ++o;
        }
      catch(const error& e)
        {
        }
    }
  }
}
#endif
