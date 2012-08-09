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

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include "smart_ptr.hpp"
#include "engine.hpp"
#include "utterance.hpp"
#include "utf.hpp"
#include "str.hpp"
#include "tts_markup.hpp"
#include "ssml.hpp"
#include "client.hpp"

#ifndef RHVOICE_DOCUMENT_HPP
#define RHVOICE_DOCUMENT_HPP
namespace RHVoice
{
  typedef std::pair<language_list::const_iterator,voice_list::const_iterator> language_voice_pair;
  class document;

  class sentence
  {
  private:
    enum command_type
      {
        command_token,
        command_mark
      };

    class abstract_command: public utterance_function
    {
    public:
      virtual command_type get_type() const=0;
    };

    typedef smart_ptr<abstract_command> command_ptr;

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
      template<typename text_iterator>
      append_token(text_iterator token_start,text_iterator token_end,const tts_markup& markup_info)
    {
      init(token_start,token_end,markup_info);
    }

      void execute(utterance& u) const;

    protected:
      append_token()
      {
      }

      template<typename text_iterator>
      void init(text_iterator token_start,text_iterator token_end,const tts_markup& markup_info)
      {
        position=token_start.offset();
        length=token_end.offset()-position;
        std::copy(token_start,token_end,str::utf8_inserter(std::back_inserter(name)));
      }

      std::string name;
      std::size_t position,length;
    };

    class append_chars: public append_token
    {
    public:
      template<typename text_iterator>
      append_chars(text_iterator token_start,text_iterator token_end,const tts_markup& markup_info):
        verbosity_level(verbosity_code|((markup_info.say_as==content_chars)?verbosity_name:verbosity_full_name))
    {
      init(token_start,token_end,markup_info);
    }

      void execute(utterance& u) const;

    private:
      verbosity_t verbosity_level;
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

    private:
      std::string name;
    };

    std::list<command_ptr> commands;
    std::vector<utf8::uint32_t> trailing_whitespace,last_token;
    const document* parent;
    double rate,pitch,volume;
    language_voice_pair language_and_voice;
    std::size_t length;

  public:
    explicit sentence(const document* parent_);

    bool empty() const
    {
      return commands.empty();
    }

    std::auto_ptr<utterance> create_utterance() const;

    template<typename text_iterator>
    text_iterator add_text(const text_iterator& text_start,const text_iterator& text_end,const tts_markup& markup_info);

    void add_mark(const std::string& name)
    {
      commands.push_back(command_ptr(new append_mark(name)));
    }

