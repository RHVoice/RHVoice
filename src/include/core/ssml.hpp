/* Copyright (C) 2012, 2021  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_SSML_HPP
#define RHVOICE_SSML_HPP

#include <stack>
#include <sstream>
#include <locale>
#include "xml.hpp"
#include "str.hpp"

namespace RHVoice
{
  namespace ssml
  {
    template<typename ch>
    class language_tracking_element_handler: public xml::element_handler<ch>
    {
    protected:
      language_tracking_element_handler(const char* name):
      xml::element_handler<ch>(name)
      {
      }

    private:
      virtual bool do_enter(xml::handler_args<ch>& args)
      {
        return true;
      }

      virtual void do_leave(xml::handler_args<ch>& args)
      {
      }

      std::stack<language_search_criteria> language_stack;

    public:
      bool enter(xml::handler_args<ch>& args)
      {
        language_stack.push(args.tts_markup_info.language_criteria);
        std::string language_tag=xml::get_attribute_value(args.node,"xml:lang");
        std::string language_code=language_tag.substr(0,language_tag.find('-'));
        if(!language_code.empty())
          {
            language_search_criteria c;
            c.set_code(language_code);
            args.tts_markup_info.language_criteria=c;
          }
        return do_enter(args);
      }

      void leave(xml::handler_args<ch>& args)
      {
        do_leave(args);
        args.tts_markup_info.language_criteria=language_stack.top();
        language_stack.pop();
      }
    };

    template<typename ch>
    class speak_handler: public language_tracking_element_handler<ch>
    {
    public:
      speak_handler():
        language_tracking_element_handler<ch>("speak")
      {
      }

    private:
      bool do_enter(xml::handler_args<ch>& args)
      {
        return ((args.node->parent())&&(args.node->parent()->type()==rapidxml::node_document));
      }
    };

    template<typename ch>
    class s_handler: public language_tracking_element_handler<ch>
    {
    public:
      s_handler():
        language_tracking_element_handler<ch>("s")
      {
      }

    private:
      bool do_enter(xml::handler_args<ch>& args)
      {
        args.target_document.finish_sentence();
        args.tts_markup_info.autosplit_sentences=false;
        return true;
      }

      void do_leave(xml::handler_args<ch>& args)
      {
        args.target_document.finish_sentence();
        args.tts_markup_info.autosplit_sentences=true;
      }
    };

    template<typename ch>
    class p_handler: public language_tracking_element_handler<ch>
    {
    public:
      p_handler():
        language_tracking_element_handler<ch>("p")
      {
      }

    private:
      bool do_enter(xml::handler_args<ch>& args)
      {
        args.target_document.finish_sentence();
        return true;
      }

      void do_leave(xml::handler_args<ch>& args)
      {
        args.target_document.finish_sentence();
      }
    };

    template<typename ch>
    class voice_handler: public language_tracking_element_handler<ch>
    {
    public:
      voice_handler():
        language_tracking_element_handler<ch>("voice")
      {
      }

    private:
      bool do_enter(xml::handler_args<ch>& args)
      {
        voice_stack.push(args.tts_markup_info.voice_criteria);
        voice_search_criteria c;
        str::tokenizer<str::is_space> names(xml::get_attribute_value(args.node,"name"));
        if(names.begin()!=names.end())
          c.add_names(names.begin(),names.end());
        if(!c.empty())
          args.tts_markup_info.voice_criteria=c;
        return true;
      }

      void do_leave(xml::handler_args<ch>& args)
      {
        args.tts_markup_info.voice_criteria=voice_stack.top();
        voice_stack.pop();
      }

      std::stack<voice_search_criteria> voice_stack;
    };

    template<typename ch>
    class mark_handler: public xml::element_handler<ch>
    {
    public:
      mark_handler():
        xml::element_handler<ch>("mark")
      {
      }

      bool enter(xml::handler_args<ch>& args)
      {
        std::string mark_name=xml::get_attribute_value(args.node,"name");
        if(!mark_name.empty())
          args.target_document.add_mark(mark_name);
        return false;
      }
    };

    template<typename ch>
    class say_as_handler: public xml::element_handler<ch>
    {
    public:
      say_as_handler():
        xml::element_handler<ch>("say-as")
      {
      }

      bool enter(xml::handler_args<ch>& args)
      {
        std::string interpret_as=xml::get_attribute_value(args.node,"interpret-as");
        if(interpret_as=="characters")
          {
            args.tts_markup_info.say_as=content_chars;
            std::string format=xml::get_attribute_value(args.node,"format");
            if(format=="glyphs")
              args.tts_markup_info.say_as=content_glyphs;
          }
        else if(interpret_as=="tts:char")
          args.tts_markup_info.say_as=content_char;
        else if(interpret_as=="tts:key")
          args.tts_markup_info.say_as=content_key;
        return true;
      }

      void leave(xml::handler_args<ch>& args)
      {
        args.tts_markup_info.say_as=content_text;
      }
    };

    template<typename ch>
    class prosody_handler: public xml::element_handler<ch>
    {
    public:
      prosody_handler():
        xml::element_handler<ch>("prosody")
      {
      }

      bool enter(xml::handler_args<ch>& args)
      {
        prosody_stack.push(args.tts_markup_info.prosody);
        update_prosody_info(args.node,"rate",args.tts_markup_info.prosody.rate);
        update_prosody_info(args.node,"pitch",args.tts_markup_info.prosody.pitch);
        update_prosody_info(args.node,"volume",args.tts_markup_info.prosody.volume);
        return true;
      }

      void leave(xml::handler_args<ch>& args)
      {
        args.tts_markup_info.prosody=prosody_stack.top();
        prosody_stack.pop();
      }

    private:
      struct value_type
      {
        int sign;
        double number;
        std::string unit;

        value_type():
          sign(0),
          number(100),
          unit("%")
        {
        }
      };

      value_type parse_value(const std::string& strval) const;
      void update_prosody_info(const rapidxml::xml_node<ch>* node,const std::string& attribute_name,double& attribute_ref);

      std::stack<prosodic_attributes> prosody_stack;
    };

    template<typename ch>
    typename prosody_handler<ch>::value_type prosody_handler<ch>::parse_value(const std::string& strval) const
    {
      value_type result;
      if(strval.empty())
        return result;
      std::istringstream instream(strval);
      instream.imbue(std::locale::classic());
      if(!str::isadigit(strval[0]))
        {
          if((strval.size()<2)||!str::isadigit(strval[1]))
            return result;
          char c='\0';
          instream >> c;
          switch(c)
            {
            case '+':
              result.sign=1;
              break;
            case '-':
              result.sign=-1;
              break;
            default:
              return result;
            }
        }
      if(instream >> result.number)
        instream >> result.unit;
      else
        result.sign=0;
      return result;
    }

    template<typename ch>
    void prosody_handler<ch>::update_prosody_info(const rapidxml::xml_node<ch>* node,const std::string& attribute_name,double& attribute_ref)
    {
      std::string strval(xml::get_attribute_value(node,attribute_name.c_str()));
      if(strval.empty())
        return;
      if(strval=="default")
        attribute_ref=1;
      else
        {
          value_type val(parse_value(strval));
          if(val.number==0)
            return;
          if(val.unit=="%")
            {
              if((val.sign==-1)&&(val.number>=100))
                val.number=99;
              attribute_ref*=(((val.sign==0)?val.number:(100.0+val.sign*val.number))/100.0);
            }
        }
    }

    template<typename ch>
    class audio_handler: public xml::element_handler<ch>
    {
    public:
      audio_handler():
        xml::element_handler<ch>("audio")
      {
      }

      bool enter(xml::handler_args<ch>& args)
      {
        std::string src=xml::get_attribute_value(args.node,"src");
        if(!src.empty())
          args.target_document.add_audio(src);
        return false;
      }
    };

    template<typename ch>
    class break_handler: public xml::element_handler<ch>
    {
    public:
      break_handler():
        xml::element_handler<ch>("break")
      {
      }

      bool enter(xml::handler_args<ch>& args)
      {
        break_strength strength=break_phrase;
        std::string str_strength=xml::get_attribute_value(args.node,"strength");
        if(!str_strength.empty())
          {
            if(str_strength=="none")
              strength=break_none;
            else if(str_strength=="x-weak")
              strength=break_default;
            else if(str_strength=="weak")
              strength=break_default;
            else if(str_strength=="medium")
              strength=break_phrase;
            else if(str_strength=="strong")
              strength=break_sentence;
            else if(str_strength=="x-strong")
              strength=break_sentence;
          }
          args.target_document.add_break(strength);
        return false;
      }
    };

    template<typename ch>
    class phoneme_handler: public xml::element_handler<ch>
    {
    public:
      phoneme_handler():
        xml::element_handler<ch>("phoneme")
      {
      }

      bool enter(xml::handler_args<ch>& args)
      {
        const std::string a=xml::get_attribute_value(args.node,"alphabet");
        if(a.empty() || a=="x-RHVoice")
          {
            xml::text_iterator<const ch*> start, end;
            auto r=xml::get_attribute_value_range(args.node,"ph", start, end);
            if(r)
              {
                auto m=args.tts_markup_info;
                m.say_as=content_phones;
                args.target_document.add_text(start, end, m);
              }
          }
        return false;
      }
    };
  }
}
#endif
