#include "audio.hpp"
#include "playback_stream_impl.hpp"

using namespace RHVoice::audio;


error::error(const std::string& msg):
  exception(msg)
{
}

disallowed_sample_rate::disallowed_sample_rate():
  error("This sample rate is unsupported")
{
}

initialization_error::initialization_error():
  error("Audio library initialization failed")
{
}

is_initialized_error::is_initialized_error():
  error("The playback stream is already initialized")
{
}

opening_error::opening_error():
  error("Unable to open a playback stream")
{
}

is_open_error::is_open_error():
  error("The playback stream is already open")
{
}

is_not_open_error::is_not_open_error():
  error("The playback stream is not open")
{
}


playback_error::playback_error():
  error("Unable to write to a playback stream")
{
}

backend_error::backend_error():
  error("Unsupported audio backend")
{
}

library_error::library_error():
  error("Unsupported audio library")
{
}


playback_stream::playback_stream(const playback_params& params_):
  params(params_),
  impl(nullptr)
{
}

playback_stream::~playback_stream()
{
  if(is_open())
    close();
}

playback_params::playback_params():
  lib(lib_default),
  backend(backend_default),
  sample_rate(24000),
  buffer_size(0)
{
}

bool playback_stream::is_initialized() const
{
  return (impl.get()!=0);
}

void playback_stream::set_lib(lib_id lib)
{
  if(is_initialized())
    throw is_initialized_error();
  else
    params.lib=lib;
}

void playback_stream::set_backend(backend_id backend)
{
  if(is_initialized())
    throw is_initialized_error();
  else
    params.backend=backend;
}

void playback_stream::set_device(const std::string& device)
{
  if(is_initialized())
    throw is_initialized_error();
  else
    params.device=device;
}

void playback_stream::set_server(const std::string& server)
{
  if(is_initialized())
    throw is_initialized_error();
  else
    params.server=server;
}

void playback_stream::set_client_name(const std::string& client_name)
{
  if(is_initialized())
    throw is_initialized_error();
  else
    params.client_name=client_name;
}

unsigned int playback_stream::get_buffer_size() const
{
  return params.buffer_size;
}

void playback_stream::set_buffer_size(unsigned int buffer_size)
{
  if(is_initialized())
    throw is_initialized_error();
  else
    params.buffer_size=buffer_size;
}

int playback_stream::get_sample_rate() const
{
  return params.sample_rate;
}

void playback_stream::reset_params()
{
  if(is_initialized())
    throw is_initialized_error();
  else
    params=playback_params();
}
