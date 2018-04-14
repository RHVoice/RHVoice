/* Copyright (C) 2012, 2014, 2016  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <algorithm>
#include <functional>
#include <iterator>
#include <sstream>
#include "core/str.hpp"
#include "core/engine.hpp"
#include "core/item.hpp"
#include "core/language.hpp"
#include "core/russian.hpp"
#include "core/english.hpp"
#include "core/esperanto.hpp"
#include "core/georgian.hpp"
#include "core/ukrainian.hpp"
#include "core/kyrgyz.hpp"
#include "core/tatar.hpp"
#include "core/stress_pattern.hpp"
#include "core/event_logger.hpp"

namespace RHVoice
{
  namespace
  {
    const value zero(std::string("0"));

    struct feat_pos_in_syl: public feature_function
    {
      feat_pos_in_syl():
        feature_function("pos_in_syl")
      {
      }

      value eval(const item& seg) const
      {
        const item& seg_in_syl(seg.as("Segment").as("SylStructure"));
        unsigned int result=std::distance(seg_in_syl.parent().begin(),seg_in_syl.get_iterator());
        return result;
      }
    };

    struct feat_pos_in_syl_bw: public feature_function
    {
      feat_pos_in_syl_bw():
        feature_function("pos_in_syl_bw")
      {
      }

      value eval(const item& seg) const
      {
        const item& seg_in_syl(seg.as("Segment").as("SylStructure"));
        unsigned int result=std::distance(seg_in_syl.get_iterator(),seg_in_syl.parent().end())-1;
        return result;
      }
    };

    struct feat_syl_numphones: public feature_function
    {
      feat_syl_numphones():
        feature_function("syl_numphones")
      {
      }

      value eval(const item& syl) const
      {
        const item& syl_in_word=syl.as("Syllable").as("SylStructure");
        unsigned int result=std::distance(syl_in_word.begin(),syl_in_word.end());
        return result;
      }
    };
    struct feat_pos_in_word: public feature_function
    {
      feat_pos_in_word():
        feature_function("pos_in_word")
      {
      }

      value eval(const item& syl) const
      {
        const item& syl_in_word=syl.as("Syllable").as("SylStructure");
        unsigned int result=std::distance(syl_in_word.parent().begin(),syl_in_word.get_iterator());
        return result;
      }
    };

    struct feat_seg_pos_in_word: public feature_function
    {
      feat_seg_pos_in_word():
        feature_function("seg_pos_in_word")
      {
      }

      value eval(const item& seg) const
      {
        const item& seg_in_word(seg.as("Transcription"));
        unsigned int result=std::distance(seg_in_word.parent().begin(),seg_in_word.get_iterator());
        return result;
      }
    };

    struct feat_seg_pos_in_word_bw: public feature_function
    {
      feat_seg_pos_in_word_bw():
        feature_function("seg_pos_in_word_bw")
      {
      }

      value eval(const item& seg) const
      {
        const item& seg_in_word(seg.as("Transcription"));
        unsigned int result=std::distance(seg_in_word.get_iterator(),seg_in_word.parent().end())-1;
        return result;
      }
    };

    struct feat_pos_in_word_bw: public feature_function
    {
      feat_pos_in_word_bw():
        feature_function("pos_in_word_bw")
      {
      }

      value eval(const item& syl) const
      {
        const item& syl_in_word=syl.as("Syllable").as("SylStructure");
        unsigned int result=std::distance(syl_in_word.get_iterator(),syl_in_word.parent().end())-1;
        return result;
      }
    };

    struct feat_word_numsyls: public feature_function
    {
      feat_word_numsyls():
        feature_function("word_numsyls")
      {
      }

      value eval(const item& word) const
      {
        const item& word_with_syls=word.as("Word").as("SylStructure");
        unsigned int result=std::distance(word_with_syls.begin(),word_with_syls.end());
        return result;
      }
    };

    struct feat_syl_in: public feature_function
    {
      feat_syl_in():
        feature_function("syl_in")
      {
      }

      value eval(const item& syl) const
      {
        const item& phrase=syl.as("SylStructure").parent().as("Phrase").parent();
        const item& first_syl_in_phrase=phrase.first_child().as("SylStructure").first_child().as("Syllable");
        unsigned int result=std::distance(first_syl_in_phrase.get_iterator(),syl.as("Syllable").get_iterator());
        return result;
      }
    };

    struct feat_syl_out: public feature_function
    {
      feat_syl_out():
        feature_function("syl_out")
      {
      }

      value eval(const item& syl) const
      {
        const item& phrase=syl.as("SylStructure").parent().as("Phrase").parent();
        const item& last_syl_in_phrase=phrase.last_child().as("SylStructure").last_child().as("Syllable");
        unsigned int result=std::distance(syl.as("Syllable").get_iterator(),last_syl_in_phrase.get_iterator());
        return result;
      }
    };

    struct feat_ssyl_in: public feature_function
    {
      feat_ssyl_in():
        feature_function("ssyl_in")
      {
      }

      value eval(const item& syl) const
      {
        const item& phrase=syl.as("SylStructure").parent().as("Phrase").parent();
        const item& first_syl_in_phrase=phrase.first_child().as("SylStructure").first_child().as("Syllable");
        unsigned int result=std::count_if(first_syl_in_phrase.get_iterator(),syl.as("Syllable").get_iterator(),feature_equals<std::string>("stress","1"));
        return result;
      }
    };

    struct feat_ssyl_out: public feature_function
    {
      feat_ssyl_out():
        feature_function("ssyl_out")
      {
      }

      value eval(const item& syl) const
      {
        const item& phrase=syl.as("SylStructure").parent().as("Phrase").parent();
        const item& last_syl_in_phrase=phrase.last_child().as("SylStructure").last_child().as("Syllable");
        unsigned int result=std::count_if(++(syl.as("Syllable").get_iterator()),++(last_syl_in_phrase.get_iterator()),feature_equals<std::string>("stress","1"));
        return result;
      }
    };

    struct feat_asyl_in: public feature_function
    {
      feat_asyl_in():
        feature_function("asyl_in")
      {
      }

      value eval(const item& syl) const
      {
        const item& phrase=syl.as("SylStructure").parent().as("Phrase").parent();
        const item& first_syl_in_phrase=phrase.first_child().as("SylStructure").first_child().as("Syllable");
        unsigned int result=std::count_if(first_syl_in_phrase.get_iterator(),syl.as("Syllable").get_iterator(),feature_equals<std::string>("accented","1"));
        return result;
      }
    };

    struct feat_asyl_out: public feature_function
    {
      feat_asyl_out():
        feature_function("asyl_out")
      {
      }

      value eval(const item& syl) const
      {
        const item& phrase=syl.as("SylStructure").parent().as("Phrase").parent();
        const item& last_syl_in_phrase=phrase.last_child().as("SylStructure").last_child().as("Syllable");
        unsigned int result=std::count_if(++(syl.as("Syllable").get_iterator()),++(last_syl_in_phrase.get_iterator()),feature_equals<std::string>("accented","1"));
        return result;
      }
    };

    struct feat_syl_vowel: public feature_function
    {
      feat_syl_vowel():
        feature_function("syl_vowel")
      {
      }

      value eval(const item& syl) const
      {
        const item& syl_in_word=syl.as("Syllable").as("SylStructure");
        item::const_iterator vowel_pos=std::find_if(syl_in_word.begin(),syl_in_word.end(),feature_equals<std::string>("ph_vc","+"));
        return ((vowel_pos==syl_in_word.end())?zero:(vowel_pos->get("name")));
      }
    };

    struct feat_pos_in_phrase: public feature_function
    {
      feat_pos_in_phrase():
        feature_function("pos_in_phrase")
      {
      }

      value eval(const item& word) const
      {
        const item& phrase=word.as("Word").as("Phrase").parent();
        unsigned int result=std::distance(phrase.begin(),word.as("Phrase").get_iterator());
        return result;
      }
    };

    struct feat_words_out: public feature_function
    {
      feat_words_out():
        feature_function("words_out")
      {
      }

      value eval(const item& word) const
      {
        const item& phrase=word.as("Word").as("Phrase").parent();
        unsigned int result=std::distance(word.as("Phrase").get_iterator(),phrase.end())-1;
        return result;
      }
    };

    struct feat_content_words_in: public feature_function
    {
      feat_content_words_in():
        feature_function("content_words_in")
      {
      }

      value eval(const item& word) const
      {
        const item& phrase=word.as("Word").as("Phrase").parent();
        unsigned int result=std::count_if(phrase.begin(),word.as("Phrase").get_iterator(),feature_equals<std::string>("gpos","content"));
        return result;
      }
    };

    struct feat_content_words_out: public feature_function
    {
      feat_content_words_out():
        feature_function("content_words_out")
      {
      }

      value eval(const item& word) const
      {
        const item& phrase=word.as("Word").as("Phrase").parent();
        unsigned int result=std::count_if(++(word.as("Phrase").get_iterator()),phrase.end(),feature_equals<std::string>("gpos","content"));
        return result;
      }
    };

    struct feat_phrase_numsyls: public feature_function
    {
      feat_phrase_numsyls():
        feature_function("phrase_numsyls")
      {
      }

      value eval(const item& phrase) const
      {
        const item& first_syl_in_phrase=phrase.as("Phrase").first_child().as("SylStructure").first_child().as("Syllable");
        const item& last_syl_in_phrase=phrase.as("Phrase").last_child().as("SylStructure").last_child().as("Syllable");
        unsigned int result=std::distance(first_syl_in_phrase.get_iterator(),++(last_syl_in_phrase.get_iterator()));
        return result;
      }
    };

    struct feat_phrase_numwords: public feature_function
    {
      feat_phrase_numwords():
        feature_function("phrase_numwords")
      {
      }

      value eval(const item& phrase) const
      {
        unsigned result=std::distance(phrase.as("Phrase").begin(),phrase.as("Phrase").end());
        return result;
      }
    };

    struct feat_syl_break: public feature_function
    {
      feat_syl_break():
        feature_function("syl_break")
      {
      }

      value eval(const item& syl) const
      {
        unsigned int result;
        const item& syl_in_utt=syl.as("Syllable");
        if(syl_in_utt.has_next())
          {
            const item& syl_in_word=syl.as("SylStructure");
            if(syl_in_word.has_next())
              result=0;
            else
              {
                if(syl_in_word.parent().as("Phrase").has_next())
                  result=1;
                else
                  result=3;
              }
          }
        else
          result=4;
        return result;
      }
    };

    struct feat_phrases_in: public feature_function
    {
      feat_phrases_in():
        feature_function("phrases_in")
      {
      }

      value eval(const item& phrase) const
      {
        const item& phrase0=phrase.as("Phrase");
        const relation& rel=phrase0.get_relation();
        unsigned int result=std::distance(rel.begin(),phrase0.get_iterator());
        return result;
      }
    };

    struct feat_phrases_out: public feature_function
    {
      feat_phrases_out():
        feature_function("phrases_out")
      {
      }

      value eval(const item& phrase) const
      {
        const item& phrase0=phrase.as("Phrase");
        const relation& rel=phrase0.get_relation();
        unsigned int result=std::distance(phrase0.get_iterator(),rel.end())-1;
        return result;
      }
    };

    struct feat_syl_coda_size: public feature_function
    {
      feat_syl_coda_size():
        feature_function("syl_codasize")
      {
      }

      value eval(const item& syl) const
      {
        const item& syl_in_word=syl.as("Syllable").as("SylStructure");
        item::const_iterator vowel_pos=std::find_if(syl_in_word.begin(),syl_in_word.end(),feature_equals<std::string>("ph_vc","+"));
        unsigned int result;
        if(vowel_pos==syl_in_word.end())
          result=std::distance(syl_in_word.begin(),syl_in_word.end());
else
  result=std::distance(vowel_pos,syl_in_word.end())-1;
        return result;
      }
    };


    class phoneme_feature_function: public feature_function
    {
    public:
      explicit phoneme_feature_function(const std::string& name):
        feature_function("ph_"+name),
        feature_name(name)
      {
      }

      value eval(const item& seg) const
      {
        const std::string& name=seg.as("Segment").get("name").as<std::string>();
        const phoneme_set& phs=seg.get_relation().get_utterance().get_language().get_phoneme_set();
        return phs.get_phoneme(name)[feature_name];
      }

    private:
      std::string feature_name;
    };

    class syl_vowel_feature_function: public feature_function
    {
    public:
      explicit syl_vowel_feature_function(const std::string& name):
        feature_function("syl_vowel_"+name),
        feature_name(name)
      {
      }

      value eval(const item& syl) const
      {
        const phoneme_set& phs=syl.get_relation().get_utterance().get_language().get_phoneme_set();
        const item& syl_in_word=syl.as("Syllable").as("SylStructure");
        item::const_iterator vowel_pos=std::find_if(syl_in_word.begin(),syl_in_word.end(),feature_equals<std::string>("ph_vc","+"));
        return ((vowel_pos==syl_in_word.end())?zero:(phs.get_phoneme(vowel_pos->get("name").as<std::string>())[feature_name]));
      }

    private:
      std::string feature_name;
    };

    class feat_word_stress_pattern: public feature_function
    {
    public:
      feat_word_stress_pattern():
        feature_function("word_stress_pattern")
      {
      }

      value eval(const item& word) const
      {
        stress_pattern default_pattern;
        value default_result(default_pattern);
        value result=word.get("stress_pattern",true);
        if(!result.empty())
          return result;
        const item& word_in_token=word.as("TokStructure");
        if(word_in_token.has_prev()||word_in_token.has_next())
          return default_result;
        const item& token=word_in_token.parent();
        result =token.get("stress_pattern",true);
        return (result.empty()?default_result:result);
      }
    };
  }

  language::language(const language_info& info_):
    phonemes(path::join(info_.get_data_path(),"phonemes.xml")),
    labeller(path::join(info_.get_data_path(),"labelling.xml")),
    tok_fst(path::join(info_.get_data_path(),"tok.fst")),
    key_fst(path::join(info_.get_data_path(),"key.fst")),
    numbers_fst(path::join(info_.get_data_path(),"numbers.fst")),
    gpos_fst(path::join(info_.get_data_path(),"gpos.fst")),
    phrasing_dtree(path::join(info_.get_data_path(),"phrasing.dt")),
    syl_fst(path::join(info_.get_data_path(),"syl.fst")),
    spell_fst(path::join(info_.get_data_path(),"spell.fst")),
    downcase_fst(path::join(info_.get_data_path(),"downcase.fst")),
    udict(info_)
  {
    try
      {
        english_phone_mapping_fst.reset(new fst(path::join(info_.get_data_path(),"english_phone_mapping.fst")));
      }
    catch(const io::open_error& e)
      {
      }
    fst msg_fst(path::join(info_.get_data_path(),"msg.fst"));
    std::vector<std::string> src;
    src.push_back("capital");
    src.push_back("%c");
    msg_fst.translate(src.begin(),src.end(),std::back_inserter(msg_cap_letter));
    src.clear();
    src.push_back("character");
    src.push_back("%d");
    msg_fst.translate(src.begin(),src.end(),std::back_inserter(msg_char_code));
    whitespace_symbols[9]="tab";
    whitespace_symbols[10]="nl";
    whitespace_symbols[13]="cr";
    whitespace_symbols[32]="sp";
    whitespace_symbols[160]="nbsp";
    register_default_features();
  }

  void language::register_default_features()
  {
    const phoneme_feature_set& phfs=get_phoneme_set().get_feature_set();
    for(phoneme_feature_set::const_iterator it(phfs.begin());it!=phfs.end();++it)
      {
        register_feature(smart_ptr<feature_function>(new phoneme_feature_function(*it)));
        register_feature(smart_ptr<feature_function>(new syl_vowel_feature_function(*it)));
      }
    register_feature(smart_ptr<feature_function>(new feat_pos_in_syl));
    register_feature(smart_ptr<feature_function>(new feat_pos_in_syl_bw));
    register_feature(smart_ptr<feature_function>(new feat_syl_numphones));
    register_feature(smart_ptr<feature_function>(new feat_pos_in_word));
    register_feature(smart_ptr<feature_function>(new feat_pos_in_word_bw));
    register_feature(smart_ptr<feature_function>(new feat_seg_pos_in_word));
    register_feature(smart_ptr<feature_function>(new feat_seg_pos_in_word_bw));
    register_feature(smart_ptr<feature_function>(new feat_word_numsyls));
    register_feature(smart_ptr<feature_function>(new feat_syl_in));
    register_feature(smart_ptr<feature_function>(new feat_syl_out));
    register_feature(smart_ptr<feature_function>(new feat_ssyl_in));
    register_feature(smart_ptr<feature_function>(new feat_ssyl_out));
    register_feature(smart_ptr<feature_function>(new feat_asyl_in));
    register_feature(smart_ptr<feature_function>(new feat_asyl_out));
    register_feature(smart_ptr<feature_function>(new feat_syl_vowel));
    register_feature(smart_ptr<feature_function>(new feat_pos_in_phrase));
    register_feature(smart_ptr<feature_function>(new feat_words_out));
    register_feature(smart_ptr<feature_function>(new feat_content_words_in));
    register_feature(smart_ptr<feature_function>(new feat_content_words_out));
    register_feature(smart_ptr<feature_function>(new feat_phrase_numsyls));
    register_feature(smart_ptr<feature_function>(new feat_phrase_numwords));
    register_feature(smart_ptr<feature_function>(new feat_syl_break));
    register_feature(smart_ptr<feature_function>(new feat_word_stress_pattern));
    register_feature(smart_ptr<feature_function>(new feat_phrases_in));
    register_feature(smart_ptr<feature_function>(new feat_phrases_out));
    register_feature(smart_ptr<feature_function>(new feat_syl_coda_size));
  }

  item& language::append_token(utterance& u,const std::string& text) const
  {
    const language_info& lang_info=get_info();
    utf8::uint32_t stress_marker=lang_info.text_settings.stress_marker;
    bool process_stress_marks=lang_info.supports_stress_marks()&&lang_info.text_settings.stress_marker.is_set(true);
    std::vector<utf8::uint32_t> chars;
    std::vector<bool> stress_mask;
    utf8::uint32_t cp;
    std::string::const_iterator it(text.begin());
    while(it!=text.end())
      {
        cp=utf8::next(it,text.end());
        if(process_stress_marks&&
           (!chars.empty())&&
           (chars.back()==stress_marker)&&
           (lang_info.is_vowel_letter(cp)))
          {
            chars.back()=cp;
            stress_mask.back()=true;
          }
        else
          {
            chars.push_back(cp);
            stress_mask.push_back(false);
          }
      }
    std::vector<std::string> tokens;
    if(!tok_fst.translate(chars.begin(),chars.end(),std::back_inserter(tokens)))
      throw tokenization_error();
    relation& token_rel=u.get_relation("Token",true);
    relation& tokstruct_rel=u.get_relation("TokStructure",true);
    item& parent_token=tokstruct_rel.append(token_rel.append());
    parent_token.set("name",text);
    std::string name,pos;
    std::vector<utf8::uint32_t>::const_iterator token_start=chars.begin();
    std::vector<utf8::uint32_t>::const_iterator token_end=token_start;
    stress_pattern stress;
    for(std::vector<std::string>::const_iterator it(tokens.begin());it!=tokens.end();++it)
      {
        if(utf8::distance(it->begin(),it->end())>1)
          {
            if(token_start==token_end)
              throw tokenization_error();
            item& token=parent_token.append_child();
            token.set("name",name);
            pos=*it;
            token.set("pos",pos);
            token.set<verbosity_t>("verbosity",(pos=="sym")?verbosity_silent:verbosity_name);
            if((pos=="word")&&(stress.get_state()!=stress_pattern::undefined))
              token.set("stress_pattern",stress);
            stress.reset();
            name.clear();
            token_start=token_end;
          }
        else
          {
            if(token_end==chars.end())
              throw tokenization_error();
            if(stress_mask[token_end-chars.begin()])
              stress.stress_syllable(1+std::count_if(token_start,token_end,is_vowel_letter(lang_info)));
            utf8::append(*token_end,std::back_inserter(name));
            ++token_end;
          }
      }
    if(!name.empty())
      throw tokenization_error();
    return parent_token.as("Token");
  }

  bool language::decode_as_english(item& token) const
  {
    if(token.has_children())
      return false;
    if(!get_info().use_pseudo_english)
      return false;
    if(english_phone_mapping_fst.get()==0)
      return false;
    const language_list& languages=get_info().get_all_languages();
    language_list::const_iterator lang_it=languages.find("English");
    if(lang_it==languages.end())
      return false;
    std::string pos=token.get("pos").as<std::string>();
    if(pos!="word"&&pos!="lseq"&&pos!="sym"&&pos!="char")
      return false;
    const std::string& name=token.get("name").as<std::string>();
    if(!lang_it->are_all_letters(str::utf8_string_begin(name),str::utf8_string_end(name)))
      return false;
    if(name.length()==1)
      {
        pos="lseq";
        token.set("pos",pos);
}
    lang_it->get_instance().decode(token);
    for(item::iterator it=token.begin();it!=token.end();++it)
      {
        it->set("english",true);
}
    return true;
}

  void language::decode(item& token) const
  {
    if(token.has_children())
      return;
    if(decode_as_english(token))
      return;
    const std::string& token_pos=token.get("pos").as<std::string>();
    const std::string& token_name=token.get("name").as<std::string>();
    if(token_pos=="word")
      decode_as_word(token,token_name);
    else if(token_pos=="lseq")
                decode_as_letter_sequence(token,token_name);
    else if(token_pos=="num")
      decode_as_number(token,token_name);
    else if(token_pos=="dig")
      decode_as_digit_string(token,token_name);
    else if(token_pos=="sym")
      decode_as_character(token,token_name);
    else if((token_pos=="key")||(token_pos=="char"))
      decode_as_key(token,token_name);
    else
      decode_as_special_symbol(token,token_name,token_pos);
  }

  void language::default_decode_as_word(item& token,const std::string& token_name) const
  {
    std::string word_name;
    downcase_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),str::append_string_iterator(word_name));
    item& word=token.append_child();
    word.set("name",word_name);
  }

  void language::decode_as_word(item& token,const std::string& token_name) const
  {
    return default_decode_as_word(token,token_name);
  }

  void language::decode_as_letter_sequence(item& token,const std::string& token_name) const
  {
    default_decode_as_word(token,token_name);
    if(token.has_children())
      token.last_child().set("lseq",true);
  }

    void language::decode_as_number(item& token,const std::string& token_name) const
  {
    numbers_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),token.back_inserter());
  }

  void language::decode_as_digit_string(item& token,const std::string& token_name) const
  {
    str::utf8_string_iterator start=str::utf8_string_begin(token_name);
    str::utf8_string_iterator end=str::utf8_string_end(token_name);
    utf8::uint32_t cp;
    for(str::utf8_string_iterator it=start;it!=end;++it)
      {
        cp=*it;
        spell_fst.translate(&cp,&cp+1,token.back_inserter());
      }
  }

  bool language::decode_as_known_character(item& token,const std::string& token_name) const
  {
    std::vector<std::string> input;
    utf8::uint32_t c=utf8::peek_next(token_name.begin(),token_name.end());
    std::map<utf8::uint32_t,std::string>::const_iterator it=whitespace_symbols.find(c);
    input.push_back((it==whitespace_symbols.end())?token_name:(it->second));
    return spell_fst.translate(input.begin(),input.end(),token.back_inserter());
  }

  void language::decode_as_unknown_character(item& token,const std::string& token_name) const
  {
    for(std::vector<std::string>::const_iterator pos(msg_char_code.begin());pos!=msg_char_code.end();++pos)
      {
        if(*pos=="%d")
          {
            std::ostringstream os;
            os << utf8::peek_next(token_name.begin(),token_name.end());
            std::string digits(os.str());
            numbers_fst.translate(str::utf8_string_begin(digits),str::utf8_string_end(digits),token.back_inserter());
          }
        else
          token.append_child().set("name",*pos);
      }
    token.set("unknown",true);
  }

  void language::indicate_case_if_necessary(item& token) const
  {
    if(!token.has_children())
      return;
    if(token.has_feature("unknown"))
      return;
    if(!(token.get("verbosity").as<verbosity_t>()&verbosity_full_name))
      return;
    const std::string& token_name=token.get("name").as<std::string>();
    std::string::const_iterator it=token_name.begin();
    utf8::uint32_t c=utf8::next(it,token_name.end());
    if(!((it==token_name.end())&&str::isupper(c)))
      return;
    bool prefix=true;
    for(std::vector<std::string>::const_iterator pos(msg_cap_letter.begin());pos!=msg_cap_letter.end();++pos)
      {
        if(*pos=="%c")
          prefix=false;
        else if(prefix)
          token.prepend_child().set("name",*pos);
        else
          token.append_child().set("name",*pos);
      }
  }

  void language::decode_as_character(item& token,const std::string& token_name) const
  {
    if(!decode_as_known_character(token,token_name))
      if(token.get("verbosity").as<verbosity_t>()&verbosity_spell)
        decode_as_unknown_character(token,token_name);
  }

  void language::decode_as_key(item& token,const std::string& token_name) const
  {
    if(!key_fst.translate(str::utf8_string_begin(token_name),str::utf8_string_end(token_name),token.back_inserter()))
      if(token.get("pos").as<std::string>()=="char")
        decode_as_character(token,token_name);
  }

  void language::do_text_analysis(utterance& u) const
  {
    udict.apply_rules(u);
    relation& tokstruct_rel=u.get_relation("TokStructure",true);
    relation& word_rel=u.add_relation("Word");
    for(relation::iterator parent_token_iter=tokstruct_rel.begin();parent_token_iter!=tokstruct_rel.end();++parent_token_iter)
      {
        for(item::iterator token_iter(parent_token_iter->begin());token_iter!=parent_token_iter->end();++token_iter)
          {
            if(token_iter->get("verbosity").as<verbosity_t>()==verbosity_silent)
              continue;
            decode(*token_iter);
            indicate_case_if_necessary(*token_iter);
            std::copy(token_iter->begin(),token_iter->end(),word_rel.back_inserter());
            std::copy(token_iter->begin(),token_iter->end(),parent_token_iter->as("Token").back_inserter());
          }
      }
  }

  void language::do_pos_tagging(utterance& u) const
  {
    relation& word_rel=u.get_relation("Word");
    gpos_fst.translate(word_rel.begin(),word_rel.end(),set_feature_iterator<std::string>("gpos",word_rel.begin(),word_rel.end()));
    for(relation::iterator word_iter(word_rel.begin());word_iter!=word_rel.end();++word_iter)
      {
        if(word_iter->has_feature("lseq"))
          word_iter->set<std::string>("gpos","content");
      }
  }

  break_strength language::get_word_break(const item& word) const
  {
    if(!word.as("Token").has_next())
      {
        const value& strength_val=word.as("Token").parent().get("break_strength",true);
        if(!strength_val.empty())
          {
            break_strength strength=strength_val.as<break_strength>();
            if(strength!=break_default)
              return strength;
          }
      }
    const std::string& brk=phrasing_dtree.predict(word).as<std::string>();
    return ((brk=="NB")?break_none:break_phrase);
  }

  void language::phrasify(utterance& u) const
  {
    relation& word_rel=u.get_relation("Word");
    relation& phrase_rel=u.add_relation("Phrase");
    relation::iterator word_iter=word_rel.begin();
    if(word_iter!=word_rel.end())
      {
        phrase_rel.append();
        do
          {
            phrase_rel.last().append_child(*word_iter);
            break_strength strength=get_word_break(*word_iter);
            ++word_iter;
            if((strength!=break_none)&&(word_iter!=word_rel.end()))
              phrase_rel.append();
          }
        while(word_iter!=word_rel.end());
      }
  }

  std::vector<std::string> language::get_english_word_transcription(const item& word) const
  {
    const language_list& languages=get_info().get_all_languages();
    language_list::const_iterator lang_it=languages.find("English");
    if(lang_it==languages.end())
      throw std::runtime_error("English language not loaded");
    if(english_phone_mapping_fst.get()==0)
      throw std::runtime_error("No phone mapping for english");
    std::vector<std::string> eng_trans=lang_it->get_instance().get_word_transcription(word);
    std::vector<std::string> native_trans;
    english_phone_mapping_fst->translate(eng_trans.begin(),eng_trans.end(),std::back_inserter(native_trans));
    return native_trans;
}

  void language::assign_pronunciation(item& word) const
  {
    std::vector<std::string> transcription((get_info().use_pseudo_english&&word.has_feature("english"))?get_english_word_transcription(word):get_word_transcription(word));
    str::tokenizer<str::is_equal_to> tokenizer("",str::is_equal_to('_'));
    std::string val("1");
    for(std::vector<std::string>::const_iterator it1=transcription.begin();it1!=transcription.end();++it1)
      {
        tokenizer.assign(*it1);
        str::tokenizer<str::is_equal_to>::iterator it2=tokenizer.begin();
        item& seg=word.append_child();
        seg.set("name",*it2);
        for(++it2;it2!=tokenizer.end();++it2)
          {
            seg.set("ph_ext_"+*it2,val);
}
}
  }

  void language::do_g2p(utterance& u) const
  {
    relation& word_rel=u.get_relation("Word");
    relation& seg_rel=u.add_relation("Segment");
    relation& trans_rel=u.add_relation("Transcription");
    for(relation::iterator word_iter=word_rel.begin();word_iter!=word_rel.end();++word_iter)
      {
        item& word=trans_rel.append(*word_iter);
        assign_pronunciation(word);
        if(!word.has_children())
          throw g2p_error();
        std::copy(word.begin(),word.end(),seg_rel.back_inserter());
      }
  }

  void language::syllabify(utterance& u) const
  {
    relation& syl_rel=u.add_relation("Syllable");
    relation& sylstruct_rel=u.add_relation("SylStructure");
    relation& trans_rel=u.get_relation("Transcription");
    std::vector<std::string> result;
    item::iterator seg_start,seg_end,seg_iter;
    std::string seg_name;
    std::size_t n;
    for(relation::iterator word_iter=trans_rel.begin();word_iter!=trans_rel.end();++word_iter)
      {
        item& word_with_syls=sylstruct_rel.append(*word_iter);
        seg_start=word_iter->begin();
        seg_end=word_iter->end();
        if(!syl_fst.translate(seg_start,seg_end,std::back_inserter(result)))
          throw syllabification_error();
        word_with_syls.append_child().set<std::string>("stress","0");
        seg_iter=seg_start;
        for(std::vector<std::string>::const_iterator pos=result.begin();pos!=result.end();++pos)
          {
            if(seg_iter==seg_end)
              throw syllabification_error();
            if(*pos==".")
              word_with_syls.append_child().set<std::string>("stress","0");
            else
              {
                seg_name=seg_iter->get("name").as<std::string>();
                n=seg_name.size()-1;
                if((seg_name[n]=='0')||(seg_name[n]=='1'))
                  {
                    if(seg_name[n]=='1')
                      word_with_syls.last_child().set<std::string>("stress","1");
                    seg_iter->set("name",seg_name.substr(0,n));
                  }
                word_with_syls.last_child().append_child(*seg_iter);
                ++seg_iter;
              }
          }
        std::copy(word_with_syls.begin(),word_with_syls.end(),syl_rel.back_inserter());
        stress_pattern stress=word_with_syls.eval("word_stress_pattern").as<stress_pattern>();
        if(stress.get_state()!=stress_pattern::undefined)
          stress.apply(word_with_syls);
        result.clear();
      }
  }

  void language::insert_pauses(utterance& u) const
  {
    relation& seg_rel=u.get_relation("Segment");
    if(seg_rel.empty())
      return;
    std::string name("pau");
    seg_rel.prepend().set("name",name);
    relation& phrase_rel=u.get_relation("Phrase");
    for(relation::iterator phrase_iter=phrase_rel.begin();phrase_iter!=phrase_rel.end();++phrase_iter)
      {
        phrase_iter->last_child().as("Transcription").last_child().as("Segment").append().set("name",name);
      }
  }

  void language::do_post_lexical_processing(utterance& u) const
  {
    post_lex(u);
  }

  void language::stress_monosyllabic_words(utterance& u) const
  {
    relation& rel=u.get_relation("SylStructure");
    for(relation::iterator word_iter=rel.begin();word_iter!=rel.end();++word_iter)
      {
        item& word=*word_iter;
        if(!word.has_children())
          continue;
        item& syl=word.first_child();
        if(syl.has_next())
          continue;
        if(std::find_if(syl.begin(),syl.end(),feature_equals<std::string>("ph_vc","+"))==syl.end())
          continue;
        syl.set<std::string>("stress","1");
}
  }

  void language::rename_palatalized_consonants(utterance& u) const
  {
    relation& seg_rel=u.get_relation("Segment");
    for(relation::iterator seg_iter(seg_rel.begin());seg_iter!=seg_rel.end();++seg_iter)
      {
        if(seg_iter->eval("ph_vc").as<std::string>()!="-")
          continue;
        const std::string& name=seg_iter->get("name").as<std::string>();
        if(!str::endswith(name,"j"))
          {
            seg_iter->set("pal",std::string("-"));
            continue;
          }
        seg_iter->set("pal",std::string("+"));
        if(name.size()>1)
          seg_iter->set("name",name.substr(0,name.size()-1));
}
}



  language_info::language_info(const std::string& name,const std::string& data_path_,const std::string& userdict_path_):
    enabled("enabled",true),
    all_languages(0),
    userdict_path(userdict_path_),
    use_pseudo_english("use_pseudo_english",true)
  {
    set_name(name);
    set_data_path(data_path_);
  }

  void language_info::register_settings(config& cfg)
  {
    do_register_settings(cfg,"languages."+get_name());
    if(!alpha2_code.empty())
      do_register_settings(cfg,"languages."+alpha2_code);
    if(!alpha3_code.empty())
      do_register_settings(cfg,"languages."+alpha3_code);
  }

  void language_info::do_register_settings(config& cfg,const std::string& prefix)
  {
    cfg.register_setting(enabled,prefix);
    cfg.register_setting(use_pseudo_english,prefix);
    voice_settings.register_self(cfg,prefix);
    text_settings.register_self(cfg,prefix);
  }

  language_list::language_list(const std::vector<std::string>& language_paths,const std::string& userdict_path,const event_logger& logger)
  {
    const std::string tag="language_list";
    register_language<russian_info>("Russian",2);
    register_language<english_info>("English",2);
    register_language<esperanto_info>("Esperanto",1);
    register_language<georgian_info>("Georgian",1);
    register_language<ukrainian_info>("Ukrainian",1);
    register_language<kyrgyz_info>("Kyrgyz",1);
    register_language<tatar_info>("Tatar",1);
    for(std::vector<std::string>::const_iterator it1=language_paths.begin();it1!=language_paths.end();++it1)
      {
        logger.log(tag,RHVoice_log_level_info,std::string("Path: ")+(*it1));
        resource_description desc("language",*it1);
        logger.log(tag,RHVoice_log_level_info,std::string("Language resource: ")+desc.name.get()+std::string(", format: ")+str::to_string(desc.format.get())+std::string(", revision: ")+str::to_string(desc.revision.get()));
        Creators::const_iterator it2=creators.find(language_id(desc.name,desc.format));
        if(it2==creators.end())
          {
            logger.log(tag,RHVoice_log_level_warning,"Unsupported language format");
            continue;
          }
        smart_ptr<language_info> lang=(it2->second)->create(*it1,path::join(userdict_path,desc.name));
        lang->all_languages=this;
        add(lang);
      }
  }

  bool language_search_criteria::operator()(const language_info& info) const
  {
    if(name.empty()||str::equal(info.get_name(),name))
      if(code.empty()||str::equal(info.get_alpha2_code(),code)||str::equal(info.get_alpha3_code(),code))
        return true;
    return false;
  }
}
