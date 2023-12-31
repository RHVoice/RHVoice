/* Copyright (C) 2012, 2014, 2019, 2020, 2021  Olga Yakovleva <olga@rhvoice.org> */
/* Copyright (C) 2022 Non-Routine LLC.  <lp@louderpages.org> */

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

#include "core/voice.hpp"
#include "core/document.hpp"

namespace RHVoice
{
  void sentence::append_token::execute(utterance& u) const
  {
    item& token=u.get_language().append_token(u,name, eos);
    token.set("whitespace",whitespace);
    token.set("position",position);
    token.set("length",length);
    u.get_relation("Event",true).append(token);
  }

  void sentence::append_chars::execute(utterance& u) const
  {
    u.get_language().on_token_break(u);
    const language_info& lang_info=u.get_language().get_info();
    item& parent_token=u.get_relation("TokStructure",true).append();
    parent_token.set("name",name);
    parent_token.set("whitespace",whitespace);
    parent_token.set("position",position);
    parent_token.set("length",length);
    u.get_relation("Token",true).append(parent_token);
    u.get_relation("Event",true).append(parent_token);
    std::string::const_iterator pos=name.begin();
    std::string::const_iterator prev_pos;
    utf8::uint32_t c;
    while(pos!=name.end())
      {
        prev_pos=pos;
        c=utf8::next(pos,name.end());
        item& token=parent_token.append_child();
        token.set("name",std::string(prev_pos,pos));
        token.set<std::string>("pos","sym");
        token.set("verbosity",verbosity_level);
      }
  }

  void sentence::append_emoji::execute(utterance& u) const
  {
    item& token=u.get_language().supports_emoji()?u.get_language().append_emoji(u,name):u.get_language().append_token(u,name, eos);
    token.set("whitespace",whitespace);
    token.set("position",position);
    token.set("length",length);
    u.get_relation("Event",true).append(token);
  }

  void sentence::append_key::execute(utterance& u) const
  {
    u.get_language().on_token_break(u);
    const language_info& lang_info=u.get_language().get_info();
    item& parent_token=u.get_relation("TokStructure",true).append();
    parent_token.set("name",name);
    parent_token.set("whitespace",whitespace);
    parent_token.set("position",position);
    parent_token.set("length",length);
    u.get_relation("Token",true).append(parent_token);
    u.get_relation("Event",true).append(parent_token);
    str::tokenizer<str::is_equal_to> tokenizer(parent_token.get("name").as<std::string>(),str::is_equal_to('_'));
    for(str::tokenizer<str::is_equal_to>::iterator it=tokenizer.begin();it!=tokenizer.end();++it)
      {
        const std::string& name=*it;
        item& token=parent_token.append_child();
        token.set("name",name);
        token.set<std::string>("pos","key");
        token.set<verbosity_t>("verbosity",verbosity_name);
        std::string::const_iterator pos=name.begin();
        utf8::uint32_t cp=utf8::next(pos,name.end());
        if(pos==name.end())
          {
            token.set<std::string>("pos",lang_info.is_letter(cp)?"lseq":"char");
            token.set<verbosity_t>("verbosity",verbosity_name|verbosity_spell);
          }
      }
  }

  void sentence::append_phones::execute(utterance& u) const
  {
    u.get_language().on_token_break(u);
    item& parent_token=u.get_relation("TokStructure",true).append();
    parent_token.set("name",name);
    u.get_relation("Token",true).append(parent_token);
    parent_token.set("whitespace",whitespace);
    parent_token.set("position",position);
    parent_token.set("length",length);
    u.get_relation("Event",true).append(parent_token);
    item& token=parent_token.append_child();
    token.set("name",name);
    token.set<std::string>("pos","ph");
    token.set<verbosity_t>("verbosity",verbosity_name);
  }

  sentence::sentence(document* parent_):
    parent(parent_),
    rate(1),
    pitch(1),
    volume(1),
    language_and_voice(parent->get_engine().get_languages().end(),parent->get_engine().get_voices().end()),
    length(0),
    num_tokens(0),
    en_id(parent->get_voice_profile())
    {
    }

