/* Copyright (C) 2013, 2014, 2018, 2019, 2022  Olga Yakovleva <olga@rhvoice.prg> */ 

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

#include <iostream>
#include "core/str.hpp"
#include "core/path.hpp"
#include "core/config.hpp"
#include "core/voice.hpp"
#include "core/hts_engine_impl.hpp"

namespace RHVoice
{
  hts_engine_impl::hts_engine_impl(const std::string& impl_name,const voice_info& info_):
    info(info_),
    data_path(info_.get_data_path()),
    beta("beta",0.4,-0.8,0.8),
    gain("gain",1.0,0.5,2.0),
    native_rate("rate", 1.0, 0.5, 2.0),
    voicing("voicing", 0.5, 0, 1.0),
    quality(quality_none),
    int_key("key",200,50,500),
    emph_shift("emph_shift",0,-12,12),
    base_pitch_shift("pitch_shift",0,-12,12),
    input(0),
    output(0),
    rate(1.0),
    pitch_shift(0),
    name(impl_name)
  {
    load_configs();
  }

  void hts_engine_impl::synthesize()
  {
    if(input->lbegin()!=input->lend())
      do_synthesize();
    if(!output->is_stopped())
      output->finish();
  }

  void hts_engine_impl::reset()
  {
    if(input->lbegin()!=input->lend())
      do_reset();
    output=0;
    input=0;
    rate=1.0;
    pitch_shift=0;
    pitch_editor.reset();
    if(eq)
      eq->reset();
  }

  void hts_engine_impl::load_configs()
  {
    config cfg2;
    cfg2.register_setting(beta);
    cfg2.register_setting(voicing);
    cfg2.register_setting(gain);
    cfg2.register_setting(native_rate);
    cfg2.register_setting(int_key);
    cfg2.register_setting(emph_shift);
    cfg2.register_setting(base_pitch_shift);
    cfg2.load(path::join(data_path,"voice.params"));
    if(int_key.is_set())
      pitch_editor.set_key(int_key);
  }

  hts_engine_impl::pointer hts_engine_impl::create(quality_t q) const
  {
    pointer p=do_create();
    p->set_quality(q);
    p->load_configs();
    p->do_initialize();
    if(p->quality>=quality_std)
      {
        std::string eq_path(path::join(p->model_path, "eq.txt"));
        try
          {
            p->eq.reset(new equalizer(eq_path));
          }
        catch(...) {}
      }
    return p;
  }

  void hts_engine_impl::on_new_sample(short sample)
  {
    if(output->is_stopped())
      {
        do_stop();
        return;
      }
    double s=(sample/32768.0);
    if(eq)
      s=eq->apply(s);
    try
      {
        output->process(&s,1);
      }
    catch(...)
      {
        output->stop();
      }
    if(output->is_stopped())
      do_stop();
  }

  sample_rate_t hts_engine_impl::get_sample_rate_for_quality(quality_t q) const
  {
    if(info.get_format()==3)
      return sample_rate_24k;
    switch(q)
      {
      case quality_min:
        return sample_rate_16k;
      default:
        return sample_rate_24k;
}
}

  void hts_engine_impl::set_quality(quality_t q)
  {
    quality=q;
    if(info.get_format()==3)
      model_path=data_path;
    else
      model_path=path::join(data_path,str::to_string(get_sample_rate_for_quality(q)));
}
}
