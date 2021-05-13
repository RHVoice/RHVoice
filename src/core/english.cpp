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

#include <vector>
#include <algorithm>
#include "core/path.hpp"
#include "core/utterance.hpp"
#include "core/relation.hpp"
#include "core/item.hpp"
#include "core/english.hpp"

namespace RHVoice
{
  namespace
  {
    char english_vowel_letters[10]={'a','A','e','E','i','I','o','O','u','U'};

    struct feat_syl_in_question: public feature_function
    {
      feat_syl_in_question():
        feature_function("syl_in_question")
      {
      }

      value eval(const item& syl) const
      {
        std::string result("0");
        std::string syl_break=syl.eval("syl_break").as<std::string>();
        if((syl_break=="3")||(syl_break=="4"))
          {
            const item& word=syl.as("SylStructure").parent();
            const item& token=word.as("TokStructure").parent();
            item::const_iterator start=++(token.get_iterator());
            item::const_iterator end=token.parent().end();
            if(std::find_if(start,end,feature_equals<std::string>("name","\?"))!=end)
              {
                const item& first_word=word.as("Word").get_relation().first();
                const std::string& name=first_word.get("name").as<std::string>();
                const std::string& gpos=first_word.get("gpos").as<std::string>();
                if((gpos!="wp")&&(name!="why")&&(name!="which"))
                  result="1";
              }
          }
        return result;
      }
    };
  }

  english_info::english_info(const std::string& data_path,const std::string& userdict_path):
    language_info("English",data_path,userdict_path)
  {
    set_alpha2_code("en");
    set_alpha3_code("eng");
    register_letter_range('a',26);
    register_letter_range('A',26);
    for(std::size_t i=0;i<10;++i)
      register_vowel_letter(english_vowel_letters[i]);
  }

  std::shared_ptr<language> english_info::create_instance() const
  {
    return std::shared_ptr<language>(new english(*this));
  }

  english::english(const english_info& info_):
    language(info_),
    info(info_),
    cmulex_fst(path::join(info_.get_data_path(),"cmulex.fst")),
    cmulex_lts(path::join(info_.get_data_path(),"cmulex.lts")),
    lseq_fst(path::join(info_.get_data_path(),"lseq.fst")),
    accents_dtree(path::join(info_.get_data_path(),"accents.dt")),
    tones_dtree(path::join(info_.get_data_path(),"tones.dt"))
  {
    register_feature(std::shared_ptr<feature_function>(new feat_syl_in_question));
  }

  void english::decode_as_special_symbol(item& token,const std::string& token_name,const std::string& token_type) const
  {
    std::string word_name;
    if(token_type=="dsep")
      {
        if(token_name==".")
          word_name="point";
      }
    else if(token_type=="sign")
      {
        if(token_name=="%")
          word_name="percent";
        else if(token_name=="+")
          word_name="plus";
        else if(token_name=="-")
          word_name="minus";
      }
    if(!word_name.empty())
      token.append_child().set("name",word_name);
  }

  std::vector<std::string> english::get_word_transcription(const item& word) const
  {
    std::vector<std::string> transcription;
    const std::string& name=word.get("name").as<std::string>();
    std::vector<utf8::uint32_t> chars(str::utf8_string_begin(name),str::utf8_string_end(name));
    if(word.has_feature("lseq"))
      lseq_fst.translate(chars.begin(),chars.end(),std::back_inserter(transcription));
    else
      {
        if(!cmulex_fst.translate(chars.begin(),chars.end(),std::back_inserter(transcription)))
          cmulex_lts.apply(chars.begin(),chars.end(),std::back_inserter(transcription));
      }
    return transcription;
  }

  void english::predict_accents_and_tones(utterance& u) const
  {
    relation& syl_rel=u.get_relation("Syllable");
    for(relation::iterator syl_iter(syl_rel.begin());syl_iter!=syl_rel.end();++syl_iter)
      {
        if(accents_dtree.predict(*syl_iter).as<std::string>()!="NONE")
          syl_iter->set<std::string>("accented","1");
        else
          syl_iter->set<std::string>("accented","0");
        syl_iter->set("endtone",tones_dtree.predict(*syl_iter));
      }
  }

  void english::correct_pronunciation_of_articles(utterance& u) const
  {
    relation& trans_rel=u.get_relation("Transcription");
    for(relation::iterator word_iter(trans_rel.begin());word_iter!=trans_rel.end();++word_iter)
      {
        const std::string& word_name=word_iter->get("name").as<std::string>();
        item& seg=word_iter->last_child().as("Segment");
        if(word_name=="the")
          {
            if(seg.eval("n.ph_vc").as<std::string>()=="+")
              seg.set<std::string>("name","iy");
          }
        else
          {
            if((word_name=="a")&&
               ((!word_iter->as("Phrase").has_next())||
                word_iter->has_feature("lseq")))
              {
                seg.set<std::string>("name","ey");
                seg.as("SylStructure").parent().set<std::string>("stress","1");
                word_iter->set<std::string>("gpos","content");
              }
          }
      }
  }

  void english::correct_pronunciation_of_contractions(utterance& u) const
  {
    std::string manner,place;
    relation& trans_rel=u.get_relation("Transcription");
    for(relation::iterator word_iter(trans_rel.begin());word_iter!=trans_rel.end();++word_iter)
      {
        const std::string& word_name=word_iter->get("name").as<std::string>();
        item& seg=word_iter->last_child().as("Segment");
        bool insert_schwa=false;
        if(word_name=="\'s")
          {
            manner=seg.eval("p.ph_ctype").as<std::string>();
            place=seg.eval("p.ph_cplace").as<std::string>();
            insert_schwa=(((manner=="f")||(manner=="a"))&&((place=="a")||(place=="p")));
          }
        else
          insert_schwa=((word_name=="\'d")||(word_name=="\'ll")||(word_name=="\'ve"));
        if(insert_schwa)
          {
            item& schwa=seg.prepend();
            schwa.set<std::string>("name","ax");
            seg.as("Transcription").prepend(schwa);
            seg.as("SylStructure").prepend(schwa);
          }
        if((word_name=="\'s")&&
           ((seg.eval("p.ph_vc").as<std::string>()=="+")||
            (seg.eval("p.ph_cvox").as<std::string>()=="+")))
          seg.set<std::string>("name","z");
      }
  }

  void english::post_lex(utterance& u) const
  {
    correct_pronunciation_of_articles(u);
    correct_pronunciation_of_contractions(u);
    predict_accents_and_tones(u);
  }
}
