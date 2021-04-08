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

#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/brazilian_portuguese.hpp"

namespace RHVoice
{
  brazilian_portuguese_info::brazilian_portuguese_info(const std::string& data_path,const std::string& userdict_path):
    language_info("Brazilian-Portuguese",data_path,userdict_path)
  {
    set_alpha2_code("pt");
    set_alpha3_code("por");
    register_letter_range(65,26);
    register_letter_range(97,26);
    register_letter_range(192,4);
    register_letter_range(199,1);
    register_letter_range(201,2);
    register_letter_range(205,1);
    register_letter_range(211,3);
    register_letter_range(218,1);
    register_letter_range(220,1);
    register_letter_range(224,4);
    register_letter_range(231,1);
    register_letter_range(233,2);
    register_letter_range(237,1);
    register_letter_range(243,3);
    register_letter_range(250,1);
    register_letter_range(252,1);
    register_vowel_letter(65);
    register_vowel_letter(69);
    register_vowel_letter(73);
    register_vowel_letter(79);
    register_vowel_letter(85);
    register_vowel_letter(97);
    register_vowel_letter(101);
    register_vowel_letter(105);
    register_vowel_letter(111);
    register_vowel_letter(117);
    register_vowel_letter(192);
    register_vowel_letter(193);
    register_vowel_letter(194);
    register_vowel_letter(195);
    register_vowel_letter(201);
    register_vowel_letter(202);
    register_vowel_letter(205);
    register_vowel_letter(211);
    register_vowel_letter(212);
    register_vowel_letter(213);
    register_vowel_letter(218);
    register_vowel_letter(220);
    register_vowel_letter(224);
    register_vowel_letter(225);
    register_vowel_letter(226);
    register_vowel_letter(227);
    register_vowel_letter(233);
    register_vowel_letter(234);
    register_vowel_letter(237);
    register_vowel_letter(243);
    register_vowel_letter(244);
    register_vowel_letter(245);
    register_vowel_letter(250);
    register_vowel_letter(252);
  }

  std::shared_ptr<language> brazilian_portuguese_info::create_instance() const
  {
    return std::shared_ptr<language>(new brazilian_portuguese(*this));
  }

  brazilian_portuguese::brazilian_portuguese(const brazilian_portuguese_info& info_):
    language(info_),
    info(info_),
    g2p_fst(path::join(info_.get_data_path(),"g2p.fst")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst"))
  {
    try
      {
        h_fst.reset(new fst(path::join(info.get_data_path(),"homographs.fst")));
      }
    catch(const io::open_error& e)
      {
      }
  }

  std::vector<std::string> brazilian_portuguese::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    if(word.has_feature("lseq"))
      lseq_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    else
      g2p_fst.translate(str::utf8_string_begin(name),str::utf8_string_end(name),std::back_inserter(transcription));
    return transcription;
  }

  void brazilian_portuguese::before_g2p(item& w) const
  {
    item& pw=w.as("Phrase");
    const std::string& name=w.get("name").as<std::string>();
    if(pw.has_next())
      return;
    if(!w.as("TokStructure").parent().has_feature("one-letter"))
      return;
    if(pw.has_prev())
      {
        if(name!="e"&&name!="o")
          return;
      }
    w.set<std::string>("gpos","content");
    w.set("lseq",true);
}

  void brazilian_portuguese::post_lex(utterance& u) const
  {
    process_homographs(u);
    fix_whw_stress(u);
}

  void brazilian_portuguese::process_homographs(utterance& u) const
  {
    if(h_fst.get()==0)
      return;
    std::string name;
    relation& phrase_rel=u.get_relation("Phrase");
    for(relation::iterator phrase_iter(phrase_rel.begin());phrase_iter!=phrase_rel.end();++phrase_iter)
      {
        h_fst->translate(phrase_iter->begin(),phrase_iter->end(),set_feature_iterator<std::string>("homograph",phrase_iter->begin(),phrase_iter->end()));
        for(item::iterator word_iter(phrase_iter->begin());word_iter!=phrase_iter->end();++word_iter)
          {
            const std::string& h=word_iter->get("homograph").as<std::string>();
            if(h=="H0")
              continue;
            item& syls=word_iter->as("SylStructure");
            for(item::reverse_iterator syl_iter=syls.rbegin();syl_iter!=syls.rend();++syl_iter)
              {
                if((*syl_iter).get("stress").as<std::string>()!="1")
                  continue;
                item::iterator vowel_pos=std::find_if((*syl_iter).begin(),(*syl_iter).end(),feature_equals<std::string>("ph_vc","+"));
                if(vowel_pos==(*syl_iter).end())
                  break;
                name=vowel_pos->get("name").as<std::string>();
                if(h=="H1")
                  {
                    if(name=="ee")
                      name="e";
                    else if(name=="oo")
                      name="o";
}
                else if(h=="H2")
                  {
                    if(name=="e")
                      name="ee";
                    else if(name=="o")
                      name="oo";
}
                vowel_pos->set<std::string>("name",name);
                break;
}
          }
      }
}

  void brazilian_portuguese::fix_whw_stress(utterance& u) const
  {
    if(u.get_utt_type()!="whq")
      return;
    relation& rel=u.get_relation("SylStructure");
    for(relation::iterator word_iter=rel.begin();word_iter!=rel.end();++word_iter)
      {
        if(word_iter->get("gpos").as<std::string>()!="whw")
          continue;
        const std::string& name=word_iter->get("name").as<std::string>();
        if(name!="que"&&name!="como")
          continue;
        word_iter->first_child().set<std::string>("stress","1");
}
}
}
