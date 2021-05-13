/* Copyright (C) 2013, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_HTS_LABEL_HPP
#define RHVOICE_HTS_LABEL_HPP

#include <string>
#include <list>
#include "item.hpp"
#include "relation.hpp"
#include "utterance.hpp"
#include "hts_labeller.hpp"
#include "property.hpp"

namespace RHVoice
{
  class hts_label
  {
  public:
    explicit hts_label(const item& segment_):
      segment(&segment_),
      time(-1),
      duration(0),
      position(-1),
      length(0)
    {
    }

    const std::string& get_name() const;

    double get_rate() const;
    double get_pitch() const;
    double get_volume() const;
    bool is_marked_by_sound_icon() const;

    void set_time(int time_)    // in samples
    {
      time=time_;
    }

    int get_time() const
    {
      return time;
    }

    void set_position(int position_)    // in frames
    {
      position=position_;
    }

    int get_position() const
    {
      return position;
    }

    void set_duration(int dur)  // in samples
    {
      duration=dur;
    }

    int get_duration() const
    {
      return duration;
    }

    void set_length(int dur)  // in frames
    {
      length=dur;
    }

    int get_length() const
    {
      return length;
    }

    const item& get_segment() const
    {
      return *segment;
}

  private:
    double calculate_speech_param(double absolute_change,double relative_change,const numeric_property<double>& default_value,const numeric_property<double>& min_value,const numeric_property<double>& max_value,bool clip) const;

    const item* get_token() const;

    const item* segment;
    mutable std::string name;
    int time,duration,position,length;
  };

  typedef std::list<hts_label> label_sequence;
}
#endif
