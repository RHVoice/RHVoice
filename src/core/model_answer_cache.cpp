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

#include <iostream>
#include "HTS_engine.h"
#include "core/model_answer_cache.hpp"

namespace RHVoice
{
  model_answer_cache::model_answer_cache(_HTS_ModelSet* _ms):
    ms(_ms)
  {
    ms->duration[0].answer_cache.impl=this;
    ms->duration[0].answer_cache.index=0;
    for(auto i=0; i<ms->num_streams; ++i)
      {
        ms->stream[0][i].answer_cache.impl=this;
        ms->stream[0][i].answer_cache.index=i+1;
      }
  }

  model_answer_cache::~model_answer_cache()
  {
      {
    ms->duration[0].answer_cache.impl=NULL;
    for(auto i=0; i<ms->num_streams; ++i)
      {
        ms->stream[0][i].answer_cache.impl=NULL;
        ms->stream[0][i].answer_cache.index=0;
      }
  }
  }

  std::size_t model_answer_cache::get(const char* label, std::size_t state, std::size_t index)
  {
    auto it=cache.find(label);
    if(it==cache.end())
      return 0;
    auto v=it->second[state-2][index];
    return v;
  }

  void model_answer_cache::put(const char* label, std::size_t state, std::size_t index, std::size_t value)
  {
    auto it=cache.find(label);
    if(it!=cache.end())
      {
        it->second[state-2][index]=value;
        return;
      }
    segment_t s;
    for(auto& x: s)
      for(auto& y: x)
        y=0;
    s[state-2][index]=value;
    cache.emplace(label, s);
  }
}

  size_t RHVoice_model_answer_cache_get(RHVoice_model_answer_cache_t cache, const char* label, size_t state)
  {
    if(!cache.impl)
      return 0;
    return reinterpret_cast<RHVoice::model_answer_cache*>(cache.impl)->get(label, state, cache.index);
  }

void RHVoice_model_answer_cache_put(RHVoice_model_answer_cache_t cache, const char* label, size_t state, size_t value)
{
  if(!cache.impl)
    return;
  reinterpret_cast<RHVoice::model_answer_cache*>(cache.impl)->put(label, state, cache.index, value);
}
