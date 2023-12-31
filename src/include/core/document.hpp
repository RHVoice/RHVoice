/* Copyright (C) 2012, 2019, 2020, 2021  Olga Yakovleva <olga@rhvoice.org> */
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

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>

#include "engine.hpp"
#include "voice_profile.hpp"
#include "utterance.hpp"
#include "utf.hpp"
#include "str.hpp"
#include "tts_markup.hpp"
#include "ssml.hpp"
#include "client.hpp"
#include "params.hpp"
#include "quality_setting.hpp"
#include "emoji.hpp"
#include "english_id.hpp"

#ifndef RHVOICE_DOCUMENT_HPP
#define RHVOICE_DOCUMENT_HPP
namespace RHVoice
{
  typedef std::pair<language_list::const_iterator,voice_list::const_iterator> language_voice_pair;
  class document;

  struct text_token
  {
    content_type type;
    std::vector<utf8::uint32_t> text,whitespace;
    std::size_t position,length;

    text_token():
      type(content_text),
      position(0),
      length(0)
    {
    }
  };

  enum sentence_position
    {
      sentence_position_initial,
      sentence_position_final,
      sentence_position_middle,
      sentence_position_single
    };

  class sentence
  {
  private:
    enum command_type
      {
        command_token,
        command_mark,
        command_audio,
        command_break,
        command_phones
      };

    class abstract_command: public utterance_function
    {
    public:
      virtual command_type get_type() const=0;

      virtual bool has_text() const
      {
        return true;
      }

      virtual bool notify_client(client& c) const
      {
        return true;
      }

      virtual void set_eos() {}
    };

    typedef std::shared_ptr<abstract_command> command_ptr;

    template<command_type type>
    class command: public abstract_command
    {
    public:
      command_type get_type() const
      {
        return type;
      }
    };

    class append_token: public command<command_token>
    {
    public:
      explicit append_token(const text_token& prev_token,const text_token& token):
        position(token.position),
        length(token.length)
    {
      std::copy(token.text.begin(),token.text.end(),str::utf8_inserter(std::back_inserter(name)));
      if(!prev_token.whitespace.empty())
        std::copy(prev_token.whitespace.begin(),prev_token.whitespace.end(),str::utf8_inserter(std::back_inserter(whitespace)));
    }

      void set_eos() {eos=true;}

      void execute(utterance& u) const;

    protected:
      std::string name,whitespace;
      std::size_t position,length;
      bool eos{false};
    };

    class append_chars: public append_token
    {
    public:
      explicit append_chars(const text_token& prev_token,const text_token& token):
        append_token(prev_token,token),
        verbosity_level(verbosity_spell|((token.type==content_glyphs)?verbosity_full_name:verbosity_name))
    {
    }

      void execute(utterance& u) const;

    private:
      verbosity_t verbosity_level;
    };

    class append_key: public append_token
    {
    public:
      explicit append_key(const text_token& prev_token,const text_token& token):
        append_token(prev_token,token)
    {
    }

      void execute(utterance& u) const;
    };

    class append_emoji: public append_token
    {
    public:
      explicit append_emoji(const text_token& prev_token,const text_token& token):
        append_token(prev_token,token)
    {
    }

      void execute(utterance& u) const;
    };

    class append_mark: public command<command_mark>
    {
    public:
      explicit append_mark(const std::string& mark_name):
        name(mark_name)
      {
      }

      void execute(utterance& u) const
      {
        u.get_relation("Event",true).append().set("mark",name);
      }

      bool has_text() const
      {
        return false;
      }

      bool notify_client(client& c) const
      {
        if(c.get_supported_events()&event_mark)
          return c.process_mark(name);
        else
          return true;
      }

    private:
      std::string name;
    };

    class append_audio: public command<command_audio>
    {
    public:
      explicit append_audio(const std::string& audio_src):
        src(audio_src)
      {
      }

      void execute(utterance& u) const
      {
        u.get_relation("Event",true).append().set("audio",src);
        u.get_language().on_token_break(u);
      }

      bool has_text() const
      {
        return false;
      }

      bool notify_client(client& c) const
      {
        return c.play_audio(src);
      }

    private:
      std::string src;
    };

    class append_break: public command<command_break>
    {
    public:
      explicit append_break(break_strength strength_):
        strength(strength_)
      {
      }

      void execute(utterance& u) const
      {
        if(u.has_relation("TokStructure"))
          {
            u.get_relation("TokStructure").last().set("break_strength",strength);
            if(strength>=break_phrase)
              u.get_language().on_token_break(u);
          }
      }

    private:
      break_strength strength;
    };

    class append_phones: public append_token
    {
    public:
      explicit append_phones(const text_token& prev_token,const text_token& token):
        append_token(prev_token,token)

    {
    }

