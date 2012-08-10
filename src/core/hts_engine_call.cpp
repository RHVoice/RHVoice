/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <list>
#include <algorithm>
#include <cmath>
#include "core/language.hpp"
#include "core/voice.hpp"
#include "core/hts_labeller.hpp"
#include "core/hts_engine_call.hpp"

extern "C"
{
  void HTS_Audio_initialize(HTS_Audio * audio, int sampling_rate, int max_buff_size)
  {
  }

  void HTS_Audio_set_parameter(HTS_Audio * audio, int sampling_rate, int max_buff_size)
  {
  }

  void HTS_Audio_write(HTS_Audio * audio, short data)
  {
    reinterpret_cast<RHVoice::hts_engine_call*>(audio->data)->write(data);
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
  hts_engine_call::hts_engine_call(hts_engine_pool& pool,const utterance& u,client& handler):
    utt(u),
    result_handler(handler),
    engine_pool(pool),
    engine(pool.acquire()),
    total_samples(0),
    buffer_size(20*HTS_Engine_get_fperiod(engine.get())),
    speaking(true),
    stream(HTS_Engine_get_sampling_rate(engine.get()),calculate_rate())
    {
      engine->audio.data=this;
      HTS_Engine_set_volume(engine.get(),calculate_volume());
    }

  hts_engine_call::~hts_engine_call()
  {
    engine->audio.data=0;
    HTS_Engine_set_stop_flag(engine.get(),false);
    HTS_Engine_set_volume(engine.get(),1);
    HTS_Engine_refresh(engine.get());
    engine_pool.release(engine);
  }

  bool hts_engine_call::execute()
  {
    int sample_rate=HTS_Engine_get_sampling_rate(engine.get());
    if(sample_rate!=result_handler.get_sample_rate())
      {
        if(!result_handler.set_sample_rate(sample_rate))
          return false;
      }
    if(!load_labels())
      return true;
    if(!HTS_Engine_create_sstream(engine.get()))
      throw hts_synthesis_error();
    total_samples=HTS_Engine_get_fperiod(engine.get())*get_part_duration(0,HTS_Engine_get_total_state(engine.get()));
    set_start_offset();
    set_pitch();
    queue_events();
    if(!HTS_Engine_create_pstream(engine.get()))
      throw hts_synthesis_error();
    if(!fire_events())
      return false;
    if(!HTS_Engine_create_gstream(engine.get()))
      throw hts_synthesis_error();
    return speaking;
  }

  bool hts_engine_call::load_labels()
  {
    int count=0;
    const hts_labeller& labeller=utt.get_language().get_hts_labeller();
    const relation& seg_rel=utt.get_relation("Segment");
    std::list<std::string> labels;
    for(relation::const_iterator seg_iter(seg_rel.begin());seg_iter!=seg_rel.end();++seg_iter,++count)
      {
        labels.push_back(labeller.eval_segment_label(*seg_iter));
      }
    if(labels.empty())
      return false;
    std::vector<char*> pointers;
    pointers.reserve(count);
    for(std::list<std::string>::const_iterator it(labels.begin());it!=labels.end();++it)
      {
        pointers.push_back(const_cast<char*>(it->c_str()));
      }
    HTS_Engine_load_label_from_string_list(engine.get(),&pointers[0],count);
    return true;
  }

  int hts_engine_call::get_part_duration(int start_state,int count) const
  {
    int end_state=start_state+count;
    int result=0;
    for(int i=start_state;i<end_state;++i)
      {
        result+=HTS_Engine_get_state_duration(const_cast<HTS_Engine*>(engine.get()),i);
      }
    return result;
  }

  void hts_engine_call::set_start_offset()
  {
    int nstates=HTS_Engine_get_nstate(engine.get());
    int fperiod=HTS_Engine_get_fperiod(engine.get());
    int start_offset=fperiod*get_part_duration(0,nstates);
    stream.set_start_offset(start_offset);
  }

  bool hts_engine_call::fire_events(bool all_remaining)
  {
    bool continue_flag=true;
    while((!events.empty())&&
          continue_flag&&
          (all_remaining||(events.front()->get_offset()==stream.get_offset())))
      {
        continue_flag=events.front()->notify(result_handler);
        events.pop();
      }
    return continue_flag;
  }

  void hts_engine_call::queue_events()
  {
    event_mask mask=result_handler.get_supported_events();
    if(mask==0)
      return;
    int nstates=HTS_Engine_get_nstate(engine.get());
    int fperiod=HTS_Engine_get_fperiod(engine.get());
    int offset=0;
    relation::const_iterator prev_seg=utt.get_relation("Segment").begin();
    relation::const_iterator cur_seg;
    int prev_state=0;
    int cur_state;
    const relation& event_rel=utt.get_relation("Event");
    if(mask&event_sentence_starts)
      events.push(event_ptr(new sentence_starts_event(offset,utt)));
    for(relation::const_iterator it(event_rel.begin());it!=event_rel.end();++it)
      {
        if(it->in("Token"))
          {
            if(it->as("Token").has_children())
              {
                cur_seg=it->as("Token").first_child().as("Transcription").first_child().as("Segment").get_iterator();
                cur_state=prev_state+std::distance(prev_seg,cur_seg)*nstates;
                if(cur_state!=prev_state)
                  offset+=get_part_duration(prev_state,cur_state-prev_state)*fperiod;
                if(mask&event_word_starts)
                  events.push(event_ptr(new word_starts_event(offset,*it)));
                prev_seg=cur_seg;
                prev_state=cur_state;
                cur_seg=++(it->as("Token").last_child().as("Transcription").last_child().as("Segment").get_iterator());
                cur_state=prev_state+std::distance(prev_seg,cur_seg)*nstates;
                offset+=get_part_duration(prev_state,cur_state-prev_state)*fperiod;
                if(mask&event_word_ends)
                  events.push(event_ptr(new word_ends_event(offset,*it)));
                prev_seg=cur_seg;
                prev_state=cur_state;
              }
          }
        else
          {
            const value& mark_val=it->get("mark",true);
            if(!mark_val.empty())
              {
                if(mask&event_mark)
                  events.push(event_ptr(new mark_event(offset,mark_val.as<std::string>())));
              }
            else
              {
                const value& audio_val=it->get("audio",true);
                if(!audio_val.empty())
                  {
                    if(mask&event_audio)
                      events.push(event_ptr(new audio_event(offset,audio_val.as<std::string>())));
                  }
              }
          }
      }
    if(mask&event_sentence_ends)
      events.push(event_ptr(new sentence_ends_event(offset,utt)));
  }

  void hts_engine_call::write(short sample)
  {
    if(!speaking)
      return;
    try
      {
        stream.write(sample);
        speech_stream::buffer_type buffer;
        bool buffer_filled=false;
        int next_event_offset=events.empty()?total_samples:events.front()->get_offset();
        int in_samples_to_next_event=next_event_offset-stream.get_offset();
        if(in_samples_to_next_event==0)
          {
            if(next_event_offset==total_samples)
              stream.flush();
            buffer_filled=stream.read(buffer,0);
          }
        else
          {
            int out_samples_to_next_event=in_samples_to_next_event/stream.get_rate();
            if(out_samples_to_next_event>=buffer_size)
              buffer_filled=stream.read(buffer,buffer_size);
          }
        if(buffer_filled)
          speaking=result_handler.play_speech(&buffer[0],buffer.size());
        if(speaking)
          speaking=fire_events();
      }
    catch(std::exception& e)
      {
        speaking=false;
      }
    if(!speaking)
      HTS_Engine_set_stop_flag(engine.get(),true);
  }

  double hts_engine_call::calculate_speech_param(double absolute_change,double relative_change,double default_value,double min_value,double max_value) const
  {
    if(!((min_value<=default_value)&&(default_value<=max_value)))
      return 1;
    double result=default_value;
    if(absolute_change>0)
      {
        if(absolute_change>=1)
          result=max_value;
        else
          result+=absolute_change*(max_value-default_value);
      }
    else if(absolute_change<0)
      {
        if(absolute_change<=-1)
          result=min_value;
        else
          result+=absolute_change*(default_value-min_value);
      }
    result*=relative_change;
    if(result<min_value)
      result=min_value;
    else if(result>max_value)
      result=max_value;
    return result;
  }

  double hts_engine_call::calculate_volume() const
  {
    const voice_params&voice_settings=utt.get_voice().get_info().settings;
    double volume=calculate_speech_param(utt.get_absolute_volume(),
                                    utt.get_relative_volume(),
                                    voice_settings.default_volume,
                                    voice_settings.min_volume,
                                    voice_settings.max_volume);
    return volume;
  }

  double hts_engine_call::calculate_rate() const
  {
    const voice_params&voice_settings=utt.get_voice().get_info().settings;
    double rate=calculate_speech_param(utt.get_absolute_rate(),
                                  utt.get_relative_rate(),
                                  voice_settings.default_rate,
                                  voice_settings.min_rate,
                                  voice_settings.max_rate);
    return rate;
  }

  double hts_engine_call::calculate_pitch() const
  {
    const voice_params&voice_settings=utt.get_voice().get_info().settings;
    double pitch=calculate_speech_param(utt.get_absolute_pitch(),
                                   utt.get_relative_pitch(),
                                   voice_settings.default_pitch,
                                   voice_settings.min_pitch,
                                   voice_settings.max_pitch);
    return pitch;
  }

  void hts_engine_call::set_pitch()
  {
    double pitch=calculate_pitch();
    for(int i=0;i<HTS_SStreamSet_get_total_state(&engine->sss);++i)
      {
        double f0=std::exp(HTS_SStreamSet_get_mean(&engine->sss,1,i,0));
        f0*=pitch;
        if(f0<10)
          f0=10;
        HTS_SStreamSet_set_mean(&engine->sss,1,i,0,std::log(f0));
      }
  }
}
