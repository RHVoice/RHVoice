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

#include <portaudio.h>
#include "portaudio.hpp"
#include "playback_stream_impl.hpp"

namespace RHVoice
{
  namespace audio
  {
    namespace
    {
      class portaudio_playback_stream_impl: public playback_stream_impl
      {
      public:
        explicit portaudio_playback_stream_impl(const playback_params& params);
        void write(const short* samples,std::size_t count);
        void drain();
        void flush();
        void idle();
        void open(uint32_t sample_rate);
        bool is_open() const;
        void close();

      private:
        PaDeviceIndex get_device() const;

        std::string device_name;
        unsigned int buffer_size;
        PaHostApiIndex host_api;
        PaStream* stream;
      };

      PaDeviceIndex portaudio_playback_stream_impl::get_device() const
      {
        const PaHostApiInfo * host_api_info=Pa_GetHostApiInfo(host_api);
        if(host_api_info==0)
          return paNoDevice;
        if(device_name.empty())
          return host_api_info->defaultOutputDevice;
        else
          {
            for(int i=0;i<host_api_info->deviceCount;++i)
              {
                PaDeviceIndex device=Pa_HostApiDeviceIndexToDeviceIndex(host_api,i);
                if(device<0)
                  return paNoDevice;
                const PaDeviceInfo* device_info=Pa_GetDeviceInfo(device);
                if(device_name==device_info->name)
                  return device;
              }
            return paNoDevice;
          }
      }

      portaudio_playback_stream_impl::portaudio_playback_stream_impl(const playback_params& params):
        device_name(params.device),
        buffer_size(params.buffer_size),
        host_api(-1),
        stream(0)
      {
        switch(params.backend)
          {
          case backend_default:
            host_api=Pa_GetDefaultHostApi();
            break;
          case backend_alsa:
            host_api=Pa_HostApiTypeIdToHostApiIndex(paALSA);
            break;
          case backend_oss:
            host_api=Pa_HostApiTypeIdToHostApiIndex(paOSS);
            break;
          default:
            break;
          }
        if(host_api<0)
          throw backend_error();
      }

    void portaudio_playback_stream_impl::open(uint32_t sample_rate)
    {
      PaStreamParameters params;
      params.device=get_device();
      if(params.device==paNoDevice)
        throw opening_error();
      params.channelCount=1;
      params.sampleFormat=paInt16;
      params.suggestedLatency=Pa_GetDeviceInfo(params.device)->defaultLowOutputLatency;
      params.hostApiSpecificStreamInfo=0;
      if(Pa_OpenStream(&stream,
                       0,
                       &params,
                       sample_rate,
                       static_cast<double>(buffer_size)/1000*sample_rate,
                       paNoFlag,
                       0,
                       0)!=paNoError)
        {
          stream=0;
          throw opening_error();
        }
    }

      bool portaudio_playback_stream_impl::is_open() const
      {
        return (stream!=0);
      }

      void portaudio_playback_stream_impl::close()
      {
        if(stream)
          {
            Pa_CloseStream(stream);
            stream=0;
          }
      }

      void portaudio_playback_stream_impl::write(const short* samples,std::size_t count)
      {
        PaError res=Pa_IsStreamStopped(stream);
        if(res<0)
          throw playback_error();
        else if(res>0)
          {
            if(Pa_StartStream(stream)!=paNoError)
              throw playback_error();
          }
        res=Pa_WriteStream(stream,samples,count);
        if((res!=paNoError)&&(res!=paOutputUnderflowed))
          throw playback_error();
      }

      void portaudio_playback_stream_impl::drain()
      {
        if(Pa_IsStreamStopped(stream)==0)
          if(Pa_StopStream(stream)!=paNoError)
            throw playback_error();
      }

      void portaudio_playback_stream_impl::flush()
      {
        if(Pa_IsStreamStopped(stream)==0)
          if(Pa_AbortStream(stream)!=paNoError)
            throw playback_error();
      }

      void portaudio_playback_stream_impl::idle()
      {
        drain();
      }
    }

    void portaudio_library::initialize()
    {
      if(Pa_Initialize()!=paNoError)
        throw initialization_error();
    }

    void portaudio_library::release()
    {
      Pa_Terminate();
    }

    lib_id portaudio_library::get_id() const
    {
      return lib_portaudio;
    }

    playback_stream_impl* portaudio_library::create_playback_stream_impl(const playback_params& params) const
    {
      return new portaudio_playback_stream_impl(params);
    }

    bool portaudio_library::supports_backend(backend_id id) const
    {
      return ((id==backend_alsa)||
              (id==backend_oss)||
              (id==backend_default));
}
  }
}
