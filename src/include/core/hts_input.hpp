/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_HTS_INPUT_HPP
#define RHVOICE_HTS_INPUT_HPP

#include "item.hpp"
#include "hts_label.hpp"
#include "events.hpp"

namespace RHVoice
{
  class hts_input
  {
  public:
    hts_input() = default;

    label_sequence::iterator lbegin()
    {
      return labels.begin();
    }

    label_sequence::const_iterator lbegin() const
    {
      return labels.begin();
    }

    label_sequence::iterator lend()
    {
      return labels.end();
    }

    label_sequence::const_iterator lend() const
    {
      return labels.end();
    }

    event_sequence::iterator ebegin()
    {
      return events.begin();
    }

    event_sequence::const_iterator ebegin() const
    {
      return events.begin();
    }

    event_sequence::iterator eend()
    {
      return events.end();
    }

    event_sequence::const_iterator eend() const
    {
      return events.end();
    }

    hts_label& add_label(const item& seg)
    {
      hts_label lab(seg);
      labels.push_back(lab);
      return labels.back();
    }

    template<class T>
    void add_event()
    {
      event::pointer p(new T);
      do_add_event(p);
    }

    template<class T,typename A>
    void add_event(const A& a)
    {
      event::pointer p(new T(a));
      do_add_event(p);
    }

  private:
    hts_input(const hts_input&);
    hts_input& operator=(const hts_input&);

    void do_add_event(event::pointer& p)
    {
      events.push_back(p);
      if(!labels.empty())
        {
          event::position pos=labels.end();
          --pos;
          p->set_position(pos);
        }
    }

    label_sequence labels;
    event_sequence events;
  };
}
#endif
