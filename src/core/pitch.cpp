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

#include <locale>
#include <algorithm>
#include "core/exception.hpp"
#include "core/hts_input.hpp"
#include "core/pitch.hpp"
#include <iostream>
namespace
{
  const double octave=std::log(2.0);
  const double threshold=1.5*octave/12.0;
}

namespace RHVoice
{
  namespace pitch
  {
    stylizer::result_t stylizer::do_stylize(stylizer::state_t& s) const
    {
      result_t res;
      if(s.points.empty())
        return res;
      while(!done(s))
        remove_point(s);
      std::size_t i=0;
      do
        {
          const point_t& p=s.points[i];
          res.push_back(p.value);
          for(std::size_t j=i+1;j<p.next_index;++j)
            res.push_back(get_expected_value(s,j,i,p.next_index));
          i=p.next_index;
}
      while(i>0);
      return res;
}

  void stylizer::remove_point(stylizer::state_t& s) const
  {
    const std::size_t i=s.dists.begin()->index;
    s.dists.erase(s.dists.begin());
    const point_t& p=s.points[i];
    point_t& pl=s.points[p.prev_index];
    point_t& pr=s.points[p.next_index];
    pl.next_index=pr.index;
    pr.prev_index=pl.index;
    if(pl.index>0)
      {
        s.dists.erase(dist_t(pl));
        set_distance(s,pl.index,pl.prev_index,pl.next_index);
        s.dists.insert(dist_t(pl));
          }
    if(pr.index<(s.points.size()-1))
      {
        s.dists.erase(dist_t(pr));
        set_distance(s,pr.index,pr.prev_index,pr.next_index);
        s.dists.insert(dist_t(pr));
          }
}

    target_list_t targets_spec_parser::parse(const std::string& spec) const
    {
      cache_t::const_iterator it=cache.find(spec);
      if(it!=cache.end())
        return it->second;
      target_t t;
      std::istringstream is(spec);
      is.imbue(std::locale::classic());
      if(!read_target(t,is,spec))
        throw file_format_error("No targets");
      target_list_t ts;
      ts.push_back(t);
      t.clear();
      while(read_target(t,is,spec))
        {
          ts.push_back(t);
          t.clear();
        }
      cache.insert(cache_t::value_type(spec,ts));
      return ts;
    }

    bool targets_spec_parser::read_target(target_t& t,std::istringstream& is,const std::string& spec) const
    {
      char c;
      if(!(is>>c))
        return false;
      if(c=='[')
        t.first=true;
      else if(c!='(')
        throw file_format_error("No ( in "+spec);
      if(!(is>>t.time))
        throw file_format_error("No time in "+spec);
      if(!(is>>t.value))
        throw file_format_error("No value in "+spec);
      if(!(is>>c))
        throw file_format_error("Ended too early: "+spec);
      if(c==']')
        t.last=true;
      else if(c!=')')
        throw file_format_error("No ) in "+spec);
      return true;
}

    const double editor::lzero=-1.0e+10;

    editor::editor():
      ling_spec(0),
      base_extractor(threshold),
      key(lzero)
    {
}

    void editor::finish()
    {
      if(voiced_intervals.empty())
        return;
      if((voiced_intervals.back().start+voiced_intervals.back().length)!=orig_values.size())
        return;
        voiced_intervals.back().complete=true;
        mod_flag=false;
        on_end_of_voiced_interval();
}

    void editor::append(double v)
    {
      double pv=orig_values.empty()?lzero:orig_values.back();
      if(!is_voiced(v)&&is_voiced(pv))
        {
          voiced_intervals.back().complete=true;
          on_end_of_voiced_interval();
        }
      bool first_frame_in_seg=orig_values.empty();
      if(orig_values.size()==(lab->get_position()+lab->get_length()))
        {
          ++lab;
          first_frame_in_seg=true;
        }
      orig_values.push_back(v);
      if(!is_voiced(v))
        {
          voiced_interval_numbers.push_back(0);
}
      else
        {
          if(!is_voiced(pv))
            {
              voiced_intervals.push_back(interval_t(get_last_orig_index()));
}
          else
            {
              ++(voiced_intervals.back().length);
}
          voiced_interval_numbers.push_back(voiced_intervals.size()-1);
}
      if(first_frame_in_seg)
        on_start_of_segment();
      if(orig_values.size()==(lab->get_position()+lab->get_length()))
        on_end_of_segment();
      if(can_return_orig_value())
        res_values.push_back(orig_values.back());
}

    void editor::on_end_of_voiced_interval()
    {
      extend_base_values();
      if(has_trailing_values(prev_point))
        {
          base_values.resize(voiced_intervals.back().start+voiced_intervals.back().length,base_values.back());
}
      extend_results();
}