  bool sentence::next_token_starts_new_sentence(const tts_markup& markup_info) const
  {
    if(empty())
      return false;
    if(!markup_info.autosplit_sentences)
      return false;
    std::size_t newline_count=str::count_newlines(prev_token.whitespace.begin(),prev_token.whitespace.end());
    if(newline_count>1)
      return true;
    if(prev_token.type==content_key)
      return false;
    std::vector<utf8::uint32_t>::const_iterator final_punctuation_start=std::find_if(prev_token.text.rbegin(),prev_token.text.rend(),std::not1(str::is_punct())).base();
    if(final_punctuation_start==prev_token.text.end())
      return false;
    if(prev_token.text.back()=='.')
      {
        if((next_token.type!=content_key)&&str::isalpha(next_token.text[0]))
          {
            if(final_punctuation_start==prev_token.text.begin())
              return true;
            else
              {
                std::vector<utf8::uint32_t>::const_iterator last_char_pos=final_punctuation_start-1;
                bool is_alpha=str::isalpha(*last_char_pos);
                bool is_digit=str::isdigit(*last_char_pos);
                if(is_alpha || is_digit)
                  {
                    if(is_alpha && last_char_pos==prev_token.text.begin())
                      return false;
                    else
                      {
                        bool unicase=false;
                        if(language_and_voice.first!=parent->get_engine().get_languages().end())
                          unicase=language_and_voice.first->has_unicase_alphabet();
                        return (unicase||str::isupper(next_token.text[0]));
                      }
                  }
                else
                  return true;
              }
          }
        else
          return true;
      }
    else
      {
        utf8::uint32_t cp;
        for(std::vector<utf8::uint32_t>::const_iterator it(final_punctuation_start);it!=prev_token.text.end();++it)
          {
            cp=*it;
	    if(language_and_voice.first!=parent->get_engine().get_languages().end())
	      {
		if(language_and_voice.first->get_instance().is_eos_punct(cp))
		  return true;
	      }
            if((cp=='.')||(cp=='\?')||(cp=='!')||(cp==';')||(cp==':'))
              return true;
          }
        return false;
      }
  }

  voice_list::const_iterator sentence::determine_next_token_voice()
  {
    voice_list::const_iterator no_result=parent->get_engine().get_voices().end();
    if(en_id.is_enabled())
      {
	en_id.add_word(next_token.text.begin(), next_token.text.end());
	return no_result;
      }
    const voice_profile& profile=parent->get_voice_profile();
    if(profile.empty())
      return no_result;
    if(profile.voice_count()==1)
      return profile.primary();
    voice_profile::iterator voice_in_profile=profile.end();
    if(next_token.type==content_key)
      {
        if((next_token.text.size()==1)||(*(next_token.text.end()-2)=='_'))
          voice_in_profile=profile.voice_for_text(next_token.text.end()-1,next_token.text.end());
      }
    else
      voice_in_profile=profile.voice_for_text(next_token.text.begin(),next_token.text.end());
    if(voice_in_profile!=profile.end())
      return *voice_in_profile;
    else if(parent->get_engine().prefer_primary_language)
      return profile.primary();
    else
      return no_result;
  }

  language_voice_pair sentence::get_language_and_voice_from_markup(const tts_markup& markup_info) const
  {
    language_voice_pair result;
    const language_list& languages=parent->get_engine().get_languages();
    const voice_list& voices=parent->get_engine().get_voices();
    result.first=languages.end();
    result.second=voices.end();
    if(!markup_info.language_criteria.empty())
      {
        language_list::const_iterator requested_language=std::find_if(languages.begin(),languages.end(),markup_info.language_criteria);
        if(requested_language!=languages.end())
          result.first=requested_language;
      }
    if(!markup_info.voice_criteria.empty())
      {
        voice_search_criteria voice_criteria=markup_info.voice_criteria;
        if(result.first!=languages.end())
          voice_criteria.set_language(*(result.first));
        voice_list::const_iterator requested_voice=std::find_if(voices.begin(),voices.end(),voice_criteria);
        if(requested_voice!=voices.end())
          {
            result.first=requested_voice->get_language();
            result.second=requested_voice;
          }
      }
    return result;
  }

