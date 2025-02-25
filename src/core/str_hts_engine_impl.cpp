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

#include <sstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include "core/io.hpp"
#include "core/str_hts_engine_impl.hpp"
#include "core/voice.hpp"
#include "core/pitch.hpp"
#include "core/model_answer_cache.hpp"
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
  size_t HTS_PStreamSet_get_vector_length(HTS_PStreamSet * pss, size_t stream_index);
double HTS_PStreamSet_get_parameter(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index, size_t vector_index);
double *HTS_PStreamSet_get_parameter_vector(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index);
HTS_Boolean HTS_PStreamSet_get_msd_flag(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index);
void HTS_SStreamSet_set_mean(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index, double f);
void HTS_SStreamSet_set_vari(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index, double f);
}

namespace RHVoice
{
  str_hts_engine_impl::str_hts_engine_impl(const voice_info& info):
    hts_engine_impl("stream",info)
  {
  }

  hts_engine_impl::pointer str_hts_engine_impl::do_create() const
  {
    return pointer(new str_hts_engine_impl(info));
  }

  void str_hts_engine_impl::maybe_patch_model(HTS_Model* mod, const std::string& patch_path)
  {
    std::ifstream fp;
    try
      {
	io::open_ifstream(fp, patch_path);
      }
    catch(...) {return;}
    std::string line;
    std::size_t nt, nn, i;
    double v;
    while(std::getline(fp, line))
      {
	std::istringstream s(line);
	if(!(s >> nt >> nn >> i >> v))
	  return;
	mod->pdf[nt][nn][i]=v;
      }
  }

