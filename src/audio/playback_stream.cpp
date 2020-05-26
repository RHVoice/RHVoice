#include "audio.hpp"
#include "playback_stream_impl.hpp"

using namespace RHVoice::audio;

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

