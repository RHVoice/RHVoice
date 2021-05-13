/* Copyright (C) 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_PITCH_HPP
#define RHVOICE_PITCH_HPP

#include <vector>
#include <set>
#include <map>
#include <queue>
#include <cmath>
#include <string>
#include <sstream>
#include "hts_label.hpp"

namespace RHVoice
{
  class hts_input;

namespace pitch
{
  inline double interpolate(std::size_t i,std::size_t i1,double v1,std::size_t i2,double v2)
  {
    if(i1==i2)
      return v1;
    return (v1+(v2-v1)/(i2-i1)*(i-i1));
}

  class stylizer
  {
  public:
    stylizer(double t):
      threshold(t)
    {
}

    typedef std::vector<double> result_t;

    template<typename iter_t> result_t stylize(const iter_t& start,const iter_t& end) const
    {
      state_t s;
      if(start!=end)
        {
          iter_t it(start);
          s.points.push_back(point_t(*it));
          for(++it;it!=end;++it)
            {
              point_t p(s.points.back(),*it);
              s.points.push_back(p);
}
          if(s.points.size()>2)
            {
              for(std::size_t i=1;i<(s.points.size()-1);++i)
                {
                  set_distance(s,i,i-1,i+1);
                  s.dists.insert(dist_t(s.points[i]));
                }
}
}
      return do_stylize(s);
}

  private:
    struct point_t
    {
      const std::size_t index;
      const double value;
      std::size_t prev_index;
      std::size_t next_index;
      double distance;

      point_t(point_t& prev,double v):
        index(prev.index+1),
        value(v),
        prev_index(prev.index),
        next_index(0),
        distance(0)
      {
        prev.next_index=index;
}

      point_t(double v):
        index(0),
        value(v),
        prev_index(0),
        next_index(0),
        distance(0)
      {
}

      bool operator<(const point_t& other) const
      {
        return (index<other.index);
}
    };

    typedef std::vector<point_t> point_list_t;

    struct dist_t
    {
      const double value;
      const std::size_t index;

      dist_t(const point_t& p):
        value(p.distance),
        index(p.index)
      {
}

      bool operator<(const dist_t& other) const
      {
        if(value<other.value)
          return true;
        if(value>other.value)
          return false;
        return (index<other.index);
}
    };

    typedef std::set<dist_t> dist_set_t;

    struct state_t
    {
      point_list_t points;
      dist_set_t dists;
    };

    result_t do_stylize(state_t& s) const;
    void remove_point(state_t& s) const;

    double get_expected_value(const state_t& s,std::size_t i,std::size_t il,std::size_t ir) const
  {
    const point_t& pl=s.points[il];
    const point_t& pr=s.points[ir];
    return interpolate(i,il,pl.value,ir,pr.value);
}

    void set_distance(state_t& s,std::size_t i,std::size_t il,std::size_t ir) const
    {
      double v=get_expected_value(s,i,il,ir);
      s.points[i].distance=std::abs(s.points[i].value-v);
}

    bool done(const state_t& s) const
    {
      if(s.dists.empty())
        return true;
      return (s.dists.begin()->value>=threshold);
}

    const double threshold;
  };

  struct target_t
  {
    bool first;
    bool last;
    char time;
    char value;

    target_t():
      first(false),
      last(false),
      time('m'),
      value('x')
    {
}

    void clear()
    {
      first=false;
      last=false;
      time='m';
      value='x';
}

  };

  typedef std::vector<target_t> target_list_t;

  class targets_spec_parser
  {
  public:

    target_list_t parse(const std::string& spec) const;

  private:
    typedef std::map<std::string,target_list_t> cache_t;

    bool read_target(target_t& t,std::istringstream& is,const std::string& spec) const;

    mutable cache_t cache;
  };

  class editor
  {
  private:
    static const double lzero;

  public:
    editor();

    bool has_result(std::size_t i) const
    {
      return (res_values.size()>i);
}

    double get_result(std::size_t i) const
    {
      return res_values[i];
}

    void append(double v);

    void append()
    {
      append(lzero);
}

    void finish();

    void reset();
    void init(const hts_input*);

    void set_key(double k);

    bool has_work() const
    {
      if(key==lzero)
        return false;
      return has_edits;
}

  private:
    const hts_input* ling_spec;

    bool is_voiced(double v) const
    {
      return (v!=lzero);
}

    bool is_voiced(std::size_t i) const
    {
      return is_voiced(orig_values[i]);
}

    std::size_t get_last_orig_index() const
    {
      return (orig_values.size()-1);
}

    bool can_return_orig_value() const;

    void on_start_of_segment();
    void on_end_of_segment();
    void on_end_of_voiced_interval();

    typedef std::vector<double> values_t;

    struct interval_t
    {
      std::size_t start;
      std::size_t length;
      bool complete;

      interval_t(std::size_t i):
        start(i),
        length(1),
        complete(false)
      {
}

      interval_t(std::size_t i,std::size_t l):
        start(i),
        length(l),
        complete(true)
      {
}
    };

      typedef std::vector<interval_t> voiced_intervals_t;

    struct point_t
    {
      target_t def;
      std::size_t position;
      double value;
      label_sequence::const_iterator first_lab,last_lab;
    };

    double get_cont_orig_value(std::size_t i) const;
    interval_t get_vowel_interval(const point_t&) const;
    interval_t get_syllable_interval(const point_t&) const;
    std::size_t translate_target_position(const interval_t& s,const interval_t& v,const point_t& pt) const;
    double translate_target_value(const point_t&,const point_t&);
    double get_orig_base_value(std::size_t i);
    void extend_base_values(const point_t&);
    void extend_base_values();
    void extend_results();
    bool has_trailing_values(const point_t&) const;
    std::size_t get_first_voiced_in_interval(interval_t i) const;
    std::size_t get_last_voiced_in_interval(interval_t i) const;

    values_t orig_values;
    values_t res_values;
    values_t orig_base_values;
    values_t base_values;
    stylizer base_extractor;
    voiced_intervals_t voiced_intervals;
    std::vector<std::size_t> voiced_interval_numbers;
    point_t prev_point;
    label_sequence::const_iterator lab;
    double key;
    double bottom_pitch;
    double top_pitch;
    double extra_bottom_pitch;
    double extra_top_pitch;
    std::queue<point_t> pos_queue;
    std::queue<point_t> val_queue;
    bool mod_flag;
    bool has_edits;
  };
}
}
#endif
