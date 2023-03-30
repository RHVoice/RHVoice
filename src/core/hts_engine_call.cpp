/* Copyright (C) 2012, 2013, 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <memory>
#include <new>
#include <algorithm>
#include <queue>
#include <cmath>
#ifndef ENABLE_SONIC
#include "core/config.h"
#endif
#if ENABLE_SONIC
#include "sonic.h"
#endif
#include "core/language.hpp"
#include "core/voice.hpp"
#include "core/tone.hpp"
#include "core/limiter.hpp"
#include "core/hts_engine_call.hpp"

namespace RHVoice
{
  namespace
  {
    class sink: public speech_processor
    {
    private:
      void on_input();
      bool accepts_insertions() const
      {
        return true;
      }

      std::size_t get_desired_input_size() const
      {
        return (player->get_audio_buffer_size()/1000.0*sample_rate);
      }

      std::vector<short> samples;
    };

    void sink::on_input()
    {
      samples.clear();
      for(std::size_t i=0;i<input.size();++i)
        {
          sample_type s=input[i]*32768;
          s=std::max<sample_type>(-32768,std::min<sample_type>(32767,s));
          samples.push_back(static_cast<short>(s));
        }
      bool should_continue=player->play_speech(&samples[0],samples.size());
      if(!should_continue)
        stop();
    }

    class notifier: public speech_processor
    {
    public:
      notifier(event_sequence::const_iterator efirst,event_sequence::const_iterator elast):
        enext(efirst),
        eend(elast),
        time(0)
      {
      }

    private:
      void on_input();
      void on_finished();

      event_sequence::const_iterator enext,eend;
      int time;
    };

    void notifier::on_input()
    {
      while((enext!=eend)&&((*enext)->get_time()!=-1)&&((*enext)->get_time()<=time))
        {
          if(!(*enext)->notify(*player))
            {
              stop();
              return;
            }
          else
            ++enext;
        }
      output=input;
      time+=output.size();
    }

    void notifier::on_finished()
    {
      while(enext!=eend)
        {
          if(!(*enext)->notify(*player))
            {
              stop();
              break;
            }
          else
            ++enext;
        }
    }
  }

    class trim: public speech_processor
    {
    public:
      trim(label_sequence::const_iterator lstart,label_sequence::const_iterator lend):
        lfirst(++lstart),
        llast(--lend),
        time(0)
      {
      }

    private:
      void on_input();

      label_sequence::const_iterator lfirst,llast;
      int time;
    };

  void trim::on_input()
  {
    int prev_time=time;
    time+=input.size();
    if((lfirst->get_time()==-1)||(prev_time<lfirst->get_time()))
      return;
    if((llast->get_time()!=-1)&&(time>llast->get_time()+0.3*sample_rate))
      return;
    output=input;
  }

  class volume_controller: public speech_processor
  {
  public:
    explicit volume_controller(double volume_):
      volume(volume_)
    {
    }

  private:
    void on_input();

    bool accepts_insertions() const
    {
      return true;
    }

    double volume;
  };

  void volume_controller::on_input()
  {
    for(std::size_t i=0;i<input.size();++i)
      output.push_back(input[i]*volume);
  }

#if ENABLE_SONIC
  class rate_controller: public speech_processor
  {
  public:
    explicit rate_controller(double rate_):
      rate(rate_),
      stream(0)
    {
    }

    ~rate_controller()
    {
      if(stream)
        sonicDestroyStream(stream);
    }

  private:
    void do_initialize();
    void on_input();
    void on_end_of_input();
    void on_output();

    double rate;
    sonicStream stream;
    std::vector<float> samples;
  };

  void rate_controller::do_initialize()
  {
    stream=sonicCreateStream(sample_rate,1);
    if(stream==0)
      throw std::bad_alloc();
    sonicSetSpeed(stream,rate);
  }

  void rate_controller::on_input()
  {
    samples.assign(input.begin(),input.end());
    if(sonicWriteFloatToStream(stream,&samples[0],samples.size())==0)
      throw std::bad_alloc();
  }

  void rate_controller::on_end_of_input()
  {
    sonicFlushStream(stream);
  }

  void rate_controller::on_output()
  {
    int n=sonicSamplesAvailable(stream);
    if(n>0)
      {
        if(n>samples.size())
          samples.resize(n,0);
        sonicReadFloatFromStream(stream,&samples[0],n);
        output.assign(samples.begin(),samples.begin()+n);
      }
  }
#endif

  class sound_icon_inserter: public speech_processor
  {
  public:
    sound_icon_inserter(label_sequence::const_iterator lstart,label_sequence::const_iterator lend);

    ~sound_icon_inserter()
    {
      delete icon;
    }

    bool empty() const
    {
      return points.empty();
    }

  private:
    void do_initialize();
    void on_input();

    std::queue<label_sequence::const_iterator> points;
    int time;
    tone* icon;
  };

  sound_icon_inserter::sound_icon_inserter(label_sequence::const_iterator lstart,label_sequence::const_iterator lend):
    time(0),
    icon(0)
  {
    for(label_sequence::const_iterator lab_iter=lstart;lab_iter!=lend;++lab_iter)
      {
        if(lab_iter->is_marked_by_sound_icon())
          points.push(lab_iter);
      }
  }

  void sound_icon_inserter::do_initialize()
  {
    icon=new tone(sample_rate,2000,0.05);
  }

  void sound_icon_inserter::on_input()
  {
    if(!points.empty())
      {
        int next_time=points.front()->get_time();
        if((next_time!=-1)&&(next_time<=time))
          {
            points.pop();
            insertion=(*icon)();
          }
      }
    output=input;
    time+=input.size();
  }

  hts_engine_call::hts_engine_call(hts_engine_pool& pool,const utterance& u,client& player_):
    utt(u),
    player(player_),
    engine_pool(pool),
    engine_impl(pool.acquire(utt.get_quality()))
  {
  }

  hts_engine_call::~hts_engine_call()
  {
    engine_impl->reset();
    engine_pool.release(engine_impl);
  }

  bool hts_engine_call::execute()
  {
    set_input();
    set_output();
    engine_impl->synthesize();
    return !output.is_stopped();
  }

  void hts_engine_call::set_input()
  {
    event_mask events_of_interest=player.get_supported_events();
    const relation& event_rel=utt.get_relation("Event");
    const relation& seg_rel=utt.get_relation("Segment");
    const relation& tokstruct_rel=utt.get_relation("TokStructure");
    relation::const_iterator seg_start=seg_rel.begin();
    // if(!seg_rel.empty())
    //   ++seg_start;
    relation::const_iterator seg_end=seg_start;
    if((!tokstruct_rel.empty())&&(events_of_interest&event_sentence_starts))
      input.add_event<sentence_starts_event>(utt);
    for(relation::const_iterator event_iter=event_rel.begin();event_iter!=event_rel.end();++event_iter)
      {
        if(event_iter->in("Token"))
          {
            const item& token=event_iter->as("Token");
            if(token.has_children())
              {
                seg_end=token.first_child().as("Transcription").first_child().as("Segment").get_iterator();
                add_labels(seg_start,seg_end);
                seg_start=seg_end;
              }
            if(events_of_interest&event_word_starts)
              input.add_event<word_starts_event>(token);
            if(token.has_children())
              {
                seg_end=++(token.last_child().as("Transcription").last_child().as("Segment").get_iterator());
                add_labels(seg_start,seg_end);
                seg_start=seg_end;
              }
            if(events_of_interest&event_word_ends)
              input.add_event<word_ends_event>(token);
            if(token.has_children()&&seg_end!=seg_rel.end()&&seg_end->get("name").as<std::string>()=="pau")
              {
                add_label(*seg_end);
                ++seg_end;
                seg_start=seg_end;
              }
          }
        else if(events_of_interest&event_mark)
          {
            const value& v=event_iter->get("mark",true);
            if(!v.empty())
              input.add_event<mark_event>(v.as<std::string>());
          }
        else if(events_of_interest&event_audio)
          {
            const value& v=event_iter->get("audio",true);
            if(!v.empty())
              input.add_event<audio_event>(v.as<std::string>());
          }
      }
    add_labels(seg_start,seg_rel.end());
    if((!tokstruct_rel.empty())&&(events_of_interest&event_sentence_ends))
      input.add_event<sentence_ends_event>(utt);
    engine_impl->set_input(input);
  }

  void hts_engine_call::add_label(const item& seg)
  {
    hts_label& lab=input.add_label(seg);
  }

  void hts_engine_call::set_output()
  {
    if(!player.configure(engine_impl->get_sample_rate()))
      throw client_error("Cannot configure player");
    output.set_client(player);
    output.set_sample_rate(engine_impl->get_sample_rate());
    if(input.ebegin()!=input.eend())
      {
        notifier* n=new notifier(input.ebegin(),input.eend());
        output.append(n);
      }
    if(input.lbegin()!=input.lend())
      {
        sound_icon_inserter* sii=new sound_icon_inserter(input.lbegin(),input.lend());
        if(sii->empty())
          delete sii;
        else
          output.append(sii);
        // trim* t=new trim(input.lbegin(),input.lend());
        // output.append(t);
        double rate=input.lbegin()->get_rate()*engine_impl->get_native_rate();
        if(rate!=1)
          {
            #if ENABLE_SONIC
            if(rate<utt.get_voice().get_info().settings.min_sonic_rate)
              engine_impl->set_rate(rate);
            else
              {
                rate_controller* rc=new rate_controller(rate);
                output.append(rc);
              }
            #else
            engine_impl->set_rate(rate);
            #endif
          }
        double volume=input.lbegin()->get_volume()*engine_impl->get_gain();
        if(volume>1 || engine_impl->uses_eq())
          {
            limiter* lm=new limiter(std::max(volume, 1.0));
            output.append(lm);
          }
        if(volume!=1)
          {
            volume_controller* vc=new volume_controller(volume);
            output.append(vc);
          }
        sink* s=new sink;
        output.append(s);
      }
    engine_impl->set_output(output);
  }
}
