/* Copyright (C) 2012, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_SAPI_TTS_BASE_HPP
#define RHVOICE_SAPI_TTS_BASE_HPP

#include "core/threading.hpp"
#include "core/engine.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class tts_base
    {
    protected:
      tts_base()
      {
      }

      ~tts_base()
      {
      }

      std::shared_ptr<engine> get_engine();

    private:

      std::vector<std::string> get_resource_paths(int id, std::string name) const;
      std::vector<std::string> get_resource_paths() const;

      static threading::mutex engine_mutex;
      static std::shared_ptr<engine> engine_ptr;
    };
  }
}
#endif