      void execute(utterance& u) const;
    };

    std::list<command_ptr> commands;
    text_token prev_token,next_token;
    document* parent;
    double rate,pitch,volume;
    language_voice_pair language_and_voice;
    std::size_t length,num_tokens;
    english_id en_id;

    static const std::size_t max_token_length=200;
    static const std::size_t max_sentence_length=1000;
    static const std::size_t max_tokens=100;

  public:
    explicit sentence(document* parent_);

    bool empty() const
    {
      return commands.empty();
    }

    std::unique_ptr<utterance> create_utterance(sentence_position position) const;

    template<typename text_iterator>
    text_iterator add_text(const text_iterator& text_start,const text_iterator& text_end,const tts_markup& markup_info);

    void add_mark(const std::string& name)
    {
      commands.push_back(command_ptr(new append_mark(name)));
    }

    void add_audio(const std::string& src)
    {
      commands.push_back(command_ptr(new append_audio(src)));
    }

    void add_break(break_strength strength)
    {
      commands.push_back(command_ptr(new append_break(strength)));
    }

    bool has_text() const;
    bool notify_client();

  private:
    template<typename text_iterator>
    text_iterator skip_whitespace(text_iterator text_start,text_iterator text_end,const tts_markup& markup_info);
    template<typename text_iterator>
    text_iterator get_next_token(text_iterator text_start,text_iterator text_end,const tts_markup& markup_info);
    bool next_token_starts_new_sentence(const tts_markup& markup_info) const;
    voice_list::const_iterator determine_next_token_voice();
    language_voice_pair get_language_and_voice_from_markup(const tts_markup& markup_info) const;
    std::unique_ptr<utterance> new_utterance() const;
    void execute_commands(utterance& u) const;
    void apply_speech_settings(utterance& u) const;
    void set_spell_single_symbol(utterance& u) const;
    void apply_verbosity_settings(utterance& u) const;
    void apply_language_processing(utterance& u) const;
  };

  class document
  {
  public:
    speech_params speech_settings;
    verbosity_params verbosity_settings;
    quality_setting quality;
    bool_property enable_bilingual{"enable_bilingual", true};

    explicit document(const std::shared_ptr<engine>& engine_ptr_,const voice_profile& profile_=voice_profile()):
      engine_ptr(engine_ptr_),
      owner(0),
      current_sentence(sentences.end()),
      profile(profile_),
      flags(0)
    {
      verbosity_settings.default_to(engine_ptr->verbosity_settings);
      quality.default_to(engine_ptr->quality);
      enable_bilingual.default_to(engine_ptr->enable_bilingual);
    }

    const engine& get_engine() const
    {
      return *engine_ptr;
    }

    bool has_owner() const
    {
      return owner;
    }

    void set_owner(client& owner_)
    {
      owner=&owner_;
    }

    client& get_owner()
    {
      return *owner;
    }

    const client& get_owner() const
    {
      return *owner;
    }

    voice_profile& get_voice_profile()
    {
      return profile;
    }

    const voice_profile& get_voice_profile() const
    {
      return profile;
    }

    int get_flags() const
    {
      return flags;
}

    void set_flags(int value)
    {
      flags=value;
}

    template<typename some_iterator>
    static std::unique_ptr<document> create_from_plain_text(const std::shared_ptr<engine>& engine_ptr,const some_iterator& text_start,const some_iterator& text_end,content_type say_as=content_text,const voice_profile& profile=voice_profile())
    {
      #ifdef _MSC_VER
      return create_from_plain_text(engine_ptr,text_start,text_end,say_as,profile,std::iterator_traits<some_iterator>::iterator_category());
            #else
      return create_from_plain_text(engine_ptr,text_start,text_end,say_as,profile,typename std::iterator_traits<some_iterator>::iterator_category());
      #endif
    }

    template<typename input_iterator>
    static std::unique_ptr<document> create_from_ssml(const std::shared_ptr<engine>& engine_ptr,const input_iterator& text_start,const input_iterator& text_end,const voice_profile& profile=voice_profile());

    typedef std::list<sentence>::iterator iterator;
    typedef std::list<sentence>::const_iterator const_iterator;

    iterator begin()
    {
      return sentences.begin();
    }

    iterator end()
    {
      return sentences.end();
    }

    const_iterator begin() const
    {
      return sentences.begin();
    }

    const_iterator end() const
    {
      return sentences.end();
    }

    template<typename text_iterator>
    void add_text(const text_iterator& text_start,const text_iterator& text_end,const tts_markup& markup_info=tts_markup());

    void add_mark(const std::string& name)
    {
      get_current_sentence().add_mark(name);
    }

    void add_audio(const std::string& src)
    {
      get_current_sentence().add_audio(src);
    }

