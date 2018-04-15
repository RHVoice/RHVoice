/* Copyright (C) 2013, 2014, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <cmath>
#include "core/std_hts_engine_impl.hpp"
#include "core/voice.hpp"
#include "HTS_engine.h"

extern "C"
{
  void HTS_Audio_initialize(HTS_Audio * audio, int sampling_rate, int max_buff_size)
  {
  }

  void HTS_Audio_set_parameter(HTS_Audio * audio, int sampling_rate, int max_buff_size)
  {
  }

  void HTS_Audio_write(HTS_Audio * audio, short sample)
  {
    static_cast<RHVoice::hts_engine_impl*>(audio->audio_interface)->on_new_sample(sample);
  }

  void HTS_Audio_flush(HTS_Audio * audio)
  {
  }

  void HTS_Audio_clear(HTS_Audio * audio)
  {
  }
}

namespace RHVoice
{
  std_hts_engine_impl::std_hts_engine_impl(const voice_info& info):
    hts_engine_impl("standard",info)
  {
  }

  hts_engine_impl::pointer std_hts_engine_impl::do_create() const
  {
    return pointer(new std_hts_engine_impl(info));
  }

  void std_hts_engine_impl::do_initialize()
  {
    engine.reset(new HTS_Engine);
    HTS_Engine_initialize(engine.get());
    engine->audio.audio_interface=this;
    std::string voice_path(path::join(model_path,"voice.data"));
    char* c_voice_path=const_cast<char*>(voice_path.c_str());
    if(!HTS_Engine_load(engine.get(),&c_voice_path,1))
      {
        HTS_Engine_clear(engine.get());
        throw initialization_error();
      }
    std::string bpf_path(path::join(model_path,"bpf.txt"));
    if(bpf_load(&engine->bpf,bpf_path.c_str())==0)
      {
        HTS_Engine_clear(engine.get());
        throw initialization_error();
      }
    HTS_Engine_set_beta(engine.get(),beta);
    HTS_Engine_set_audio_buff_size(engine.get(),HTS_Engine_get_fperiod(engine.get()));
  }

  std_hts_engine_impl::~std_hts_engine_impl()
  {
    if(engine.get()!=0)
    HTS_Engine_clear(engine.get());
  }

  void std_hts_engine_impl::do_synthesize()
  {
    set_speed();
    set_pitch();
    load_labels();
    set_time_info();
    if(!HTS_Engine_generate_parameter_sequence(engine.get()))
      throw synthesis_error();
    if(!HTS_Engine_generate_sample_sequence(engine.get()))
      throw synthesis_error();
  }

  void std_hts_engine_impl::do_reset()
  {
    HTS_Engine_set_stop_flag(engine.get(),false);
    HTS_Engine_refresh(engine.get());
    HTS_Engine_add_half_tone(engine.get(),0);
  }

  void std_hts_engine_impl::load_labels()
  {
    if(input->lbegin()==input->lend())
      throw synthesis_error();
    std::vector<char*> pointers;
    for(label_sequence::const_iterator it=input->lbegin();it!=input->lend();++it)
      {
        pointers.push_back(const_cast<char*>(it->get_name().c_str()));
      }
    if(!HTS_Engine_generate_state_sequence_from_strings(engine.get(),&pointers[0],pointers.size()))
      throw synthesis_error();
  }

  void std_hts_engine_impl::set_time_info()
  {
    int fperiod=HTS_Engine_get_fperiod(engine.get());
    int n=HTS_Engine_get_nstate(engine.get());
    int time=0;
    int dur=0;
    int i=0;
    for(label_sequence::iterator lab_iter=input->lbegin();lab_iter!=input->lend();++lab_iter,++i)
      {
        lab_iter->set_time(time);
        dur=0;
        for(int j=0;j<n;++j)
          dur+=HTS_Engine_get_state_duration(engine.get(),i*n+j)*fperiod;
        lab_iter->set_duration(dur);
        time+=dur;
      }
  }

  void std_hts_engine_impl::set_pitch()
  {
    if(input->lbegin()==input->lend())
      return;
    double factor=input->lbegin()->get_pitch();
    if(factor==1)
      return;
    double shift=std::log(factor)*12;
    HTS_Engine_add_half_tone(engine.get(),shift);
  }

  void std_hts_engine_impl::set_speed()
  {
    if(rate==1)
      return;
    HTS_Engine_set_speed(engine.get(),rate);
  }

  void std_hts_engine_impl::do_stop()
  {
    HTS_Engine_set_stop_flag(engine.get(),TRUE);
  }

  bool std_hts_engine_impl::supports_quality(quality_t q) const
    {
      if(q<quality_max)
        return false;
      return true;
}

}
