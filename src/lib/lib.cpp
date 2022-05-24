/* Copyright (C) 2012, 2013, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <memory>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <functional>

#include "core/engine.hpp"
#include "core/document.hpp"
#include "core/client.hpp"
#include "core/language.hpp"
#include "core/voice.hpp"
#include "core/voice_profile.hpp"
#include "RHVoice.h"

using namespace RHVoice;

struct RHVoice_message_struct: public client
{
  template<typename ch>
  RHVoice_message_struct(const std::shared_ptr<engine>& engine_ptr,const RHVoice_callbacks& callbacks_,const ch* text,unsigned int length,RHVoice_message_type message_type,const RHVoice_synth_params* synth_params,void* user_data_);

  bool play_speech(const short* samples,std::size_t count)
  {
    return callbacks.play_speech(samples,count,user_data);
  }

  event_mask get_supported_events() const;

  bool process_mark(const std::string& name)
  {
    return callbacks.process_mark(name.c_str(),user_data);
  }

  bool sentence_starts(std::size_t position,std::size_t length)
  {
    return callbacks.sentence_starts(position,length,user_data);
  }

  bool sentence_ends(std::size_t position,std::size_t length)
  {
    return callbacks.sentence_ends(position,length,user_data);
  }

  bool word_starts(std::size_t position,std::size_t length)
  {
    return callbacks.word_starts(position,length,user_data);
  }

  bool word_ends(std::size_t position,std::size_t length)
  {
    return callbacks.word_ends(position,length,user_data);
  }

  bool play_audio(const std::string& src)
  {
    return callbacks.play_audio(src.c_str(),user_data);
  }

  bool set_sample_rate(int sr)
  {
    return callbacks.set_sample_rate(sr,user_data);
}

  void done()
  {
    callbacks.done(user_data);
}

  void speak()
  {
    doc_ptr->synthesize();
  }

private:
  RHVoice_message_struct(const RHVoice_message_struct&);
  RHVoice_message_struct& operator=(const RHVoice_message_struct&);

  std::unique_ptr<document> doc_ptr;
  RHVoice_callbacks callbacks;
  void* user_data;
};

struct RHVoice_tts_engine_struct
{
  RHVoice_tts_engine_struct(const RHVoice_init_params* params);

  unsigned int get_number_of_voices() const
  {
    return voice_info_array.size();
  }

  const RHVoice_voice_info* get_voices() const
  {
    return (voice_info_array.empty()?0:&voice_info_array[0]);
  }

  unsigned int get_number_of_voice_profiles() const
  {
    return voice_profile_names_array.size();
  }

  char const * const * get_voice_profiles() const
  {
    return (voice_profile_names_array.empty()?0:&voice_profile_names_array[0]);
  }

  bool are_languages_compatible(const char* language1,const char* language2) const;

  template<typename ch>
  RHVoice_message new_message(const ch* text,unsigned int length,RHVoice_message_type message_type,const RHVoice_synth_params* synth_params,void* user_data) const
  {
    return (new RHVoice_message_struct(engine_ptr,callbacks,text,length,message_type,synth_params,user_data));
  }

private:
  RHVoice_tts_engine_struct(const RHVoice_tts_engine_struct&);
  RHVoice_tts_engine_struct& operator=(const RHVoice_tts_engine_struct&);

  struct convert_voice_info: public std::unary_function<const voice_info&,RHVoice_voice_info>
  {
    RHVoice_voice_info operator()(const voice_info& info) const;
  };

  struct get_voice_profile_name: public std::unary_function<const voice_profile&,const char*>
  {
    const char* operator()(const voice_profile& p) const
    {
      return p.get_name().c_str();
    }
  };

  std::shared_ptr<engine> engine_ptr;
  RHVoice_callbacks callbacks;
  std::vector<RHVoice_voice_info> voice_info_array;
  std::vector<const char*> voice_profile_names_array;
};

RHVoice_tts_engine_struct::RHVoice_tts_engine_struct(const RHVoice_init_params* init_params)
{
  if(!init_params)
    throw std::invalid_argument("No initialization parameters provided");
  if(!init_params->callbacks.play_speech||!init_params->callbacks.set_sample_rate)
    throw std::invalid_argument("A mandatory callback function is not provided by the client");
  engine::init_params engine_params;
  if(init_params->data_path)
    engine_params.data_path.assign(init_params->data_path);
  if(init_params->config_path)
    engine_params.config_path.assign(init_params->config_path);
  if(init_params->resource_paths)
    {
      const char** p=init_params->resource_paths;
      while(*p)
        {
          engine_params.resource_paths.emplace_back(*p);
          ++p;
        }
    }
  engine_ptr=engine::create(engine_params);
  if(engine_ptr->get_voices().empty())
    throw std::runtime_error("No voices");
  std::transform(engine_ptr->get_voices().begin(),engine_ptr->get_voices().end(),std::back_inserter(voice_info_array),convert_voice_info());
  const std::set<voice_profile>& profiles=engine_ptr->get_voice_profiles();
  std::transform(profiles.begin(),profiles.end(),std::back_inserter(voice_profile_names_array),get_voice_profile_name());
  callbacks=init_params->callbacks;
}

RHVoice_voice_info RHVoice_tts_engine_struct::convert_voice_info::operator()(const voice_info& info) const
{
  RHVoice_voice_info result;
  const std::string& alpha2_code=info.get_language()->get_alpha2_code();
  result.language=(alpha2_code.empty()?info.get_language()->get_alpha3_code():alpha2_code).c_str();
  const std::string& country_code=info.get_alpha2_country_code().empty()?info.get_alpha3_country_code():info.get_alpha2_country_code();
  if(!country_code.empty())
    result.country=country_code.c_str();
  else
    result.country=0;
  result.name=info.get_name().c_str();
  result.gender=info.get_gender();
  return result;
}

bool RHVoice_tts_engine_struct::are_languages_compatible(const char* language1,const char* language2) const
{
  if(!(language1&&language2))
    throw std::invalid_argument("A language name is a null pointer");
  const language_list& languages=engine_ptr->get_languages();
  language_search_criteria c;
  c.set_code(language1);
  language_list::const_iterator first=std::find_if(languages.begin(),languages.end(),c);
  if(first==languages.end())
    throw std::invalid_argument("Unknown language");
  c.set_code(language2);
  language_list::const_iterator second=std::find_if(languages.begin(),languages.end(),c);
  if(second==languages.end())
    throw std::invalid_argument("Unknown language");
  return !(first->has_common_letters(*second));
}

template<typename ch>
RHVoice_message_struct::RHVoice_message_struct(const std::shared_ptr<engine>& engine_ptr,const RHVoice_callbacks& callbacks_,const ch* text,unsigned int length,RHVoice_message_type message_type,const RHVoice_synth_params* synth_params,void* user_data_):
  callbacks(callbacks_),
  user_data(user_data_)
{
  if(!text)
    throw std::invalid_argument("Text is a null pointer");
  if(length==0)
    throw std::invalid_argument("Text is an empty string");
  if(!synth_params)
    throw std::invalid_argument("No synthesis parameters");
  if(!synth_params->voice_profile)
    throw std::invalid_argument("The main voice name is mandatory");
  voice_profile profile=engine_ptr->create_voice_profile(synth_params->voice_profile);
  if(profile.empty())
    throw std::invalid_argument("The voice with this name does not exist or has been disabled by the user");
  switch(message_type)
    {
    case RHVoice_message_text:
      doc_ptr=document::create_from_plain_text(engine_ptr,text,text+length,content_text,profile);
      break;
    case RHVoice_message_ssml:
      doc_ptr=document::create_from_ssml(engine_ptr,text,text+length,profile);
      break;
    case RHVoice_message_characters:
      doc_ptr=document::create_from_plain_text(engine_ptr,text,text+length,content_chars,profile);
      break;
    case RHVoice_message_key:
      doc_ptr=document::create_from_plain_text(engine_ptr,text,text+length,content_key,profile);
      break;
    default:
      throw std::invalid_argument("Unknown message type");
    }
  doc_ptr->set_owner(*this);
  doc_ptr->speech_settings.absolute.rate=synth_params->absolute_rate;
  doc_ptr->speech_settings.absolute.pitch=synth_params->absolute_pitch;
  doc_ptr->speech_settings.absolute.volume=synth_params->absolute_volume;
  doc_ptr->speech_settings.relative.rate=synth_params->relative_rate;
  doc_ptr->speech_settings.relative.pitch=synth_params->relative_pitch;
  doc_ptr->speech_settings.relative.volume=synth_params->relative_volume;
  doc_ptr->set_flags(synth_params->flags);
  doc_ptr->verbosity_settings.punctuation_mode=synth_params->punctuation_mode;
  if(synth_params->punctuation_list)
    doc_ptr->verbosity_settings.punctuation_list.set_from_string(synth_params->punctuation_list);
}

event_mask RHVoice_message_struct::get_supported_events() const
{
  event_mask result=0;
  if(callbacks.process_mark)
    result|=event_mark;
  if(callbacks.word_starts)
    result|=event_word_starts;
  if(callbacks.word_ends)
    result|=event_word_ends;
  if(callbacks.sentence_starts)
    result|=event_sentence_starts;
  if(callbacks.sentence_ends)
    result|=event_sentence_ends;
  if(callbacks.play_audio)
    result|=event_audio;
  if(callbacks.done)
    result|=event_done;
  return result;
}

const char* RHVoice_get_version()
{
  return VERSION;
}

RHVoice_tts_engine RHVoice_new_tts_engine(const RHVoice_init_params* init_params)
{
  try
    {
      return (new RHVoice_tts_engine_struct(init_params));
    }
  catch(const std::exception& e)
    {
      return 0;
    }
}

void RHVoice_delete_tts_engine(RHVoice_tts_engine tts_engine)
{
  delete tts_engine;
}

unsigned int RHVoice_get_number_of_voices(RHVoice_tts_engine tts_engine)
{
  return(tts_engine?(tts_engine->get_number_of_voices()):0);
}

const RHVoice_voice_info* RHVoice_get_voices(RHVoice_tts_engine tts_engine)
{
  return (tts_engine?(tts_engine->get_voices()):0);
}

unsigned int RHVoice_get_number_of_voice_profiles(RHVoice_tts_engine tts_engine)
{
  return(tts_engine?(tts_engine->get_number_of_voice_profiles()):0);
}

char const * const * RHVoice_get_voice_profiles(RHVoice_tts_engine tts_engine)
{
  return (tts_engine?(tts_engine->get_voice_profiles()):0);
}

int RHVoice_are_languages_compatible(RHVoice_tts_engine tts_engine,const char* language1,const char* language2)
{
  try
    {
      return (tts_engine?(tts_engine->are_languages_compatible(language1,language2)):0);
    }
  catch(const std::exception& e)
    {
      return 0;
    }
}

RHVoice_message RHVoice_new_message(RHVoice_tts_engine tts_engine,const char* text,unsigned int length,RHVoice_message_type message_type,const RHVoice_synth_params* synth_params,void* user_data)
{
  try
    {
      return (tts_engine?(tts_engine->new_message(text,length,message_type,synth_params,user_data)):0);
    }
  catch(const std::exception& e)
    {
      return 0;
    }
}

RHVoice_message RHVoice_new_message_w(RHVoice_tts_engine tts_engine,const wchar_t* text,unsigned int length,RHVoice_message_type message_type,const RHVoice_synth_params* synth_params,void* user_data)
{
  try
    {
      return (tts_engine?(tts_engine->new_message(text,length,message_type,synth_params,user_data)):0);
    }
  catch(const std::exception& e)
    {
      return 0;
    }
}

void RHVoice_delete_message(RHVoice_message message)
{
  delete message;
}

int RHVoice_speak(RHVoice_message message)
{
  try
    {
      if(message)
        {
          message->speak();
          return 1;
        }
      else
        return 0;
    }
  catch(const std::exception& e)
    {
      return 0;
    }
}
