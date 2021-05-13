/* Copyright (C) 2012, 2014  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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


    utf8::uint32_t russian_vowel_letters[20]={1025,1040,1045,1048,1054,1059,1067,1069,1070,1071,1072,1077,1080,1086,1091,1099,1101,1102,1103,1105};
  }

  russian_info::russian_info(const std::string& data_path,const std::string& userdict_path):
    language_info("Russian",data_path,userdict_path)
  {
    set_alpha2_code("ru");
    set_alpha3_code("rus");
    register_letter_range(0x430,32);
    register_letter_range(0x410,32);
    register_letter(0x451);
    register_letter(0x401);
    for(std::size_t i=0;i<20;++i)
      register_vowel_letter(russian_vowel_letters[i]);
  }


  std::shared_ptr<language> russian_info::create_instance() const
  {
    return std::shared_ptr<language>(new russian(*this));
  }

  russian::russian(const russian_info& info_):
    language(info_),
    info(info_),
    clit_fst(path::join(info_.get_data_path(),"clitics.fst")),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst")),
    untranslit_fst(path::join(info_.get_data_path(),"untranslit.fst")),
    split_fst(path::join(info_.get_data_path(),"split.fst")),
    dict_fst(path::join(info_.get_data_path(),"dict.fst")),
    stress_fst(path::join(info_.get_data_path(),"stress.fst")),
    stress_rules(path::join(info_.get_data_path(),"stress.fsm"),io::integer_reader<uint8_t>())
  {
    try
      {
        rulex_dict_fst.reset(new fst(path::join(info.get_data_path(),"rulex_dict.fst")));
        rulex_rules_fst.reset(new fst(path::join(info.get_data_path(),"rulex_rules.fst")));
      }
    catch(const io::open_error& e)
      {
      }
    try
      {
        stress_marks_fst.reset(new fst(path::join(info.get_data_path(),"stress_marks.fst")));
      }
    catch(const io::open_error& e)
      {
      }
  }

  bool russian::decode_as_russian_word(item& token,const std::string& token_name) const
  {
    std::list<std::string> lowercase_letters;
    downcase_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),std::back_inserter(lowercase_letters));
    std::list<std::string> untranslit_result;
    untranslit_fst.translate(lowercase_letters.begin(),lowercase_letters.end(),std::back_inserter(untranslit_result));
    std::list<std::string> split_result;
    split_fst.translate(untranslit_result.begin(),untranslit_result.end(),std::back_inserter(split_result));
    if(split_result.empty())
      return true;
    std::string word_name;
    std::list<std::string>::const_iterator it=split_result.begin();
    std::string chr;
    do
      {
        chr=*it;
        if(chr!=".")
          word_name+=chr;
        ++it;
        if(it==split_result.end()||chr==".")
          {
            item& word=token.append_child();
            word.set("name",word_name);
            word_name.clear();
          }
      }
    while(it!=split_result.end());
    return true;
  }

  void russian::decode_as_word(item& token,const std::string& token_name) const
  {
      decode_as_russian_word(token,token_name);
  }

  void russian::mark_clitics(utterance& u) const
  {
    relation& phrase_rel=u.get_relation("Phrase");
    for(relation::iterator phrase_iter(phrase_rel.begin());phrase_iter!=phrase_rel.end();++phrase_iter)
      {
        clit_fst.translate(phrase_iter->begin(),phrase_iter->end(),set_feature_iterator<std::string>("clitic",phrase_iter->begin(),phrase_iter->end()));
        for(item::iterator word_iter(phrase_iter->begin());word_iter!=phrase_iter->end();++word_iter)
          {
            if(word_iter->has_feature("lseq"))
              {
                word_iter->set<std::string>("clitic","host");
                word_iter->set<std::string>("gpos","content");
              }
          }
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
      }
    relation& sylstruct_rel=u.get_relation("SylStructure");
    for(relation::iterator word_iter(sylstruct_rel.begin());word_iter!=sylstruct_rel.end();++word_iter)
      {
        if(word_iter->has_feature("english"))
          continue;
        if(word_iter->eval("word_stress_pattern").as<stress_pattern>().get_state()!=stress_pattern::undefined)
          continue;
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

  bool russian::transcribe_letter_sequence(const item& word,std::vector<std::string>& transcription) const
  {
    if(!word.has_feature("lseq"))
      return false;
    const std::string& name=word.get("name").as<std::string>();
    lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return true;
  }

  bool russian::transcribe_word_with_stress_marks(const item& word,std::vector<std::string>& transcription) const
  {
    const std::string& name=word.get("name").as<std::string>();
    if(stress_marks_fst)
      {
        std::vector<std::string> stressed;
        if(stress_marks_fst->translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(stressed)))
          {
            g2p_fst.translate(stressed.begin(),stressed.end(),std::back_inserter(transcription));
            return true;
          }
      }
    if(word.eval("word_stress_pattern").as<stress_pattern>().get_state()==stress_pattern::undefined)
      return false;
    g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return true;
  }

  bool russian::transcribe_word_from_dict(const item& word,std::vector<std::string>& transcription) const
  {
    const std::string& name=word.get("name").as<std::string>();
    if(dict_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription)))
      return true;
    else
      return false;
  }

  bool russian::transcribe_monosyllabic_word(const item& word,std::vector<std::string>& transcription) const
  {
    const std::string& name=word.get("name").as<std::string>();
    str::utf8_string_iterator start=str::utf8_string_begin(name);
    str::utf8_string_iterator end=str::utf8_string_end(name);
    str::utf8_string_iterator first_vowel_pos=std::find_if(start,end,is_vowel_letter(info));
    if(first_vowel_pos==end)
      return false;
    str::utf8_string_iterator next_pos=first_vowel_pos;
    ++next_pos;
    if(std::find_if(next_pos,end,is_vowel_letter(info))!=end)
      return false;
    std::vector<utf8::uint32_t> letters(start,first_vowel_pos);
    utf8::uint32_t cp=*first_vowel_pos;
    letters.push_back((cp==1105)?cp:str::toupper(cp));
    letters.insert(letters.end(),next_pos,end);
    g2p_fst.translate(letters.begin(),letters.end(),std::back_inserter(transcription));
    return true;
  }

  bool russian::transcribe_word_from_stress_dict(const item& word,std::vector<std::string>& transcription) const
  {
    const std::string& name=word.get("name").as<std::string>();
    std::vector<std::string> stressed;
    if(stress_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(stressed)))
      {
        g2p_fst.translate(stressed.begin(),stressed.end(),std::back_inserter(transcription));
        return true;
      }
    else
      return false;
  }

  bool russian::transcribe_word_applying_stress_rules(const item& word,std::vector<std::string>& transcription) const
  {
    const std::string& name=word.get("name").as<std::string>();
    std::vector<utf8::uint32_t> letters;
    letters.push_back('#');
    letters.insert(letters.end(),str::utf8_string_begin(name),str::utf8_string_end(name));
    letters.push_back('#');
    rules<uint8_t>::match m(stress_rules,letters.begin(),letters.end());
    if(m.empty())
      return false;
    std::size_t pos=m.pos(m.size()-1)+m.value(m.size()-1);
    letters.at(pos)=str::toupper(letters.at(pos));
    g2p_fst.translate(letters.begin()+1,letters.end()-1,std::back_inserter(transcription));
    return true;
  }

  bool russian::transcribe_unknown_word(const item& word,std::vector<std::string>& transcription) const
  {
    const std::string& name=word.get("name").as<std::string>();
    g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return true;
  }

  bool russian::transcribe_word_from_rulex(const item& word,std::vector<std::string>& transcription) const
  {
    if(rulex_dict_fst.get()==0)
      return false;
    const std::string& name=word.get("name").as<std::string>();
    std::vector<std::string> stressed;
    if(rulex_dict_fst->translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(stressed)))
      {
        g2p_fst.translate(stressed.begin(),stressed.end(),std::back_inserter(transcription));
        return true;
      }
    else
      {
        if(rulex_rules_fst.get()==0)
          return false;
        if(rulex_rules_fst->translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(stressed)))
          {
            g2p_fst.translate(stressed.begin(),stressed.end(),std::back_inserter(transcription));
            return true;
          }
        else
          return false;
      }
  }

  std::vector<std::string> russian::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    transcribe_letter_sequence(word,transcription)||
      transcribe_word_with_stress_marks(word,transcription)||
      transcribe_word_from_dict(word,transcription)||
      transcribe_word_from_rulex(word,transcription)||
      transcribe_word_from_stress_dict(word,transcription)||
      transcribe_monosyllabic_word(word,transcription)||
      transcribe_word_applying_stress_rules(word,transcription)||
      transcribe_unknown_word(word,transcription);
    return transcription;
  }

  void russian::reduce_vowels(utterance& u) const
  {
    item::iterator vowel_pos;
    std::string name,prev_pal;
    relation& sylstruct_rel=u.get_relation("SylStructure");
    for(relation::iterator word_iter(sylstruct_rel.begin());word_iter!=sylstruct_rel.end();++word_iter)
      {
        if(word_iter->has_feature("lseq")||
           word_iter->has_feature("english"))
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
        if((*seg_iter).in("Transcription")&&
           (*seg_iter).as("Transcription").parent().has_feature("english"))
          continue;
        std::string vpair=(*seg_iter).eval("ph_vpair").as<std::string>();
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
