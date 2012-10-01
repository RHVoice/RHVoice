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

#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
#include "core/io.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/russian.hpp"

namespace RHVoice
{
  namespace
  {
    struct is_not_v_segment: public std::unary_function<const item&,bool>
    {
      bool operator()(const item& seg) const
      {
        return (seg.get("name").as<std::string>()[0]!='v');
      }
    };

    struct is_vowel: public std::unary_function<utf8::uint32_t,bool>
    {
      bool operator()(utf8::uint32_t c) const
      {
        return std::binary_search(vowels,vowels+10,c);
      }
    private:
      static utf8::uint32_t vowels[10];
    };

    utf8::uint32_t is_vowel::vowels[10]={1072,1077,1080,1086,1091,1099,1101,1102,1103,1105};
  }

  smart_ptr<language> russian_info::create_instance() const
  {
    return smart_ptr<language>(new russian(*this));
  }

  russian::russian(const language_info& info):
    language(info),
    clit_fst(path::join(info.get_data_path(),"clitics.fst")),
    g2p_fst(path::join(info.get_data_path(),"g2p.fst")),
    lseq_fst(path::join(info.get_data_path(),"lseq.fst")),
    untranslit_fst(path::join(info.get_data_path(),"untranslit.fst")),
    dict_fst(path::join(info.get_data_path(),"dict.fst")),
    stress_rules(path::join(info.get_data_path(),"stress.fsm"),io::integer_reader<uint8_t>())
  {
  }

