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

#include <cmath>
#include <utility>
#include <iostream>
#include "core/hts_vocoder_wrapper.hpp"
#include "HTS_hidden.h"

namespace RHVoice
{
  hts_vocoder_wrapper::hts_vocoder_wrapper()
  {}

  void hts_vocoder_wrapper::init(_HTS_Engine* e, pitch::editor* ed, double psh)
  {
    vocoder.reset(new _HTS_Vocoder);
    engine=e;
    pitch_editor=ed;
    pitch_shift=psh;
    HTS_Vocoder_initialize(vocoder.get(),
                           HTS_ModelSet_get_vector_length(&engine->ms, 0) - 1,
                           0,
                           0,
                           HTS_Engine_get_sampling_frequency(engine),
                           HTS_Engine_get_fperiod(engine));
}

  void hts_vocoder_wrapper::clear()
  {
    if(vocoder==nullptr)
      return;
    HTS_Vocoder_clear(vocoder.get());
    vocoder.reset();
    engine=nullptr;
    pitch_editor=nullptr;
    count=0;
    pitch_shift=0;
    while(!fq.empty())
      fq.pop();
  }

  hts_vocoder_wrapper::~hts_vocoder_wrapper()
  {
    clear();
  }

  void hts_vocoder_wrapper::synth(std::size_t i, std::size_t n)
  {
    auto* pss=&engine->pss;
    const auto nspec=HTS_PStreamSet_get_vector_length(pss, 0);
    const auto nbap=HTS_PStreamSet_get_vector_length(pss, 2);
    std::size_t l=0;
    for(std::size_t j=0; j<i; ++j)
      {
        if(HTS_PStreamSet_get_msd_flag(pss, 1, j) == TRUE)
          ++l;
      }
    const auto m=i+n;
    for(auto j=i; j<m; ++j)
      {
        frame_t f;
        f.index=count;
        auto spec=HTS_PStreamSet_get_parameter_vector(pss, 0, j);
        f.spec.assign(spec, spec+nspec);
        f.lf0=HTS_NODATA;
        if(HTS_PStreamSet_get_msd_flag(pss, 1, j) == TRUE)
          {
            f.voiced=true;
            f.lf0=HTS_PStreamSet_get_parameter(pss, 1, l, 0);
            ++l;
          }
        if(pitch_editor->has_work())
          {
            if(f.voiced)
              pitch_editor->append(f.lf0);
            else
              pitch_editor->append();
          }
        auto bap=HTS_PStreamSet_get_parameter_vector(pss, 2, j);
        f.bap.assign(bap, bap+nbap);
        for(auto& v: f.bap)
          {
            if(v>0)
              v=0;
            v=std::pow(10.0, v/10.0);
            if(v>1)
              v=1;    
          }
        fq.push(std::move(f));
        ++count;
      }
    do_synth();
  }

  void hts_vocoder_wrapper::finish()
  {
    pitch_editor->finish();
    do_synth();
  }

  void hts_vocoder_wrapper::do_synth()
  {
    const auto nspec=HTS_PStreamSet_get_vector_length(&engine->pss, 0);
    while(!fq.empty())
      {
        if(engine->condition.stop)
          return;
        auto& f=fq.front();
        auto lf0=f.lf0;
        if(pitch_editor->has_work() && f.voiced)
          {
            if(!pitch_editor->has_result(f.index))
              return;
            lf0=pitch_editor->get_result(f.index);
          }
        if(f.voiced)
          lf0+=pitch_shift;
        HTS_Vocoder_synthesize(vocoder.get(),
                               nspec - 1,
                               lf0,
                               f.spec.data(),
                               f.bap.data(),
                               &engine->bpf,
                               engine->condition.alpha,
                               engine->condition.beta,
                               engine->condition.volume,
                               nullptr,
                               &engine->audio);
        fq.pop();
      }
  }
}
