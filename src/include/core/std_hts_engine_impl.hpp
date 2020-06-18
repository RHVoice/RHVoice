/* Copyright (C) 2013, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
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
#include "quality_setting.hpp"

struct _HTS_Engine;

namespace RHVoice
{
  class voice_info;

  class std_hts_engine_impl: public hts_engine_impl
  {
  public:
    explicit std_hts_engine_impl(const voice_info& info);
    ~std_hts_engine_impl();

    virtual bool supports_quality(quality_t q) const;

  private:
    pointer do_create() const;
    void do_initialize();
    void do_reset();
    void do_synthesize();
    void do_stop();
    void load_labels();
    void set_time_info();
    void set_speed();
    void edit_pitch();

    std::unique_ptr<_HTS_Engine> engine;
  };
}
#endif