    void add_break(break_strength strength)
    {
      if(strength==break_sentence)
        finish_sentence();
      else
        get_current_sentence().add_break(strength);
    }

    void finish_sentence()
    {
      current_sentence=sentences.end();
    }

    void synthesize();

  private:
    sentence& get_current_sentence()
    {
      if(current_sentence==sentences.end())
        current_sentence=sentences.insert(sentences.end(),sentence(this));
      return *current_sentence;
    }

    template<typename input_iterator>
    static std::unique_ptr<document> create_from_plain_text(const std::shared_ptr<engine>& engine_ptr,const input_iterator& text_start,const input_iterator& text_end,content_type say_as,const voice_profile& profile,std::input_iterator_tag)
    {
      #ifdef _MSC_VER
      typedef std::iterator_traits<input_iterator>::value_type char_type;
      #else
      typedef typename std::iterator_traits<input_iterator>::value_type char_type;
      #endif
      std::vector<char_type> tmp_buf(text_start,text_end);
      tts_markup m;
      m.say_as=say_as;
      std::unique_ptr<document> doc_ptr(new document(engine_ptr,profile));
      #ifdef _MSC_VER
      typedef utf::text_iterator<std::vector<char_type>::const_iterator> char_iterator;
      #else
      typedef utf::text_iterator<typename std::vector<char_type>::const_iterator> char_iterator;
      #endif
      doc_ptr->add_text(char_iterator(tmp_buf.begin(),tmp_buf.begin(),tmp_buf.end()),char_iterator(tmp_buf.end(),tmp_buf.begin(),tmp_buf.end()),m);
      return doc_ptr;
    }

    template<typename forward_iterator>
    static std::unique_ptr<document> create_from_plain_text(const std::shared_ptr<engine>& engine_ptr,const forward_iterator& text_start,const forward_iterator& text_end,content_type say_as,const voice_profile& profile,std::forward_iterator_tag)
    {
      std::unique_ptr<document> doc_ptr(new document(engine_ptr,profile));
      typedef utf::text_iterator<forward_iterator> text_iterator;
      tts_markup m;
      m.say_as=say_as;
      doc_ptr->add_text(text_iterator(text_start,text_start,text_end),text_iterator(text_end,text_start,text_end),m);
      return doc_ptr;
    }

    std::shared_ptr<engine> engine_ptr;
    client* owner;
    std::list<sentence> sentences;
    std::list<sentence>::iterator current_sentence;
    voice_profile profile;
    int flags;
  };

  template<typename text_iterator>
  text_iterator sentence::skip_whitespace(text_iterator text_start,text_iterator text_end,const tts_markup& markup_info)
  {
    if(markup_info.say_as!=content_text)
      return text_start;
    text_iterator whitespace_end=std::find_if(text_start,text_end,std::not1(str::is_space()));
    if(whitespace_end!=text_start)
      prev_token.whitespace.insert(prev_token.whitespace.end(),text_start,whitespace_end);
    return whitespace_end;
  }

  template<typename text_iterator>
  text_iterator sentence::get_next_token(text_iterator text_start,text_iterator text_end,const tts_markup& markup_info)
  {
    next_token.type=markup_info.say_as;
    next_token.text.clear();
    next_token.whitespace.clear();
    next_token.position=text_start.offset();
    next_token.length=0;
    if(text_start==text_end)
      return text_start;
    text_iterator token_end=text_start;
    if(markup_info.say_as==content_key || markup_info.say_as==content_phones)
      token_end=text_end;
    else if((markup_info.say_as!=content_text)&&str::isspace(*text_start))
      ++token_end;
    else
      {
        emoji_scanner es;
        for(std::size_t i=0;(token_end!=text_end)&&(i<max_token_length)&&!str::isspace(*token_end);++token_end,++i)
          {
            if(!es.process(*token_end))
              continue;
            text_iterator emoji_start=token_end;
            text_iterator emoji_end=emoji_start;
            ++emoji_end;
            for(;emoji_end!=text_end;++emoji_end)
              {
                if(es.get_result()!=0&&emoji_start!=text_start)
                  break;
                if(!es.process(*emoji_end))
                  break;
}
            if(es.get_result()==0)
              {
                es.reset();
                continue;
}
            if(emoji_start!=text_start)
              {
                token_end=emoji_start;
                break;
}
            token_end=emoji_start;
            std::advance(token_end,es.get_result());
            next_token.type=content_emoji;
            break;
}
      }
    next_token.text.assign(text_start,token_end);
    next_token.length=token_end.offset()-next_token.position;
    return token_end;
  }