  void russian::decode_as_word(item& token) const
  {
    const std::string& token_name=token.get("name").as<std::string>();
    std::list<std::string> lowercase_letters;
    downcase_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),std::back_inserter(lowercase_letters));
    std::string word_name;
    untranslit_fst.translate(lowercase_letters.begin(),lowercase_letters.end(),str::append_string_iterator(word_name));
    item& word=token.append_child();
    word.set("name",word_name);
  }

  void russian::mark_clitics(utterance& u) const
  {
    relation& phrase_rel=u.get_relation("Phrase");
    for(relation::iterator phrase_iter(phrase_rel.begin());phrase_iter!=phrase_rel.end();++phrase_iter)
      {
        clit_fst.translate(phrase_iter->begin(),phrase_iter->end(),set_feature_iterator<std::string>("clitic",phrase_iter->begin(),phrase_iter->end()));
        item& first_word=phrase_iter->first_child();
        if(first_word.get("clitic").as<std::string>()=="en")
          {
            first_word.set<std::string>("clitic","host");
            first_word.set<std::string>("gpos","content");
          }
        item& last_word=phrase_iter->last_child();
        if(last_word.get("clitic").as<std::string>()=="pro")
          {
            last_word.set<std::string>("clitic","host");
            last_word.set<std::string>("gpos","content");
          }
        for(item::iterator word_iter(phrase_iter->begin());word_iter!=phrase_iter->end();++word_iter)
          {
            if(word_iter->as("TokStructure").parent().has_feature("stress_marks")||
               word_iter->has_feature("lseq"))
              {
                word_iter->set<std::string>("clitic","host");
                word_iter->set<std::string>("gpos","content");
              }
          }
      }
    relation& sylstruct_rel=u.get_relation("SylStructure");
    for(relation::iterator word_iter(sylstruct_rel.begin());word_iter!=sylstruct_rel.end();++word_iter)
      {
        if(word_iter->get("clitic").as<std::string>()=="host")
          {
            item&seg=word_iter->as("Transcription").first_child();
            if(!seg.has_next()&&(seg.eval("ph_vc").as<std::string>()=="-"))
              {
                const std::string& name=seg.get("name").as<std::string>();
                if(name=="s")
                  {
                    item& extra_seg=seg.prepend();
                    extra_seg.set<std::string>("name","e");
                    seg.as("Segment").prepend(extra_seg);
                    seg.as("SylStructure").prepend(extra_seg);
                  }
                else
                  {
                    item& extra_seg=seg.append();
                    extra_seg.set<std::string>("name",(name=="k")?"a":"e");
                    seg.as("Segment").append(extra_seg);
                    seg.as("SylStructure").append(extra_seg);
                  }
                word_iter->first_child().set<std::string>("stress","1");
              }
          }
        else
          {
            for(item::iterator syl_iter(word_iter->begin());syl_iter!=word_iter->end();++syl_iter)
              {
                syl_iter->set<std::string>("stress","0");
              }
          }
      }
  }

  void russian::transcribe_word(item& word) const
  {
    const std::string& name=word.get("name").as<std::string>();
    std::vector<utf8::uint32_t> chars;
    chars.push_back('#');
    chars.insert(chars.end(),str::utf8_string_begin(name),str::utf8_string_end(name));
    chars.push_back('#');
    if(word.has_feature("lseq"))
      {
        lseq_fst.translate(chars.begin()+1,chars.end()-1,word.back_inserter());
        return;
      }
    std::list<std::string> stressed_word;
    bool explicit_stress=false;
    const value& val_stress_marks=word.as("TokStructure").parent().get("stress_marks",true);
    if(!val_stress_marks.empty())
      {
        utf8::uint32_t cp;
        unsigned int i;
        const std::vector<unsigned int>& stress_marks=val_stress_marks.as<std::vector<unsigned int> >();
        for(std::vector<unsigned int>::const_iterator it(stress_marks.begin());it!=stress_marks.end();++it)
          {
            i=*it+1;
            cp=chars.at(i);
            if(is_vowel()(cp))
              {
                if(cp!=1105)
                  chars[i]=str::toupper(cp);
                explicit_stress=true;
              }
          }
      }
    if((!explicit_stress)&&(!dict_fst.translate(chars.begin()+1,chars.end()-1,std::back_inserter(stressed_word))))
      {
        if(std::find(chars.begin(),chars.end(),1105)==chars.end())
          {
            std::vector<utf8::uint32_t>::iterator first_vowel_pos=std::find_if(chars.begin(),chars.end(),is_vowel());
            if(first_vowel_pos!=chars.end())
              {
                if(std::find_if(first_vowel_pos+1,chars.end(),is_vowel())==chars.end())
                  *first_vowel_pos=str::toupper(*first_vowel_pos);
                else
                  {
                    rules<uint8_t>::match m(stress_rules,chars.begin(),chars.end());
                    if(!m.empty())
                      {
                        std::size_t pos=m.pos(m.size()-1)+m.value(m.size()-1);
                        chars[pos]=str::toupper(chars[pos]);
                      }
                  }
              }
          }
      }
    if(stressed_word.empty())
      g2p_fst.translate(chars.begin()+1,chars.end()-1,word.back_inserter());
      else
        g2p_fst.translate(stressed_word.begin(),stressed_word.end(),word.back_inserter());
  }

  void russian::reduce_vowels(utterance& u) const
  {
    item::iterator vowel_pos;
    std::string name,prev_pal;
    relation& sylstruct_rel=u.get_relation("SylStructure");
    for(relation::iterator word_iter(sylstruct_rel.begin());word_iter!=sylstruct_rel.end();++word_iter)
      {
        if(word_iter->has_feature("lseq"))
          continue;
        const std::string& clitic=word_iter->get("clitic").as<std::string>();
        for(item::iterator syl_iter(word_iter->begin());syl_iter!=word_iter->end();++syl_iter)
          {
            if(syl_iter->get("stress").as<std::string>()!="0")
              continue;
            vowel_pos=std::find_if(syl_iter->begin(),syl_iter->end(),feature_equals<std::string>("ph_vc","+"));
            if(vowel_pos==syl_iter->end())
              continue;
            item& vowel=vowel_pos->as("Transcription");
            name=vowel.get("name").as<std::string>();
            if(vowel.has_prev()&&vowel.has_next())
              {
                prev_pal=vowel.eval("p.ph_cpal").as<std::string>();
                if(prev_pal=="+")
                  {
                    if((name=="e")||(name=="a")||(name=="o"))
                      vowel.set<std::string>("name","i");
                  }
                else
                  {
                    if(name=="o")
                      vowel.set<std::string>("name","a");
                    else
                      {
                        if((name=="e")&&(prev_pal=="-"))
                          vowel.set<std::string>("name","y");
                      }
                  }
              }
            else
              {
                if(name=="o")
                  vowel.set<std::string>("name","a");
                else
                  {
                    if((name=="e")&&(clitic=="pro")&&vowel.has_prev())
                      vowel.set<std::string>("name","i");
                  }
              }
          }
      }
  }

  void russian::do_final_devoicing_and_voicing_assimilation(utterance& u) const
  {
    relation& seg_rel=u.get_relation("Segment");
    for(relation::reverse_iterator seg_iter(seg_rel.rbegin());seg_iter!=seg_rel.rend();++seg_iter)
      {
        std::string vpair=seg_iter->eval("ph_vpair").as<std::string>();
        if(vpair!="0")
          {
            item& seg=*seg_iter;
            item& next_seg=*std::find_if(++(seg.get_iterator()),seg_rel.end(),is_not_v_segment());
            if(seg.eval("ph_cvox").as<std::string>()=="+")
              {
                if((next_seg.get("name").as<std::string>()=="pau")||
                   (next_seg.eval("ph_cvox").as<std::string>()=="-")||
                   ((!seg.as("Transcription").has_next())&&
                    (seg.eval("R:Transcription.parent.clitic").as<std::string>()!="pro")&&
                    (next_seg.eval("ph_vpair").as<std::string>()=="0")))
                  seg.set("name",vpair);
              }
            else
              {
                if((next_seg.eval("ph_cvox").as<std::string>()=="+")&&
                   (next_seg.eval("ph_vpair").as<std::string>()!="0"))
                  seg.set("name",vpair);
              }
          }
      }
  }

  void russian::rename_unstressed_vowels(utterance& u) const
  {
    relation& seg_rel=u.get_relation("Segment");
    for(relation::iterator seg_iter=seg_rel.begin();seg_iter!=seg_rel.end();++seg_iter)
      {
        if((seg_iter->eval("ph_vc").as<std::string>()=="+")&&
           (seg_iter->eval("R:SylStructure.parent.stress").as<std::string>()=="0"))
          {
            std::string name(seg_iter->get("name").as<std::string>());
            name+="0";
            seg_iter->set("name",name);
          }
      }
  }

  void russian::post_lex(utterance& u) const
  {
    mark_clitics(u);
    reduce_vowels(u);
    do_final_devoicing_and_voicing_assimilation(u);
    rename_unstressed_vowels(u);
  }
}
