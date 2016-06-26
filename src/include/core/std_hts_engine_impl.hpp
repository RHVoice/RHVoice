/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_STD_HTS_ENGINE_IMPL_HPP
#define RHVOICE_STD_HTS_ENGINE_IMPL_HPP

#include <memory>
#include "hts_engine_impl.hpp"

struct _HTS_Engine;

namespace RHVoice
{
  class std_hts_engine_impl: public hts_engine_impl
  {
  public:
    explicit std_hts_engine_impl(const std::string& voice_path);
    ~std_hts_engine_impl();

  private:
    pointer do_create() const;
    void do_initialize();
    void do_reset();
    void do_synthesize();
    void do_stop();
    void load_labels();
    void set_time_info();
    void set_pitch();
    void set_speed();

    std::auto_ptr<_HTS_Engine> engine;
  };
}
#endif