  template<typename text_iterator>
  text_iterator sentence::add_text(const text_iterator& text_start,const text_iterator& text_end,const tts_markup& markup_info)
  {
    const language_list& languages=parent->get_engine().get_languages();
    const voice_list& voices=parent->get_engine().get_voices();
    language_voice_pair markup_language_and_voice=get_language_and_voice_from_markup(markup_info);
    if((language_and_voice.first!=languages.end())&&
       (markup_language_and_voice.first!=languages.end()))
      {
        if(markup_language_and_voice.first!=language_and_voice.first)
          return text_start;
        if((markup_language_and_voice.second!=voices.end())&&
           (language_and_voice.second!=voices.end())&&
           (markup_language_and_voice.second!=language_and_voice.second))
          return text_start;
      }
    text_iterator sentence_end=text_start;
    text_iterator token_end;
    voice_list::const_iterator token_voice=voices.end();
    std::size_t old_length=length;
    do
      {
        sentence_end=skip_whitespace(sentence_end,text_end,markup_info);
        if(sentence_end==text_end)
          break;
        token_end=get_next_token(sentence_end,text_end,markup_info);
        if(next_token_starts_new_sentence(markup_info))
          break;
        if(markup_language_and_voice.first==languages.end())
          {
            token_voice=determine_next_token_voice();
            if(token_voice!=voices.end())
              {
                if(language_and_voice.first==languages.end())
                  {
                    language_and_voice.first=token_voice->get_language();
                    language_and_voice.second=token_voice;
                  }
                else
                  {
                    if(language_and_voice.first!=token_voice->get_language())
                      break;
                  }
              }
          }
        else if(language_and_voice.first==languages.end())
          language_and_voice=markup_language_and_voice;
        if(next_token.type==content_text)
          commands.push_back(command_ptr(new append_token(prev_token,next_token)));
        else if(next_token.type==content_emoji)
          commands.push_back(command_ptr(new append_emoji(prev_token,next_token)));
        else if(next_token.type==content_phones)
          commands.push_back(command_ptr(new append_phones(prev_token,next_token)));
        else if(next_token.type==content_key)
          commands.push_back(command_ptr(new append_key(prev_token,next_token)));
        else
          commands.push_back(command_ptr(new append_chars(prev_token,next_token)));
        prev_token=next_token;
        length+=prev_token.text.size();
        ++num_tokens;
        sentence_end=token_end;
      }
    while((sentence_end!=text_end)&&(prev_token.text.size()<max_token_length)&&(length<max_sentence_length)&&(num_tokens<max_tokens));
    if(length>old_length)
      {
        if(old_length==0)
          {
            rate=markup_info.prosody.rate;
            pitch=markup_info.prosody.pitch;
            volume=markup_info.prosody.volume;
          }
      }
    return sentence_end;
  }

  template<typename input_iterator>
  std::unique_ptr<document> document::create_from_ssml(const std::shared_ptr<engine>& engine_ptr,const input_iterator& text_start,const input_iterator& text_end,const voice_profile& profile)
  {
    std::unique_ptr<document> doc_ptr(new document(engine_ptr,profile));
    #ifdef _MSC_VER
    typedef std::iterator_traits<input_iterator>::value_type char_type;
    #else
    typedef typename std::iterator_traits<input_iterator>::value_type char_type;
    #endif
    xml::parser<char_type> parser;
    xml::text_handler<char_type> text_handler;
    parser.set_text_handler(text_handler);
    ssml::speak_handler<char_type> speak_handler;
    parser.add_element_handler(speak_handler);
    ssml::s_handler<char_type> s_handler;
    parser.add_element_handler(s_handler);
    ssml::p_handler<char_type> p_handler;
    parser.add_element_handler(p_handler);
    ssml::voice_handler<char_type> voice_handler;
    parser.add_element_handler(voice_handler);
    ssml::mark_handler<char_type> mark_handler;
    parser.add_element_handler(mark_handler);
    ssml::say_as_handler<char_type> say_as_handler;
    parser.add_element_handler(say_as_handler);
    ssml::prosody_handler<char_type> prosody_handler;
    parser.add_element_handler(prosody_handler);
    ssml::audio_handler<char_type> audio_handler;
    parser.add_element_handler(audio_handler);
    ssml::break_handler<char_type> break_handler;
    parser.add_element_handler(break_handler);
    ssml::phoneme_handler<char_type> phoneme_handler;
    parser.add_element_handler(phoneme_handler);
    parser.parse(text_start,text_end,*doc_ptr);
    return doc_ptr;
  }

  template<typename text_iterator>
  void document::add_text(const text_iterator& text_start,const text_iterator& text_end,const tts_markup& markup_info)
  {
    text_iterator sentence_end=text_start;
    while(sentence_end!=text_end)
      {
        sentence_end=get_current_sentence().add_text(sentence_end,text_end,markup_info);
        if(sentence_end!=text_end)
          finish_sentence();
      }
  }
}
#endif
