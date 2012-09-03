/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_SAPI_SPEAKIMPL_HPP
#define RHVOICE_SAPI_SPEAKIMPL_HPP

#include <windows.h>
#include <sapi.h>
#include <sapiddk.h>
#include "core/client.hpp"
#include "core/document.hpp"
#include "tts_base.hpp"

namespace RHVoice
{
  namespace sapi
  {
    class SpeakImpl: public client, public tts_base
    {
    public:
      struct init_params
      {
        const SPVTEXTFRAG* input;
        ISpTTSEngineSite* caller;
        document::init_params document_params;
      };

      SpeakImpl(const init_params& p);
      void operator()();
      event_mask get_supported_events() const;
      bool play_speech(const short* samples,std::size_t count);
      bool process_mark(const std::string& name);
      bool word_starts(std::size_t position,std::size_t length);
      bool sentence_starts(std::size_t position,std::size_t length);

    private:
      SpeakImpl(const SpeakImpl&);
      SpeakImpl& operator=(const SpeakImpl&);

      bool check_actions();
      bool queue_boundary_event(bool is_word,std::size_t position,std::size_t length);


      ISpTTSEngineSite* caller;
      document doc;
      ULONGLONG bytes_written;
    };
  }
}
#endif
