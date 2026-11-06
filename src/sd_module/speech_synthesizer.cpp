/* Copyright (C) 2012, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <exception>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "core/document.hpp"
#include "io.hpp"
#include "speech_synthesizer.hpp"

namespace RHVoice
{
  namespace sd
  {
    namespace
    {
      const char* const xml_space=" \t\r\n";

      bool is_name_start(char c)
      {
        return (((c>='a')&&(c<='z'))||((c>='A')&&(c<='Z'))||(c=='_')||(c==':'));
      }

      // True if the message is a well-formed document with speak as its
      // only top-level element. The parser accepts several of them, and
      // the engine would then read the first and silently drop the rest.
      bool is_ssml_document(const std::string& text)
      {
        std::vector<char> xml_text(text.begin(),text.end());
        std::replace(xml_text.begin(),xml_text.end(),'\0',' ');
        xml_text.push_back('\0');
        rapidxml::xml_document<char> xml_doc;
        try
          {
            xml_doc.parse<rapidxml::parse_non_destructive>(&xml_text[0]);
          }
        catch(const rapidxml::parse_error&)
          {
            return false;
          }
        const rapidxml::xml_node<char>* root=xml_doc.first_node();
        return (root&&(root->type()==rapidxml::node_element)&&!root->next_sibling()&&
                (std::string(root->name(),root->name_size())=="speak"));
      }

      // True if the ampersand at pos starts a reference the parser accepts.
      bool is_reference(const std::string& s,std::size_t pos,std::size_t end)
      {
        const std::size_t semicolon=s.find(';',pos+1);
        if((semicolon==std::string::npos)||(semicolon>=end)||(semicolon==pos+1))
          return false;
        const std::string name(s,pos+1,semicolon-pos-1);
        if(name[0]!='#')
          return ((name=="amp")||(name=="lt")||(name=="gt")||(name=="quot")||(name=="apos"));
        const bool hex=((name.size()>1)&&(name[1]=='x'));
        const std::size_t first=hex?2:1;
        if(first==name.size())
          return false;
        return (name.find_first_not_of(hex?"0123456789abcdefABCDEF":"0123456789",first)==std::string::npos);
      }

      void append_text(std::string& out,const std::string& s,std::size_t start,std::size_t end)
      {
        for(std::size_t pos=start;pos<end;++pos)
          {
            const char c=s[pos];
            if(c=='<')
              out.append("&lt;");
            else if(c=='"')
              out.append("&quot;");
            else if((c=='&')&&!is_reference(s,pos,end))
              out.append("&amp;");
            else
              out.push_back(c);
          }
      }

      // Escapes the ampersands that do not start a reference. The engine
      // reads such an ampersand as the character before it, and one that
      // begins a character reference without its semicolon makes it take
      // the text up to the next semicolon as the reference: the words
      // between are lost, and if a tag comes first, the message is never
      // finished.
      std::string escape_ampersands(const std::string& s)
      {
        std::string out;
        out.reserve(s.size());
        std::size_t lt=s.find('<');
        for(std::size_t pos=0;pos<s.size();++pos)
          {
            if(pos>lt)
              lt=s.find('<',pos);
            if((s[pos]=='&')&&!is_reference(s,pos,std::min(lt,s.size())))
              out.append("&amp;");
            else
              out.push_back(s[pos]);
          }
        return out;
      }

      // Finds the name of the index mark in a tag given without its angle
      // brackets, e.g. mark name="1"/
      bool get_mark_name(const std::string& s,std::size_t start,std::size_t end,std::size_t& name_start,std::size_t& name_end)
      {
        std::size_t pos=s.find_first_of(" \t\r\n/",start);
        if((pos==std::string::npos)||(pos>=end)||(s.compare(start,pos-start,"mark")!=0))
          return false;
        while(true)
          {
            pos=s.find_first_not_of(xml_space,pos);
            if((pos==std::string::npos)||(pos>=end)||!is_name_start(s[pos]))
              return false;
            const std::size_t attr_start=pos;
            pos=s.find_first_of(" \t\r\n=",pos);
            if((pos==std::string::npos)||(pos>=end))
              return false;
            const std::size_t attr_end=pos;
            pos=s.find_first_not_of(xml_space,pos);
            if((pos==std::string::npos)||(pos>=end)||(s[pos]!='='))
              return false;
            pos=s.find_first_not_of(xml_space,pos+1);
            if((pos==std::string::npos)||(pos>=end)||((s[pos]!='"')&&(s[pos]!='\'')))
              return false;
            const std::size_t value_start=pos+1;
            pos=s.find(s[pos],value_start);
            if((pos==std::string::npos)||(pos>=end))
              return false;
            if(s.compare(attr_start,attr_end-attr_start,"name")==0)
              {
                name_start=value_start;
                name_end=pos;
                return true;
              }
            ++pos;
          }
      }

      // Makes a well-formed document out of markup the XML parser rejects.
      // The text and the index marks stay, every other tag goes, so the
      // message is still spoken and its marks are still reported.
      std::string recover_ssml(const std::string& s)
      {
        std::string out("<speak>");
        std::size_t pos=0;
        while(pos<s.size())
          {
            const std::size_t lt=s.find('<',pos);
            if(lt==std::string::npos)
              {
                append_text(out,s,pos,s.size());
                break;
              }
            append_text(out,s,pos,lt);
            std::size_t end=std::string::npos;
            if(s.compare(lt,4,"<!--")==0)
              {
                end=s.find("-->",lt+4);
                if(end!=std::string::npos)
                  end+=3;
              }
            else if(s.compare(lt,9,"<![CDATA[")==0)
              {
                end=s.find("]]>",lt+9);
                if(end!=std::string::npos)
                  {
                    append_text(out,s,lt+9,end);
                    end+=3;
                  }
              }
            else if(s.compare(lt,2,"<?")==0)
              {
                end=s.find("?>",lt+2);
                if(end!=std::string::npos)
                  end+=2;
              }
            else if((lt+1<s.size())&&((s[lt+1]=='!')||(s[lt+1]=='/')||is_name_start(s[lt+1])))
              {
                const std::size_t gt=s.find_first_of("<>",lt+1);
                if((gt!=std::string::npos)&&(s[gt]=='>'))
                  {
                    std::size_t name_start,name_end;
                    if(get_mark_name(s,lt+1,gt,name_start,name_end))
                      {
                        out.append("<mark name=\"");
                        append_text(out,s,name_start,name_end);
                        out.append("\"/>");
                      }
                    else
                      out.push_back(' ');
                    end=gt+1;
                  }
              }
            if(end==std::string::npos)
              {
                // Not markup: a less-than sign in the text.
                out.append("&lt;");
                end=lt+1;
              }
            pos=end;
          }
        out.append("</speak>");
        return out;
      }

      // Speech Dispatcher sends every text message as SSML: its own escaped
      // text inside a speak element, or whatever a client in SSML mode
      // wrote. A client's markup can lack the speak element, which the
      // engine needs as the root, or not be XML at all.
      std::unique_ptr<document> create_text_document(const std::shared_ptr<engine>& engine_ptr,const std::string& message,const voice_profile& profile)
      {
        const std::string text(escape_ampersands(message));
        if(is_ssml_document(text))
          return document::create_from_ssml(engine_ptr,text.begin(),text.end(),profile);
        // The engine reads a speak element only as the root, so a fragment
        // that has one somewhere inside cannot be wrapped into another.
        if(text.find("<speak")==std::string::npos)
          {
            const std::string fragment("<speak>"+text+"</speak>");
            try
              {
                return document::create_from_ssml(engine_ptr,fragment.begin(),fragment.end(),profile);
              }
            catch(const rapidxml::parse_error& e)
              {
                logger::log(2,"The message is not well-formed SSML: ",e.what());
              }
          }
        logger::log(2,"Keeping only the text and the index marks of the message");
        const std::string recovered(recover_ssml(text));
        return document::create_from_ssml(engine_ptr,recovered.begin(),recovered.end(),profile);
      }
    }

    void speech_synthesizer::signal_stop()
    {
      threading::lock message_lock(message_mutex);
      status=status_stopped;
      message_available.signal();
    }

    void speech_synthesizer::stop()
    {
      if(!has_started())
        return;
      state::Set(state::stopped);
      signal_stop();
      player.stop();
      player.join();
      logger::log(2,"Stopped the playback thread");
    }

    void speech_synthesizer::send_message(const tts_message& msg)
    {
      threading::lock message_lock(message_mutex);
      if(status!=status_empty)
        return;
      current_message=msg;
      status=status_message;
      message_available.signal();
    }

    bool speech_synthesizer::get_message(tts_message& msg)
    {
      threading::lock message_lock(message_mutex);
      if(status==status_stopped)
        return false;
      while(status!=status_message)
        {
          message_available.wait(message_mutex);
          if(status==status_stopped)
            return false;
        }
      msg=current_message;
      status=status_empty;
      return true;
    }

    void speech_synthesizer::prepare()
    {
      player.start();
      logger::log(2,"Started the playback thread");
    }

    void speech_synthesizer::run()
    {
      try
        {
          tts_message msg;
          while(get_message(msg))
            {
              player.enqueue(synth_result_ptr(new start_of_speech));
              synthesize_message(msg);
              player.enqueue(synth_result_ptr(new end_of_speech));
        }
        }
      catch(const std::exception& e)
        {
          logger::log(2,"Fatal error in the synthesis thread: ",e.what());
        }
    }

    void speech_synthesizer::synthesize_message(const tts_message& msg)
    {
      try
        {
          std::unique_ptr<document> doc;
          switch(msg.type)
            {
            case tts_message_text:
              doc=create_text_document(tts_engine,msg.content,msg.profile);
              break;
            case tts_message_char:
              doc=document::create_from_plain_text(tts_engine,msg.content.begin(),msg.content.end(),content_char,msg.profile);
              break;
            case tts_message_key:
              doc=document::create_from_plain_text(tts_engine,msg.content.begin(),msg.content.end(),content_key,msg.profile);
              break;
            default:
              doc=document::create_from_plain_text(tts_engine,msg.content.begin(),msg.content.end(),content_text,msg.profile);
              break;
            }
          if(cancelled())
            return;
          doc->speech_settings.absolute.rate=msg.rate;
          doc->speech_settings.absolute.pitch=msg.pitch;
          doc->speech_settings.absolute.volume=msg.volume;
          doc->verbosity_settings.punctuation_mode=msg.punctuation_mode;
          doc->verbosity_settings.capitals_mode=msg.capitals_mode;
          doc->set_owner(*this);
          doc->synthesize();
        }
      // The thread has to survive anything one message does: run() must
      // still report the end of this message and take the next one. An XML
      // parse error is not an RHVoice::exception, and used to end the thread
      // with the message begun and never finished.
      catch(const std::exception& e)
        {
          logger::log(2,"Synthesis error: ",e.what());
        }
    }

    bool speech_synthesizer::process_mark(const std::string& name)
    {
      player.enqueue(synth_result_ptr(new index_mark(name)));
      return (!cancelled_or_pausing());
    }

    bool speech_synthesizer::play_speech(const short* samples,std::size_t count)
    {
      if(cancelled())
        return false;
      player.enqueue(synth_result_ptr(new speech_chunk(samples,count)));
      return true;
    }

    bool speech_synthesizer::set_sample_rate(int sample_rate)
    {
      if(cancelled())
        return true;
      player.enqueue(synth_result_ptr(new sample_rate_setting(sample_rate)));
      return true;
    }
  }
}
