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

#include <pulse/simple.h>
#include "pulse.hpp"
#include "playback_stream_impl.hpp"

namespace RHVoice
{
  namespace audio
  {
    namespace
    {
      const       uint32_t default_value=-1;

      class pulse_playback_stream_impl: public playback_stream_impl
      {
      public:
        explicit pulse_playback_stream_impl(const playback_params& params);
        void write(const short* samples,std::size_t count);
        void drain();
        void flush();
        void open(uint32_t sample_rate);
        bool is_open() const;
        void close();

      private:
        std::string device;
        std::string server;
        std::string client_name;
        unsigned int buffer_size;
        pa_simple* connection;
      };

      pulse_playback_stream_impl::pulse_playback_stream_impl(const playback_params& params):
        device(params.device),
        server(params.server),
        client_name(params.client_name),
        buffer_size(params.buffer_size),
        connection(0)
      {
        if((params.backend!=backend_default)&&(params.backend!=backend_pulse))
          throw backend_error();
      }

    void pulse_playback_stream_impl::open(uint32_t sample_rate)
    {
      pa_sample_spec sample_spec={PA_SAMPLE_S16NE,sample_rate,1};
      pa_buffer_attr attr={default_value,default_value,default_value,default_value,default_value};
      if(buffer_size!=0)
        attr.tlength=static_cast<double>(buffer_size)/1000*sample_rate;
      connection=pa_simple_new(server.empty()?0:server.c_str(),
                               client_name.empty()?"RHVoice":client_name.c_str(),
                               PA_STREAM_PLAYBACK,
                               device.empty()?0:device.c_str(),
                               "Speech",
                               &sample_spec,
                               0,
                               &attr,
                               0);
      if(connection==0)
        throw opening_error();
    }

      bool pulse_playback_stream_impl::is_open() const
      {
        return (connection!=0);
      }

      void pulse_playback_stream_impl::close()
      {
        if(connection)
          {
            pa_simple_free(connection);
            connection=0;
          }
      }

      void pulse_playback_stream_impl::write(const short* samples,std::size_t count)
      {
        if(pa_simple_write(connection,samples,2*count,0)<0)
          throw playback_error();
      }

      void pulse_playback_stream_impl::drain()
      {
        if(pa_simple_drain(connection,0)<0)
          throw playback_error();
      }

      void pulse_playback_stream_impl::flush()
      {
        if(pa_simple_flush(connection,0)<0)
          throw playback_error();
      }
    }

    lib_id pulse_library::get_id() const
    {
      return lib_pulse;
    }

    playback_stream_impl* pulse_library::create_playback_stream_impl(const playback_params& params) const
    {
      return new pulse_playback_stream_impl(params);
    }

    bool pulse_library::supports_backend(backend_id id) const
    {
      return ((id==backend_pulse)||(id==backend_default));
}
  }
}
