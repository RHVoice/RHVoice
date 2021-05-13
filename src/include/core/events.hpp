/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_EVENTS_HPP
#define RHVOICE_EVENTS_HPP

#include <list>

#include "client.hpp"
#include "hts_label.hpp"
#include "utterance.hpp"
#include "relation.hpp"
#include "item.hpp"

namespace RHVoice
{
  class event
  {
  public:
    typedef std::shared_ptr<event> pointer;
    typedef label_sequence::const_iterator position;

    virtual ~event()
    {
    }

    position get_position() const
    {
      return pos;
    }

    void set_position(position pos_)
    {
      pos=pos_;
    }

    int get_time() const
    {
      if(pos==position())
        return 0;
      else
        return (pos->get_time()+pos->get_duration());
    }

    virtual bool notify(client& c) const=0;

  private:
    position pos;
  };

  typedef std::list<event::pointer> event_sequence;

  class text_unit_event: public event
  {
  protected:
    std::size_t text_start,text_length;

    text_unit_event():
      text_start(0),
      text_length(0)
    {
    }
  };

  class sentence_event: public text_unit_event
  {
  protected:
    sentence_event(const utterance& utt)
    {
      const relation& token_rel=utt.get_relation("Token");
      text_start=token_rel.first().get("position").as<std::size_t>();
      text_length=token_rel.last().get("position").as<std::size_t>()+token_rel.last().get("length").as<std::size_t>()-text_start;
    }
  };

  class sentence_starts_event: public sentence_event
  {
  public:
    explicit sentence_starts_event(const utterance& utt):
    sentence_event(utt)
    {
    }

    bool notify(client& c) const
    {
      return c.sentence_starts(text_start,text_length);
    }
  };

  class sentence_ends_event: public sentence_event
  {
  public:
    explicit sentence_ends_event(const utterance& utt):
      sentence_event(utt)
    {
    }

    bool notify(client& c) const
    {
      return c.sentence_ends(text_start,text_length);
    }
  };

  class word_event: public text_unit_event
  {
  protected:
    word_event(const item& token)
    {
      text_start=token.get("position").as<std::size_t>();
      text_length=token.get("length").as<std::size_t>();
    }
  };

  class word_starts_event: public word_event
  {
  public:
    explicit word_starts_event(const item& token):
      word_event(token)
    {
    }

    bool notify(client& c) const
    {
      return c.word_starts(text_start,text_length);
    }
  };

  class word_ends_event: public word_event
  {
  public:
    explicit word_ends_event(const item& token):
      word_event(token)
    {
    }

    bool notify(client& c) const
    {
      return c.word_ends(text_start,text_length);
    }
  };

  class mark_event: public event
  {
  public:
    explicit mark_event(const std::string& mark):
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

  class audio_event: public event
  {
  public:
    explicit audio_event(const std::string& audio_src):
      src(audio_src)
    {
    }

    bool notify(client& c) const
    {
      return c.play_audio(src);
    }

  private:
    std::string src;
  };
}
#endif
