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

#ifndef RHVOICE_AUDIO_LIBAO_HPP
#define RHVOICE_AUDIO_LIBAO_HPP

#include "library.hpp"

namespace RHVoice
{
  namespace audio
  {
    class libao_library: public library
    {
    public:
      lib_id get_id() const;
      bool supports_backend(backend_id id) const;

    private:
      void initialize();
      void release();
      playback_stream_impl* create_playback_stream_impl(const playback_params& params) const;
    };
  }
}
#endif
