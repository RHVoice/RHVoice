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

#include <new>
#include <ao/ao.h>
#include "libao.hpp"
#include "playback_stream_impl.hpp"

namespace RHVoice
{
  namespace audio
  {
    namespace
    {
      class option_list
      {
      public:
        option_list():
          head(0)
        {
        }

        ~option_list()
        {
          if(head!=0)
            ao_free_options(head);
        }

        operator ao_option* () const
        {
          return head;
        }

        void append(const char* key,const std::string& value)
        {
          if(!value.empty())
            if(ao_append_option(&head,key,value.c_str())==0)
              throw std::bad_alloc();
        }

      private:
        option_list(const option_list&);
        option_list& operator=(const option_list&);

        ao_option* head;
      };
      class libao_playback_stream_impl: public playback_stream_impl
      {
      public:
        explicit libao_playback_stream_impl(const playback_params& params);
        void write(const short* samples,std::size_t count);
        void open(uint32_t sample_rate);
        bool is_open() const;
        void close();

      private:
        bool is_file;
        std::string device_name;
        std::string server_name;
        std::string client_name;
        int driver_id;
        ao_device* device_handle;
      };

      libao_playback_stream_impl::libao_playback_stream_impl(const playback_params& params):
        is_file(params.backend==backend_file),
        device_name(params.device),
        server_name(params.server),
        client_name(params.client_name),
        driver_id(-1),
        device_handle(0)
      {
        switch(params.backend)
          {
          case backend_default:
            driver_id=ao_default_driver_id();
            break;
          case backend_pulse:
            driver_id=ao_driver_id("pulse");
            break;
          case backend_alsa:
            driver_id=ao_driver_id("alsa");
            break;
          case backend_oss:
            driver_id=ao_driver_id("oss");
            break;
          case backend_file:
            driver_id=ao_driver_id("wav");
            break;
          default:
            break;
          }
        if(driver_id<0)
          throw backend_error();
      }

    void libao_playback_stream_impl::open(uint32_t sample_rate)
    {
      if(sample_rate > std::numeric_limits<int>::max()){
        throw disallowed_sample_rate();
      }
      ao_sample_format sample_format={16,static_cast<int>(sample_rate),1,AO_FMT_NATIVE,0};
      if(is_file)
        device_handle=ao_open_file(driver_id,device_name.c_str(),1,&sample_format,0);
      else
        {
          option_list options;
          options.append("dev",device_name);
          options.append("server",server_name);
          options.append("client_name",client_name);
          device_handle=ao_open_live(driver_id,&sample_format,options);
        }
      if(device_handle==0)
        throw opening_error();
    }

      bool libao_playback_stream_impl::is_open() const
      {
        return (device_handle!=0);
      }

      void libao_playback_stream_impl::close()
      {
        if(device_handle)
          {
            ao_close(device_handle);
            device_handle=0;
          }
      }

      void libao_playback_stream_impl::write(const short* samples,std::size_t count)
      {
        if(ao_play(device_handle,reinterpret_cast<char*>(const_cast<short*>(samples)),2*count)==0)
          throw playback_error();
      }
    }

    void libao_library::initialize()
    {
      ao_initialize();
    }

    void libao_library::release()
    {
      ao_shutdown();
    }

    lib_id libao_library::get_id() const
    {
      return lib_libao;
    }

    playback_stream_impl* libao_library::create_playback_stream_impl(const playback_params& params) const
    {
      return new libao_playback_stream_impl(params);
    }

    bool libao_library::supports_backend(backend_id id) const
    {
      return ((id==backend_default)||
              (id==backend_pulse)||
              (id==backend_alsa)||
              (id==backend_oss)||
              (id==backend_file));
}
  }
}
