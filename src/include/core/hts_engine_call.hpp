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

#ifndef RHVOICE_HTS_ENGINE_CALL_HPP
#define RHVOICE_HTS_ENGINE_CALL_HPP

#include <string>
#include <queue>
#include "speech_stream.hpp"
#include "exception.hpp"
#include "smart_ptr.hpp"
#include "client.hpp"
#include "utterance.hpp"
#include "item.hpp"
#include "relation.hpp"
#include "hts_engine_pool.hpp"

namespace RHVoice
{
  class hts_synthesis_error: public exception
  {
  public:
    hts_synthesis_error():
      exception("Hts synthesis error")
    {
    }
  };

  class hts_engine_call
  {
    friend void ::HTS_Audio_write(HTS_Audio * audio, short data);
  public:
    hts_engine_call(hts_engine_pool& pool,const utterance& u,client& handler);
    ~hts_engine_call();
    bool execute();

  private:
    hts_engine_call(const hts_engine_call&);
    hts_engine_call& operator=(const hts_engine_call&);

    void write(short sample);
    bool load_labels();
    int get_part_duration(int start_state,int count) const;
    void set_start_offset();
    void queue_events();
    bool fire_events(bool all_remaining=false);
    double calculate_speech_param(double absolute_change,double relative_change,double default_value,double min_value,double max_value) const;
    double calculate_rate() const;
    double calculate_pitch() const;
    double calculate_volume() const;
    void set_pitch();

    class event
    {
    public:
      virtual ~event()
      {
      }

      int get_offset() const
      {
        return offset;
      }

      virtual bool notify(client& c) const=0;

    protected:
      event(int offset_):
      offset(offset_)
      {
      }

    private:
      int offset;
    };

    typedef smart_ptr<event> event_ptr;

    class sentence_event: public event
    {
    protected:
      sentence_event(int offset,const utterance& utt):
        event(offset)
      {
        const relation& token_rel=utt.get_relation("Token");
        position=token_rel.first().get("position").as<std::size_t>();
        length=(token_rel.last().get("position").as<std::size_t>() +token_rel.last().get("length").as<std::size_t>())-position;
      }

      std::size_t position,length;
    };

    class sentence_starts_event: public sentence_event
    {
    public:
      sentence_starts_event(int offset,const utterance& utt):
        sentence_event(offset,utt)
      {
      }

      bool notify(client& c) const
      {
        return c.sentence_starts(position,length);
      }
    };

    class sentence_ends_event: public sentence_event
    {
    public:
      sentence_ends_event(int offset,const utterance& utt):
        sentence_event(offset,utt)
      {
      }

      bool notify(client& c) const
      {
        return c.sentence_ends(position,length);
      }
    };

    class word_event: public event
    {
    protected:
      word_event(int offset,const item& token):
        event(offset)
      {
        position=token.get("position").as<std::size_t>();
        length=token.get("length").as<std::size_t>();
      }

      std::size_t position,length;
    };

    class word_starts_event: public word_event
    {
    public:
      word_starts_event(int offset,const item& token):
        word_event(offset,token)
      {
      }

      bool notify(client& c) const
      {
        return c.word_starts(position,length);
      }
    };

    class word_ends_event: public word_event
    {
    public:
      word_ends_event(int offset,const item& token):
        word_event(offset,token)
      {
      }

      bool notify(client& c) const
      {
        return c.word_ends(position,length);
      }
    };

    class mark_event: public event
    {
    public:
      mark_event(int offset,const std::string& mark):
        event(offset),
        name(mark)
      {
      }

      bool notify(client& c) const
      {
        return c.process_mark(name);
      }

    private:
      std::string name;
    };

    const utterance& utt;
    client& result_handler;
    hts_engine_pool& engine_pool;
    smart_ptr<HTS_Engine> engine;
    int total_samples;
    int buffer_size;
    bool speaking;
    std::queue<event_ptr> events;
    speech_stream stream;
  };
}
#endif