    void editor::on_end_of_segment()
    {
      if(pos_queue.empty())
        return;
      if(pos_queue.front().last_lab!=lab)
        return;
      interval_t s(get_syllable_interval(pos_queue.front()));
      interval_t v(get_vowel_interval(pos_queue.front()));
      while(!pos_queue.empty()&&(pos_queue.front().last_lab==lab))
        {
          point_t& pt=pos_queue.front();
          pt.position=translate_target_position(s,v,pt);
          val_queue.push(pt);
          pos_queue.pop();
}
      if(val_queue.back().def.last)
        mod_flag=false;
      extend_base_values();
      extend_results();
}

    void editor::on_start_of_segment()
    {
      if(pos_queue.empty())
        return;
      if(pos_queue.front().first_lab!=lab)
        return;
      mod_flag=true;
}

    bool editor::can_return_orig_value() const
    {
      if(mod_flag)
        return false;
      if(!val_queue.empty())
        return false;
      if((res_values.size()+1)!=orig_values.size())
        return false;
      if(base_values.size()>res_values.size())
        return false;
      return true;
}

    editor::interval_t editor::get_syllable_interval(const editor::point_t& pt) const
    {
      interval_t s(pt.first_lab->get_position(),pt.last_lab->get_position()+pt.last_lab->get_length()-pt.first_lab->get_position());
      return s;
    }

    std::size_t editor::translate_target_position(const editor::interval_t& s,const editor::interval_t& v,const editor::point_t& pt) const
    {
      if(v.length>0)
        {
          switch(pt.def.time)
            {
            case 's':
              return get_first_voiced_in_interval(v);
            case 'e':
              return get_last_voiced_in_interval(v);
            case 'm':
              return (v.start+static_cast<std::size_t>(0.5*(v.length-1+0.5)));
            default:
              break;
}
}
      switch(pt.def.time)
        {
        case 'f':
        case 'e':
          return get_last_voiced_in_interval(s);
        case 'm':
          return (s.start+static_cast<std::size_t>(0.5*(s.length-1+0.5)));
        case 'I':
          return s.start;
        case 'F':
          return (s.start+s.length-1);
        default:
          return get_first_voiced_in_interval(s);
        }
}

    double editor::get_orig_base_value(std::size_t i)
    {
      if(!is_voiced(i))
        return orig_values[i];
      if(i<orig_base_values.size()&&orig_base_values[i]!=lzero)
        return orig_base_values[i];
      const interval_t& v=voiced_intervals[voiced_interval_numbers[i]];
      if(!v.complete)
        return lzero;
      stylizer::result_t base(base_extractor.stylize(orig_values.begin()+v.start,orig_values.begin()+v.start+v.length));
      if(orig_base_values.size()<(v.start+v.length))
        orig_base_values.resize(v.start+v.length,lzero);
      std::copy(base.begin(),base.end(),orig_base_values.begin()+v.start);
      return orig_base_values[i];
}

    double editor::translate_target_value(const editor::point_t& ppt,const editor::point_t& pt)
    {
      switch(pt.def.value)
        {
        case 't':
          return top_pitch;
        case 'T':
          return extra_top_pitch;
        case 'b':
          return bottom_pitch;
        case 'm':
          return key;
        default:
          break;
}
      double v;
      if(!ppt.def.last)
        {
          switch(pt.def.value)
            {
            case 'h':
              v=0.5*(ppt.value+top_pitch);
              break;
              break;
            case 'l':
              v=0.5*(ppt.value+bottom_pitch);
              break;
            case 'u':
              v=0.75*ppt.value+0.25*top_pitch;
              break;
            case 'd':
              v=0.75*ppt.value+0.25*bottom_pitch;
              break;
            case 's':
              v=ppt.value;
              break;
            default:
              v=get_orig_base_value(pt.position);
              break;
}
}
      else
        v=get_orig_base_value(pt.position);
      if(!is_voiced(v))
        {
          v=get_cont_orig_value(pt.position);
          if(!is_voiced(v))
            v=key;
        }
      if(v>extra_top_pitch)
        v=extra_top_pitch;
      else if(v<bottom_pitch)
        v=bottom_pitch;
      return v;
      }

    void editor::extend_base_values(const editor::point_t& pt)
    {
      if(pt.position>=base_values.size())
        base_values.resize(pt.position+1,lzero);
      if(!pt.def.first)
        {
          for(std::size_t i=prev_point.position+1;i<pt.position;++i)
            {
              if(!is_voiced(i))
                continue;
              base_values[i]=interpolate(i,prev_point.position,prev_point.value,pt.position,pt.value);
}
        }
      base_values.back()=pt.value;
      prev_point=pt;
}

    void editor::extend_results()
    {
      std::size_t i=res_values.size();
      if(i>=base_values.size())
        return;
      double b;
      for(;i<base_values.size();++i)
        {
          if(!is_voiced(i)||base_values[i]==lzero)
            {
              res_values.push_back(orig_values[i]);
              continue;
}
          b=get_orig_base_value(i);
          if(b==lzero)
            return;
          res_values.push_back(base_values[i]+(orig_values[i]-b));
}
      if(mod_flag)
        return;
      for(;i<orig_values.size();++i)
        {
          res_values.push_back(orig_values[i]);
}
}

