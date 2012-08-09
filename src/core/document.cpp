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

#include "core/voice.hpp"
#include "core/document.hpp"

namespace RHVoice
{
  void sentence::append_token::execute(utterance& u) const
  {
    item& token=u.get_language().append_token(u,name);
    token.set("position",position);
    token.set("length",length);
    u.get_relation("Event",true).append(token);
  }

  void sentence::append_chars::execute(utterance& u) const
  {
    const language_info& lang_info=u.get_language().get_info();
    item& parent_token=u.get_relation("TokStructure",true).append();
    parent_token.set("name",name);
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
        token.set<std::string>("pos",(lang_info.is_letter(c)?"lseq":"sym"));
        token.set("verbosity",verbosity_level);
      }
  }

  sentence::sentence(const document* parent_):
    parent(parent_),
    rate(1),
    pitch(1),
    volume(1),
    length(0)
    {
    }

  bool sentence::token_starts_new_sentence(const std::vector<utf8::uint32_t>& token,const tts_markup& markup_info) const
  {
    if(empty())
      return false;
    if(!markup_info.autosplit_sentences)
      return false;
    std::size_t newline_count=str::count_newlines(trailing_whitespace.begin(),trailing_whitespace.end());
    if(newline_count>1)
      return true;
    std::vector<utf8::uint32_t>::const_iterator final_punctuation_start=std::find_if(last_token.rbegin(),last_token.rend(),std::not1(str::is_punct())).base();
    if(final_punctuation_start==last_token.end())
      return false;
    if(last_token.back()=='.')
      {
        if(str::isalpha(token[0]))
          {
            if(final_punctuation_start==last_token.begin())
              return true;
            else
              {
                std::vector<utf8::uint32_t>::const_iterator last_char_pos=final_punctuation_start-1;
                if(str::isalpha(*last_char_pos))
                  {
                    if(last_char_pos==last_token.begin())
                      return false;
                    else
                      return str::isupper(token[0]);
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
        for(std::vector<utf8::uint32_t>::const_iterator it(final_punctuation_start);it!=last_token.end();++it)
          {
            cp=*it;
            if((cp=='.')||(cp=='\?')||(cp=='!')||(cp==';')||(cp==':'))
              return true;
          }
        return false;
      }
  }

  language_list::const_iterator sentence::determine_token_language(const std::vector<utf8::uint32_t>& token) const
  {
    language_list::const_iterator result;
    if(parent->has_main_language())
      {
        language_list::const_iterator main_language=parent->get_main_language();
        if(parent->has_extra_language())
          {
            language_list::const_iterator extra_language=parent->get_extra_language();
            std::size_t count1=main_language->count_letters_in_text(token.begin(),token.end());
            std::size_t count2=extra_language->count_letters_in_text(token.begin(),token.end());
            if(count1||count2)
              result=(count2>count1)?extra_language:main_language;
          }
        else
          result=main_language;
      }
    return result;
  }

  language_voice_pair sentence::get_language_and_voice_from_markup(const tts_markup& markup_info) const
  {
    language_voice_pair result;
    const language_list& languages=parent->get_engine().get_languages();
    const voice_list& voices=parent->get_engine().get_voices();
    if(!markup_info.language_criteria.empty())
      {
        language_list::const_iterator requested_language=std::find_if(languages.begin(),languages.end(),markup_info.language_criteria);
        if(requested_language!=languages.end())
          result.first=requested_language;
      }
    if(!markup_info.voice_criteria.empty())
      {
        voice_search_criteria voice_criteria=markup_info.voice_criteria;
        voice_criteria.set_language(result.first);
        voice_list::const_iterator requested_voice=std::find_if(voices.begin(),voices.end(),voice_criteria);
        if(requested_voice!=voices.end())
          {
            result.first=requested_voice->get_language();
            result.second=requested_voice;
          }
      }
    return result;
  }

  std::auto_ptr<utterance> sentence::create_utterance() const
  {
    std::auto_ptr<utterance> u;
    const language_list& languages=parent->get_engine().get_languages();
    language_list::const_iterator current_language=language_and_voice.first;
    const voice_list& voices=parent->get_engine().get_voices();
    voice_list::const_iterator current_voice=language_and_voice.second;
    if(current_language==language_list::const_iterator())
      {
        if(parent->has_main_language())
          current_language=parent->get_main_language();
        else
          current_language=languages.begin();
      }
    if(current_voice==voice_list::const_iterator())
      {
        if(parent->has_main_voice())
          {
            if(current_language==parent->get_main_language())
              current_voice=parent->get_main_voice();
            else if(parent->has_extra_voice()&&(current_language==parent->get_extra_language()))
              current_voice=parent->get_extra_voice();
          }
        if(current_voice==voice_list::const_iterator())
          {
            voice_search_criteria c;
            c.set_language(current_language);
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
    u->set_absolute_rate(parent->speech_settings.absolute.rate);
    u->set_relative_rate(parent->speech_settings.relative.rate*rate);
    u->set_absolute_pitch(parent->speech_settings.absolute.pitch);
    u->set_relative_pitch(parent->speech_settings.relative.pitch*pitch);
    u->set_absolute_volume(parent->speech_settings.absolute.volume);
    u->set_relative_volume(parent->speech_settings.relative.volume*volume);
    for(std::list<command_ptr>::const_iterator it(commands.begin());it!=commands.end();++it)
      {
        (*it)->execute(*u);
      }
    language_ref.do_text_analysis(*u);
    language_ref.do_pos_tagging(*u);
    language_ref.phrasify(*u);
    language_ref.transcribe(*u);
    language_ref.syllabify(*u);
    language_ref.insert_pauses(*u);
    language_ref.do_post_lexical_processing(*u);
    return u;
  }

  void document::synthesize()
  {
    if(!has_owner())
      return;
    std::auto_ptr<utterance> u;
    for(const_iterator it(begin());it!=end();++it)
      {
        u=it->create_utterance();
        if((u.get()!=0)&&(u->has_voice()))
          if(!(u->get_voice().synthesize(*u,get_owner())))
            break;
      }
  }
}
