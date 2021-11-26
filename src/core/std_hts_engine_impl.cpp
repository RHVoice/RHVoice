/* Copyright (C) 2013, 2014, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <cmath>
#include <cstdlib>
#include "core/io.hpp"
#include "core/std_hts_engine_impl.hpp"
#include "core/voice.hpp"
#include "core/pitch.hpp"
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

  size_t HTS_PStreamSet_get_total_frame(HTS_PStreamSet * pss);
double HTS_PStreamSet_get_parameter(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index, size_t vector_index);
double *HTS_PStreamSet_get_parameter_vector(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index);
HTS_Boolean HTS_PStreamSet_get_msd_flag(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index);
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
    base_frame_shift=HTS_Engine_get_fperiod(engine.get());
  }

  std_hts_engine_impl::~std_hts_engine_impl()
  {
    if(engine.get()!=0)
    HTS_Engine_clear(engine.get());
  }

  void std_hts_engine_impl::do_synthesize()
  {
    set_speed();
    load_labels();
    set_time_info();
    if(!HTS_Engine_generate_parameter_sequence(engine.get()))
      throw synthesis_error();
    edit_pitch();
    if(!HTS_Engine_generate_sample_sequence(engine.get()))
      throw synthesis_error();
    output_debug_info();
  }

  void std_hts_engine_impl::do_reset()
  {
    HTS_Engine_set_stop_flag(engine.get(),false);
    HTS_Engine_set_fperiod(engine.get(), base_frame_shift);
    HTS_Engine_refresh(engine.get());
    HTS_Engine_add_half_tone(engine.get(),0);
  }

  void std_hts_engine_impl::load_labels()
  {
    if(input->lbegin()==input->lend())
      throw synthesis_error();
    std::vector<char*> pointers;
    std::vector<double> dur_mods;
    for(label_sequence::const_iterator it=input->lbegin();it!=input->lend();++it)
      {
        pointers.push_back(const_cast<char*>(it->get_name().c_str()));
        dur_mods.push_back(1);
        if(it->get_segment().has_feature("dur_mod"))
          dur_mods.back()=it->get_segment().get("dur_mod").as<double>();
      }
    if(!HTS_Engine_generate_state_sequence_from_strings(engine.get(),&pointers[0],pointers.size(),&dur_mods[0]))
      throw synthesis_error();
  }

  void std_hts_engine_impl::set_time_info()
  {
    int fperiod=HTS_Engine_get_fperiod(engine.get());
    int n=HTS_Engine_get_nstate(engine.get());
    int pos=0;
    int len=0;
    int i=0;
    for(label_sequence::iterator lab_iter=input->lbegin();lab_iter!=input->lend();++lab_iter,++i)
      {
        lab_iter->set_position(pos);
        lab_iter->set_time(pos*fperiod);
        len=0;
        for(int j=0;j<n;++j)
          len+=HTS_Engine_get_state_duration(engine.get(),i*n+j);
        lab_iter->set_length(len);
        lab_iter->set_duration(len*fperiod);
        pos+=len;
      }
  }

  void std_hts_engine_impl::set_speed()
  {
    if(rate==1)
      return;
    const std::size_t frame_shift=static_cast<std::size_t>(std::round(base_frame_shift/rate));
    HTS_Engine_set_fperiod(engine.get(),frame_shift);
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

  void std_hts_engine_impl::edit_pitch()
  {
    if(!pitch_editor.has_work()&&pitch_shift==0)
      return;
    std::size_t n=HTS_PStreamSet_get_total_frame(&engine->pss);
    std::size_t m=0;
    if(pitch_editor.has_work())
      {
        for(std::size_t i=0;i<n;++i)
          {
            if(HTS_PStreamSet_get_msd_flag(&engine->pss,1,i))
              {
                pitch_editor.append(HTS_PStreamSet_get_parameter(&engine->pss,1,m,0));
                ++m;
              }
            else
              pitch_editor.append();
          }
        pitch_editor.finish();
        m=0;
      }
    for(std::size_t i=0;i<n;++i)
      {
        if(HTS_PStreamSet_get_msd_flag(&engine->pss,1,i))
          {
            double v=pitch_editor.has_work()?pitch_editor.get_result(i):HTS_PStreamSet_get_parameter(&engine->pss,1,m,0);
            v+=pitch_shift;
            HTS_PStreamSet_get_parameter_vector(&engine->pss,1,m)[0]=v;
            ++m;
          }
}
}

  void std_hts_engine_impl::output_debug_info()
  {
    if(const char* var=std::getenv("RHVOICE_DEBUG_HTS_FILE"))
      {
        io::file_handle ifh(io::open_file(var+std::string(".info"), "wt"));
        HTS_Engine_save_information(engine.get(), ifh.get());
        io::file_handle mfh(io::open_file(var+std::string(".mgc"), "wt"));
        HTS_Engine_save_generated_parameter(engine.get(), 0, mfh.get());
        io::file_handle ffh(io::open_file(var+std::string(".lf0"), "wt"));
        HTS_Engine_save_generated_parameter(engine.get(), 1, ffh.get());
                                                          io::file_handle lfh(io::open_file(var+std::string(".lab"), "wt"));
                                                                              HTS_Engine_save_label(engine.get(), lfh.get());
      }
  }
}