  std::unique_ptr<utterance> sentence::new_utterance() const
  {
    const voice_profile& profile=parent->get_voice_profile();
    int lang_index=0;
    if(en_id.is_enabled())
      {
	auto r=en_id.get_result();
	if(r>=0)
	  lang_index=r;
      }
    std::unique_ptr<utterance> u;
    const language_list& languages=parent->get_engine().get_languages();
    language_list::const_iterator current_language=language_and_voice.first;
    const voice_list& voices=parent->get_engine().get_voices();
    voice_list::const_iterator current_voice=language_and_voice.second;
    if(current_language==languages.end())
      {
        if(!profile.empty())
          current_language=profile.get_voice(lang_index)->get_language();
        else
          current_language=languages.begin();
      }
    if(current_voice==voices.end())
      {
        if(!profile.empty())
          {
            voice_profile::iterator voice_in_profile=profile.voice_for_language(current_language);
            if(voice_in_profile!=profile.end())
              current_voice=*voice_in_profile;
          }
        if(current_voice==voices.end())
          {
            voice_search_criteria c;
            c.set_language(*current_language);
            c.set_preferred();
            current_voice=std::find_if(voices.begin(),voices.end(),c);
            if(current_voice==voices.end())
              {
                c.clear_preferred();
                current_voice=std::find_if(voices.begin(),voices.end(),c);
              }
          }
      }
    const language& language_ref=current_language->get_instance();
    u.reset(new utterance(language_ref));
    if(current_voice!=voices.end())
      u->set_voice(current_voice->get_instance());
    return u;
  }

  void sentence::apply_speech_settings(utterance& u) const
  {
    u.set_absolute_rate(parent->speech_settings.absolute.rate);
    u.set_relative_rate(parent->speech_settings.relative.rate*rate);
    u.set_absolute_pitch(parent->speech_settings.absolute.pitch);
    u.set_relative_pitch(parent->speech_settings.relative.pitch*pitch);
    u.set_absolute_volume(parent->speech_settings.absolute.volume);
    u.set_relative_volume(parent->speech_settings.relative.volume*volume);
  }

  void sentence::execute_commands(utterance& u) const
  {
    for(auto it=commands.rbegin(); it!=commands.rend(); ++it)
      {
        if((*it)->has_text())
          {
            (*it)->set_eos();
            break;
          }
      }
    for(std::list<command_ptr>::const_iterator it(commands.begin());it!=commands.end();++it)
      {
        (*it)->execute(u);
      }
  }

  void sentence::apply_language_processing(utterance& u) const
  {
    const language& language_ref=u.get_language();
    language_ref.do_text_analysis(u);
    language_ref.do_pos_tagging(u);
    language_ref.phrasify(u);
    language_ref.detect_utt_type(u);
    language_ref.do_g2p(u);
    language_ref.syllabify(u);
    language_ref.insert_pauses(u);
    language_ref.do_post_lexical_processing(u);
    language_ref.do_syl_accents(u);
    language_ref.set_pitch_modifications(u);
    language_ref.set_duration_modifications(u);
  }

