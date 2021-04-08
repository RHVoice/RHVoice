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

#ifndef RHVOICE_SD_STATE_HPP
#define RHVOICE_SD_STATE_HPP

#include <bitset>
#include "core/threading.hpp"

namespace RHVoice
{
  namespace sd
  {
    class state
    {
    public:
      enum flag_t
        {
          speaking,
          stopped,
          pausing,
          paused
        };

      state():
        state_lock(state_mutex)
      {
      }

      bool test(flag_t flag) const
      {
        return flags.test(flag);
      }

      static bool Test(flag_t flag)
      {
        threading::lock lock(state_mutex);
        return flags.test(flag);
      }

      void set(flag_t flag)
      {
        flags.set(flag);
      }

      static void Set(flag_t flag)
      {
        threading::lock lock(state_mutex);
        flags.set(flag);
      }

      void clear(flag_t flag)
      {
        flags.reset(flag);
      }

      static void Clear(flag_t flag)
      {
        threading::lock lock(state_mutex);
        flags.reset(flag);
      }

    private:
      state(const state&);
      state& operator=(const state&);

      static threading::mutex state_mutex;
      static std::bitset<paused+1> flags;

      threading::lock state_lock;
    };
  }
}
#endif