  private:
    bool token_starts_new_sentence(const std::vector<utf8::uint32_t>& token,const tts_markup& markup_info) const;
    language_list::const_iterator determine_token_language(const std::vector<uint32_t>& token) const;
    language_voice_pair get_language_and_voice_from_markup(const tts_markup& markup_info) const;
  };

  class document
  {
  public:
    struct init_params
    {
      voice_list::const_iterator main_voice,extra_voice;
    };

    speech_params speech_settings;

    explicit document(const smart_ptr<engine>& engine_ptr_,const init_params& params=init_params()):
      engine_ptr(engine_ptr_),
      owner(0),
      current_sentence(sentences.end()),
      main_voice(params.main_voice),
      extra_voice(params.extra_voice)
    {
      if(has_main_language()&&
         has_extra_language()&&
         (get_main_language()->has_common_letters(*get_extra_language())))
        throw std::invalid_argument("Invalid language pair");
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

    bool has_main_language() const
    {
      return has_main_voice();
    }

    language_list::const_iterator get_main_language() const
    {
      return main_voice->get_language();
    }

    bool has_main_voice() const
    {
      return (main_voice!=voice_list::const_iterator());
    }

    voice_list::const_iterator get_main_voice() const
    {
      return main_voice;
    }

    bool has_extra_language() const
    {
      return has_extra_voice();
    }

    language_list::const_iterator get_extra_language() const
    {
      return extra_voice->get_language();
    }

    bool has_extra_voice() const
    {
      return (extra_voice!=voice_list::const_iterator());
    }

    voice_list::const_iterator get_extra_voice() const
    {
      return extra_voice;
    }

    template<typename some_iterator>
    static std::auto_ptr<document> create_from_plain_text(const smart_ptr<engine>& engine_ptr,const some_iterator& text_start,const some_iterator& text_end,const init_params& params=init_params())
    {
      #ifdef _MSC_VER
      return create_from_plain_text(engine_ptr,text_start,text_end,params,std::iterator_traits<some_iterator>::iterator_category());
            #else
      return create_from_plain_text(engine_ptr,text_start,text_end,params,typename std::iterator_traits<some_iterator>::iterator_category());
      #endif
    }

    template<typename input_iterator>
    static std::auto_ptr<document> create_from_ssml(const smart_ptr<engine>& engine_ptr,const input_iterator& text_start,const input_iterator& text_end,const init_params& params=init_params());

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
    static std::auto_ptr<document> create_from_plain_text(const smart_ptr<engine>& engine_ptr,const input_iterator& text_start,const input_iterator& text_end,const init_params& params,std::input_iterator_tag)
    {
      #ifdef _MSC_VER
      typedef std::iterator_traits<input_iterator>::value_type char_type;
      #else
      typedef typename std::iterator_traits<input_iterator>::value_type char_type;
      #endif
      std::vector<char_type> tmp_buf(text_start,text_end);
      std::auto_ptr<document> doc_ptr(new document(engine_ptr,params));
      #ifdef _MSC_VER
      typedef utf::text_iterator<std::vector<char_type>::const_iterator> char_iterator;
      #else
      typedef utf::text_iterator<typename std::vector<char_type>::const_iterator> char_iterator;
      #endif
      doc_ptr->add_text(char_iterator(tmp_buf.begin(),tmp_buf.begin(),tmp_buf.end()),char_iterator(tmp_buf.end(),tmp_buf.begin(),tmp_buf.end()));
      return doc_ptr;
    }

    template<typename forward_iterator>
    static std::auto_ptr<document> create_from_plain_text(const smart_ptr<engine>& engine_ptr,const forward_iterator& text_start,const forward_iterator& text_end,const init_params& params,std::forward_iterator_tag)
    {
      std::auto_ptr<document> doc_ptr(new document(engine_ptr,params));
      typedef utf::text_iterator<forward_iterator> text_iterator;
      doc_ptr->add_text(text_iterator(text_start,text_start,text_end),text_iterator(text_end,text_start,text_end));
      return doc_ptr;
    }

    smart_ptr<engine> engine_ptr;
    client* owner;
    std::list<sentence> sentences;
    std::list<sentence>::iterator current_sentence;
    voice_list::const_iterator main_voice,extra_voice;
  };

  template<typename text_iterator>
  text_iterator sentence::add_text(const text_iterator& text_start,const text_iterator& text_end,const tts_markup& markup_info)
  {
    bool spell=((markup_info.say_as==content_char)||(markup_info.say_as==content_chars)||(markup_info.say_as==content_glyphs));
    language_voice_pair markup_language_and_voice=get_language_and_voice_from_markup(markup_info);
    if((language_and_voice.first!=language_list::const_iterator())&&
       (markup_language_and_voice.first!=language_list::const_iterator()))
      {
        if(markup_language_and_voice.first!=language_and_voice.first)
          return text_start;
        if((markup_language_and_voice.second!=voice_list::const_iterator())&&
           (language_and_voice.second!=voice_list::const_iterator())&&
           (markup_language_and_voice.second!=language_and_voice.second))
          return text_start;
      }
    std::vector<utf8::uint32_t> token;
    text_iterator token_start,token_end;
    text_iterator sentence_end=text_start;
    language_list::const_iterator token_language;
    std::size_t old_length=length;
    while(sentence_end!=text_end)
      {
        token_start=std::find_if(sentence_end,text_end,std::not1(str::is_space()));
        if(token_start!=sentence_end)
          {
            trailing_whitespace.insert(trailing_whitespace.end(),sentence_end,token_start);
            if(spell)
              {
                commands.push_back(command_ptr(new append_chars(sentence_end,token_start,markup_info)));
                length+=std::distance(sentence_end,token_start);
              }
            sentence_end=token_start;
            if(token_start==text_end)
              break;
          }
        token_end=std::find_if(token_start,text_end,str::is_space());
        token.assign(token_start,token_end);
        if(token_starts_new_sentence(token,markup_info))
          break;
        else
          {
            if(markup_language_and_voice.first==language_list::const_iterator())
              {
                token_language=determine_token_language(token);
                if(token_language!=language_list::const_iterator())
                  {
                    if(language_and_voice.first==language_list::const_iterator())
                      language_and_voice.first=token_language;
                    else
                      {
                        if(language_and_voice.first!=token_language)
                          break;
                      }
                  }
              }
            trailing_whitespace.clear();
            if(spell)
              commands.push_back(command_ptr(new append_chars(token_start,token_end,markup_info)));
            else
              commands.push_back(command_ptr(new append_token(token_start,token_end,markup_info)));
            length+=token.size();
            sentence_end=token_end;
            last_token=token;
          }
      }
    if(length>old_length)
      {
        if(old_length==0)
          {
            rate=markup_info.prosody.rate;
            pitch=markup_info.prosody.pitch;
            volume=markup_info.prosody.volume;
          }
        if(markup_language_and_voice.first!=language_list::const_iterator())
          {
            language_and_voice.first=markup_language_and_voice.first;
            if(markup_language_and_voice.second!=voice_list::const_iterator())
              language_and_voice.second=markup_language_and_voice.second;
          }
      }
    return sentence_end;
  }

  template<typename input_iterator>
  std::auto_ptr<document> document::create_from_ssml(const smart_ptr<engine>& engine_ptr,const input_iterator& text_start,const input_iterator& text_end,const init_params& params)
  {
    std::auto_ptr<document> doc_ptr(new document(engine_ptr,params));
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