  void sentence::apply_verbosity_settings(utterance& u) const
  {
    if((parent->verbosity_settings.punctuation_mode!=RHVoice_punctuation_none)||
       (parent->verbosity_settings.capitals_mode!=RHVoice_capitals_off))
      {
        relation& tokstruct_rel=u.get_relation("TokStructure");
        for(relation::iterator parent_token_iter=tokstruct_rel.begin();parent_token_iter!=tokstruct_rel.end();++parent_token_iter)
          {
            if(parent_token_iter->has_feature("emoji"))
              continue;
            for(item::iterator token_iter=parent_token_iter->begin();token_iter!=parent_token_iter->end();++token_iter)
              {
                const std::string& pos=token_iter->get("pos").as<std::string>();
                const std::string& name=token_iter->get("name").as<std::string>();
                utf8::uint32_t c=utf8::peek_next(name.begin(),name.end());
                if((pos=="sym")&&
                   (((parent->verbosity_settings.punctuation_mode==RHVoice_punctuation_all)&&
                     (str::ispunct(c)||str::issym(c)))||
                    ((parent->verbosity_settings.punctuation_mode==RHVoice_punctuation_some)&&
                     parent->verbosity_settings.punctuation_list.includes(c))))
                      token_iter->set<verbosity_t>("verbosity",verbosity_name);
                if(parent->verbosity_settings.capitals_mode!=RHVoice_capitals_off)
                  {
                    verbosity_t verbosity_level=token_iter->get("verbosity").as<verbosity_t>();
                    if((verbosity_level&verbosity_spell)&&
                       (!(verbosity_level&verbosity_full_name))&&
                       str::isupper(c))
                      {
                        if(parent->verbosity_settings.capitals_mode==RHVoice_capitals_pitch)
                          verbosity_level|=verbosity_pitch;
                        else if(parent->verbosity_settings.capitals_mode==RHVoice_capitals_sound)
                          verbosity_level|=verbosity_sound;
                        else
                          verbosity_level|=verbosity_full_name;
                        token_iter->set("verbosity",verbosity_level);
                      }
                  }
              }
          }
      }
  }

  void sentence::set_spell_single_symbol(utterance& u) const
  {
    relation& tokstruct=u.get_relation("TokStructure");
    if(tokstruct.empty())
      return;
    item& token=tokstruct.first();
    if(token.has_next())
      return;
    item& subtoken=token.first_child();
    if(subtoken.has_next())
      return;
    const std::string& pos=subtoken.get("pos").as<std::string>();
    if(pos!="sym")
      return;
    verbosity_t level=subtoken.get("verbosity").as<verbosity_t>();
    if(level!=verbosity_silent)
      return;
    level=verbosity_name;
    subtoken.set("verbosity",level);
  }

  std::unique_ptr<utterance> sentence::create_utterance(sentence_position pos) const
  {
    std::unique_ptr<utterance> u=new_utterance();
    u->set_bilingual_enabled(parent->enable_bilingual);
    apply_speech_settings(*u);
    execute_commands(*u);
    u->get_language().tokenize(*u);
    if(pos==sentence_position_single)
      set_spell_single_symbol(*u);
    apply_verbosity_settings(*u);
    apply_language_processing(*u);
    u->set_quality(parent->quality);
    u->set_flags(parent->get_flags());
    return u;
  }

  bool sentence::has_text() const
  {
    for(std::list<command_ptr>::const_iterator it(commands.begin());it!=commands.end();++it)
      {
        if((*it)->has_text())
          return true;
      }
    return false;
  }

  bool sentence::notify_client()
  {
    for(std::list<command_ptr>::const_iterator it(commands.begin());it!=commands.end();++it)
      {
        if(!((*it)->notify_client(parent->get_owner())))
          return false;
      }
    return true;
  }

  void document::synthesize()
  {
    if(!has_owner())
      return;
    std::unique_ptr<utterance> u;
    sentence_position pos=sentence_position_initial;
    for(iterator it(begin());it!=end();++it)
      {
        if(!(it->has_text()))
          {
            if(it->notify_client())
              continue;
            else
              return;
          }
        const_iterator tmp_it=it;
        ++tmp_it;
        if(tmp_it==end())
          {
            if(pos==sentence_position_initial)
              pos=sentence_position_single;
            else
              pos=sentence_position_final;
          }
        u=it->create_utterance(pos);
        if((u.get()!=0)&&(u->has_voice()))
          if(!(u->get_voice().synthesize(*u,get_owner())))
            return;
        pos=sentence_position_middle;
      }
    if(owner->get_supported_events()&event_done)
      owner->done();
  }
}
