/* Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_MODEL_ANSWER_CACHE_HPP
#define RHVOICE_MODEL_ANSWER_CACHE_HPP

#include <string>
#include <map>
#include <array>

extern "C"
{
  struct _HTS_ModelSet;
}

namespace RHVoice {
  class model_answer_cache
  {
  public:
    model_answer_cache(_HTS_ModelSet*);
~model_answer_cache();
    std::size_t get(const char* label, std::size_t state, std::size_t index);
    void put(const char* label, std::size_t state, std::size_t index, std::size_t value);

  private:

    using state_t=std::array<std::size_t, 4>;
    using segment_t=std::array<state_t, 5>;
    using cache_t=std::map<std::string, segment_t>;

    cache_t cache;
    _HTS_ModelSet *ms;
  };
}
#endif
