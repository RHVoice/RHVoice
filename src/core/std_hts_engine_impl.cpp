/* Copyright (C) 2013, 2014  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <cmath>
#include "core/std_hts_engine_impl.hpp"
#include "HTS_engine.h"

namespace RHVoice
{
  std_hts_engine_impl::std_hts_engine_impl(const std::string& voice_path):
    hts_engine_impl("standard",voice_path)
  {
  }

  hts_engine_impl::pointer std_hts_engine_impl::do_create() const
  {
    return pointer(new std_hts_engine_impl(data_path));
  }

  void std_hts_engine_impl::do_initialize()
  {
    engine.reset(new HTS_Engine);
    HTS_Engine_initialize(engine.get(),3);
    engine->audio.data=this;
    HTS_Engine_set_audio_buff_size(engine.get(),fperiod);
    model_file_list dur_files(data_path,"dur");
    if(!HTS_Engine_load_duration_from_fn(engine.get(),&dur_files.pdf,&dur_files.tree,1))
      {
        HTS_Engine_clear(engine.get());
        throw initialization_error();
      }
    model_file_list mgc_files(data_path,"mgc",3);
    if(!HTS_Engine_load_parameter_from_fn(engine.get(),&mgc_files.pdf,&mgc_files.tree,mgc_files.windows,0,false,mgc_files.num_windows,1))
      {
        HTS_Engine_clear(engine.get());
        throw initialization_error();
      }
    model_file_list lf0_files(data_path,"lf0",3);
    if(!HTS_Engine_load_parameter_from_fn(engine.get(),&lf0_files.pdf,&lf0_files.tree,lf0_files.windows,1,true,lf0_files.num_windows,1))
      {
        HTS_Engine_clear(engine.get());
        throw initialization_error();
      }
    model_file_list lpf_files(data_path,"lpf",1);
    if(!HTS_Engine_load_parameter_from_fn(engine.get(),&lpf_files.pdf,&lpf_files.tree,lpf_files.windows,2,false,lpf_files.num_windows,1))
      {
        HTS_Engine_clear(engine.get());
        throw initialization_error();
      }
    HTS_Engine_set_sampling_rate(engine.get(),sample_rate);
    HTS_Engine_set_fperiod(engine.get(),fperiod);
    HTS_Engine_set_alpha(engine.get(),alpha);
    HTS_Engine_set_gamma(engine.get(),0);
    HTS_Engine_set_log_gain(engine.get(),true);
    HTS_Engine_set_beta(engine.get(),beta);
    HTS_Engine_set_msd_threshold(engine.get(),1,msd_threshold);
    HTS_Engine_set_duration_interpolation_weight(engine.get(),0,1.0);
    HTS_Engine_set_parameter_interpolation_weight(engine.get(),0,0,1.0);
    HTS_Engine_set_parameter_interpolation_weight(engine.get(),1,0,1.0);
    HTS_Engine_set_parameter_interpolation_weight(engine.get(),2,0,1.0);
  }

  std_hts_engine_impl::~std_hts_engine_impl()
  {
    if(engine.get()!=0)
    HTS_Engine_clear(engine.get());
  }

  void std_hts_engine_impl::do_synthesize()
  {
    load_labels();
    if(!HTS_Engine_create_sstream(engine.get()))
      throw synthesis_error();
    set_time_info();
    set_pitch();
    if(!HTS_Engine_create_pstream(engine.get()))
      throw synthesis_error();
    if(!HTS_Engine_create_gstream(engine.get()))
      throw synthesis_error();
  }

  void std_hts_engine_impl::do_reset()
  {
    HTS_Engine_set_stop_flag(engine.get(),false);
    HTS_Engine_refresh(engine.get());
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
    HTS_Engine_load_label_from_string_list(engine.get(),&pointers[0],pointers.size());
    if(rate!=1)
      HTS_Label_set_speech_speed(&(engine->label),rate);
  }

  void std_hts_engine_impl::set_time_info()
  {
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
    int n=HTS_Engine_get_nstate(engine.get());
    int i=0;
    for(label_sequence::const_iterator lab_iter=input->lbegin();lab_iter!=input->lend();++lab_iter,i+=n)
      {
        double pitch=lab_iter->get_pitch();
        if(pitch!=1)
          for(int j=0;j<n;++j)
            {
              double f0=std::exp(HTS_Engine_get_state_mean(engine.get(),1,i+j,0))*pitch;
              if(f0<20)
                f0=20;
              HTS_Engine_set_state_mean(engine.get(),1,i+j,0,std::log(f0));
            }
      }
  }
}