    double editor::get_cont_orig_value(std::size_t i) const
    {
      if(is_voiced(i))
        return orig_values[i];
      std::size_t i1=i;
      double v1=lzero;
      if(i>0)
        {
          do
            {
              --i1;
              v1=orig_values[i1];
              if(is_voiced(v1))
                break;
}
          while(i1!=0);
}
      double v2=lzero;
      std::size_t i2=i+1;
      for(;i2<orig_values.size();++i2)
        {
          v2=orig_values[i2];
          if(is_voiced(v2))
            break;
}
      if(!is_voiced(v1))
        return v2;
      if(!is_voiced(v2))
        return v1;
      return interpolate(i,i1,v1,i2,v2);
}

    editor::interval_t editor::get_vowel_interval(const editor::point_t& pt) const
    {
      interval_t v(0,0);
      label_sequence::const_iterator end=pt.last_lab;
      ++end;
      for(label_sequence::const_iterator it=pt.first_lab;it!=end;++it)
        {
          if(it->get_segment().eval("ph_vc").as<std::string>()=="+")
            {
              v.start=it->get_position();
              v.length=it->get_length();
              break;
}
}
      return v;
}

    void editor::reset()
    {
      ling_spec=0;
      orig_values.clear();
      res_values.clear();
      orig_base_values.clear();
      base_values.clear();
      voiced_intervals.clear();
      voiced_interval_numbers.clear();
      prev_point=point_t();
      prev_point.def.last=true;
      lab=label_sequence::const_iterator();
      while(!pos_queue.empty())
        pos_queue.pop();
      while(!val_queue.empty())
        val_queue.pop();
      mod_flag=false;
      has_edits=false;
}

    void editor::init(const hts_input* ls)
    {
      if(key==lzero)
        return;
      reset();
      ling_spec=ls;
      lab=ling_spec->lbegin();
      prev_point.def.last=true;
      label_sequence::const_iterator first_lab=lab;
      label_sequence::const_iterator last_lab=lab;
      while(first_lab!=ling_spec->lend())
        {
          const item* seg_ptr=first_lab->get_segment().as_ptr("SylStructure");
          if(seg_ptr==0)
            {
              ++first_lab;
              continue;
}
          last_lab=first_lab;
          const item& syl=seg_ptr->parent();
          const std::size_t n=std::distance(syl.begin(),syl.end());
          if(n>1)
            std::advance(last_lab,n-1);
          const item* syl_ptr=syl.as_ptr("PitchMod");
          if(syl_ptr==0)
            {
              first_lab=last_lab;
              ++first_lab;
              continue;
}
          for(item::const_iterator it=syl_ptr->begin();it!=syl_ptr->end();++it)
            {
              point_t pt;
              pt.first_lab=first_lab;
              pt.last_lab=last_lab;
              pt.def.first=!it->has_prev()&&!it->parent().has_prev();
              pt.def.last=!it->has_next()&&!it->parent().has_next();
              pt.def.time=it->get("time").as<char>();
              pt.def.value=it->get("value").as<char>();
              pos_queue.push(pt);
}
          first_lab=last_lab;
          ++first_lab;
}
      has_edits=!pos_queue.empty();
}

    bool editor::has_trailing_values(const editor::point_t& pt) const
    {
      if(base_values.empty())
        return false;
      if(!pt.def.last)
        return false;
      if(!is_voiced(pt.position))
        return false;
      if(pt.position<voiced_intervals.back().start)
        return false;
      if(pt.position>=(voiced_intervals.back().start+voiced_intervals.back().length-1))
        return false;
      const item& syl=pt.last_lab->get_segment().as("SylStructure").parent();
      if(syl.has_next())
        return false;
      const item& word=syl.parent().as("Phrase");
      if(word.has_next())
        return false;
      return true;
}

    void editor::extend_base_values()
    {
      while(!val_queue.empty())
        {
          point_t& pt=val_queue.front();
          if(is_voiced(pt.position)&&pt.def.value=='x'&&!voiced_intervals[voiced_interval_numbers[pt.position]].complete)
            return;
          pt.value=translate_target_value(prev_point,pt);
          extend_base_values(pt);
          val_queue.pop();
}
}

    void editor::set_key(double k)
    {
      key=std::log(k);
      bottom_pitch=key-octave/2.0;
      top_pitch=key+octave/2.0;
      extra_top_pitch=top_pitch+octave/6.0;
}

    std::size_t editor::get_first_voiced_in_interval(interval_t i) const
    {
      std::size_t end=i.start+i.length;
      for(std::size_t p=i.start;p<end;++p)
        {
          if(is_voiced(p))
            return p;
}
      return i.start;
}

    std::size_t editor::get_last_voiced_in_interval(interval_t i) const
    {
      if(i.length==0)
        return i.start;
      std::size_t p=i.start+i.length;
      while(p>i.start)
        {
          --p;
          if(is_voiced(p))
            return p;
}
      return (i.start+i.length-1);
}
}
}