  void str_hts_engine_impl::do_initialize()
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
    sample_rate=static_cast<sample_rate_t>(HTS_Engine_get_sampling_frequency(engine.get()));
    std::string bpf_path(path::join(model_path,"bpf.txt"));
    if(bpf_load(&engine->bpf,bpf_path.c_str())==0)
      {
        HTS_Engine_clear(engine.get());
        throw initialization_error();
      }
    HTS_Engine_set_beta(engine.get(),beta);
    HTS_Engine_set_msd_threshold(engine.get(), 1, voicing);
    HTS_Engine_set_audio_buff_size(engine.get(),HTS_Engine_get_fperiod(engine.get()));
    base_frame_shift=HTS_Engine_get_fperiod(engine.get());
        try
          {
            units.reset(new unit_db(model_path, engine.get()));
          }
        catch(...) {}
	maybe_patch_model(&(engine->ms.duration[0]), path::join(model_path, "dur.patch"));
	maybe_patch_model(&(engine->ms.stream[0][1]), path::join(model_path, "lf0.patch"));
  }

  str_hts_engine_impl::~str_hts_engine_impl()
  {
    if(engine.get()!=0)
    HTS_Engine_clear(engine.get());
  }

  void str_hts_engine_impl::do_synthesize()
  {
    const auto& stream_settings=info.get_stream_settings();
    fixed_size=stream_settings.fixed_size;
    view_size=stream_settings.view_size;
    model_answer_cache answer_cache{&engine->ms};
    check_units();
    set_speed();
    queue_labels();
    vocoder.init(engine.get(), &pitch_editor, pitch_shift);
    while(fill_lab_view())
      {
        HTS_Engine_refresh(engine.get());
        if(!HTS_Engine_generate_state_sequence_from_strings(engine.get(),lab_view.data(), lab_view.size(), dur_mods.data()))
      throw synthesis_error();
if(output->is_stopped())
      return;
 if(units_flag)
   copy_units();
    restore_params();
    if(!HTS_Engine_generate_parameter_sequence(engine.get()))
      throw synthesis_error();
if(output->is_stopped())
      return;
    set_frame_ranges();
    set_label_timing();
        save_params();
    vocoder.synth(first_frame, num_frames);
    if(output->is_stopped())
      return;
    const auto drop_size=first_iter?(fixed_size-1):fixed_size;
        if(lab_view.size() <= drop_size)
          break;
        if(drop_size>0)
          {
            lab_view.erase(lab_view.cbegin(), lab_view.cbegin()+drop_size);
            dur_mods.erase(dur_mods.cbegin(), dur_mods.cbegin()+drop_size);
            engine->extra.view_pos_in_utt+=drop_size;
          }
        first_iter=false;
        first_frame_in_utt+=num_frames;
      }
    vocoder.finish();
  }

  void str_hts_engine_impl::do_reset()
  {
    HTS_Engine_set_stop_flag(engine.get(),false);
    HTS_Engine_set_fperiod(engine.get(), base_frame_shift);
    HTS_Engine_refresh(engine.get());
    HTS_Engine_add_half_tone(engine.get(),0);
    while(!lab_queue.empty())
      lab_queue.pop();
    lab_view.clear();
    dur_mods.clear();
    vocoder.clear();
    for(auto& v: par_mem)
      {
        v.hts_view.clear();
        v.data.clear();
      }
    first_iter=true;
    engine->extra.view_pos_in_utt=0;
    num_frames=0;
    first_frame=0;
    first_frame_in_utt=0;
    num_mem_frames=0;
    num_voiced_frames=0;
    num_voiced_mem_frames=0;
      units_flag=false;;
  }

  void str_hts_engine_impl::queue_labels()
  {
    if(input->lbegin()==input->lend())
      throw synthesis_error();
    for(label_sequence::const_iterator it=input->lbegin();it!=input->lend();++it)
      {
        auto& lab=*it;
        lab_queue.push(&lab);
      }
  }

  bool str_hts_engine_impl::fill_lab_view()
  {
    if(lab_queue.empty())
      return false;
    const auto target_size=(quality==quality_max || units_flag)?lab_queue.size():(first_iter?view_size:(view_size+1));
    while(!lab_queue.empty() && lab_view.size() < target_size)
      {
        auto& lab=*(lab_queue.front());
        lab_view.push_back(const_cast<char*>(lab.get_name().c_str()));
        dur_mods.push_back(1);
        if(lab.get_segment().has_feature("dur_mod"))
          dur_mods.back()=lab.get_segment().get("dur_mod").as<double>();
        lab_queue.pop();        
      }
    return true;
  }

  void str_hts_engine_impl::check_units()
  {
    if(!units)
      return;
    auto it1=input->lbegin();
    ++it1;
    if(it1==input->lend())
      return;
    auto it2=input->lend();
    --it2;
    if(it1==it2)
      return;
	for(auto it=it1; it!=it2;++it)
	  {
	    if(!units->has_unit(it->get_name()))
	      return;
}
    units_flag=true;
  }

  void str_hts_engine_impl::copy_units()
  {
    const auto ns=HTS_Engine_get_nstate(engine.get());
    std::size_t i=0;
    for(auto lab=engine->label.head;lab!=nullptr;lab=lab->next, ++i)
      {
	if(!units->has_unit(lab->name))
	  continue;
	const auto& u=units->get_unit(lab->name);
	for(auto s=0;s<ns;++s)
	  {
	    const auto& state=u.states.at(s);
	    const auto k=i*ns+s;
	    if(k>=(ns-1))
	    engine->sss.duration[k]=state.dur_mean;
	    for(auto j=0; j<state.mgc_mean.size();++j)
	      {
      	HTS_SStreamSet_set_mean(&engine->sss, 0, k, j, state.mgc_mean[j]);
		HTS_SStreamSet_set_vari(&engine->sss, 0, k, j, state.mgc_var[j]);
	      }
	  }
      }
    engine->sss.total_frame=0;
    for(auto i=0; i<engine->sss.total_state; ++i)
      engine->sss.total_frame+=engine->sss.duration[i];
  }

  void str_hts_engine_impl::set_frame_ranges()
  {
    first_frame=0;
    num_frames=0;
    num_mem_frames=0;
    num_voiced_frames=0;
    num_voiced_mem_frames=0;
    const auto total_frames=HTS_PStreamSet_get_total_frame(&engine->pss);
    if(first_iter && lab_queue.empty())
      {
        num_frames=total_frames;
        return;
      }
    const auto ns=HTS_Engine_get_nstate(engine.get());
    const auto total_states=HTS_Engine_get_total_state(engine.get());
    const std::size_t skip_states=first_iter?0:ns;
    const auto view_states=total_states-skip_states;
    const auto fixed_states=lab_queue.empty()?view_states:std::min(fixed_size*ns, view_states);
    for(auto i=0;i<skip_states;++i)
      {
        const auto d=HTS_Engine_get_state_duration(engine.get(), i);
        first_frame+=d;
      }
    for(auto i=0;i<fixed_states;++i)
      {
        const auto d=HTS_Engine_get_state_duration(engine.get(), skip_states+i);
        num_frames+=d;
        
      }
    if(lab_queue.empty())
      return;
    const auto last_frame=first_frame+num_frames-1;
    for(auto i=1; i<=ns; ++i)
      {
        const auto d=HTS_Engine_get_state_duration(engine.get(), skip_states+fixed_states-i);
        num_mem_frames+=d;
      }
    for(auto i=0; i<=last_frame;++i)
      {
        if(HTS_PStreamSet_get_msd_flag(&engine->pss, 1, i))
          ++num_voiced_frames;
      }
    for(auto i=0; i<num_mem_frames; ++i)
      {
        if(HTS_PStreamSet_get_msd_flag(&engine->pss, 1, last_frame-i))
          ++num_voiced_mem_frames;
      }
  }

  void str_hts_engine_impl::set_label_timing()
  {
    auto lab=input->lbegin();
    if(engine->extra.view_pos_in_utt>0)
      std::advance(lab, engine->extra.view_pos_in_utt);
    if(!first_iter)
      ++lab;
        const auto ns=HTS_Engine_get_nstate(engine.get());
        const auto fp=HTS_Engine_get_fperiod(engine.get());
        std::size_t fs=first_iter?0:ns;
        auto pos=first_frame_in_utt;
       const auto end_pos=pos+num_frames;
       while(pos<end_pos)
         {
           lab->set_position(pos);
           lab->set_time(pos*fp);
           std::size_t len=0;
           for(auto i=0; i<ns; ++i)
             len+=HTS_Engine_get_state_duration(engine.get(), fs+i);
           lab->set_length(len);
           lab->set_duration(len*fp);
           pos+=len;
           ++lab;
           fs+=ns;
         }
  }

  void str_hts_engine_impl::save_params()
  {
    if(lab_queue.empty())
      return;
    const auto nstr=HTS_Engine_get_nstream(engine.get());
  for(auto i=0; i<nstr; ++i)
    {
      par_mem[i].hts_view.clear();
      par_mem[i].data.clear();
      const auto n=HTS_PStreamSet_get_vector_length(&engine->pss, i);
      const auto ff=(i==1)?(num_voiced_frames-num_voiced_mem_frames):(first_frame+num_frames-num_mem_frames);
      const auto mf=(i==1)?num_voiced_mem_frames:num_mem_frames;
      for(auto j=0; j< mf; ++j)
        {
          const auto v=HTS_PStreamSet_get_parameter_vector(&engine->pss, i, ff+j);
          par_mem[i].data.emplace_back(v, v+n);
        }
      for(auto& x: par_mem[i].data)
        par_mem[i].hts_view.push_back(x.data());
    }   
    }

  void str_hts_engine_impl::restore_params()
  {
    if(first_iter)
      return;
        const auto nstr=HTS_Engine_get_nstream(engine.get());
        for(auto i=0; i<nstr; ++i)
          {
            auto& v=par_mem[i].hts_view;
            if(v.empty())
              continue;
            auto& s=engine->sss.sstream[i];
            s.prev_par=v.data();
            s.num_prev_frames=v.size();
          }
  }

  void str_hts_engine_impl::set_speed()
  {
    if(rate==1)
      return;
    const std::size_t frame_shift=static_cast<std::size_t>(std::round(base_frame_shift/rate));
    HTS_Engine_set_fperiod(engine.get(),frame_shift);
  }

  void str_hts_engine_impl::do_stop()
  {
    HTS_Engine_set_stop_flag(engine.get(),TRUE);
  }

  bool str_hts_engine_impl::supports_quality(quality_t q) const
    {
      if(quality==quality_none)
        return true;
      return (q==quality);
    }
}
