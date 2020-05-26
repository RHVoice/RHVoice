/* Copyright (C) 2013, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <cstdlib>
#include <fstream>
#include "core/path.hpp"
#include "core/io.hpp"
#include "core/language.hpp"
#include "userdict_parser.h"
#include "userdict_parser.c"

namespace RHVoice
{
  namespace userdict
  {
    position::position(utterance& utt):
      token(0),
      text(0),
      character(token_start)
    {
      relation& rel=utt.get_relation("TokStructure",true);
      for(relation::iterator it=rel.begin();it!=rel.end();++it)
        {
          if(it->has_children())
            {
              set_token(it->first_child());
              break;
            }
        }
    }

    void position::forward_token()
    {
      if(token==0)
        return;
      if(token->has_next())
        {
          set_token(token->next());
          return;
        }
      item::iterator parent=token->parent().get_iterator();
      while(parent->has_next())
        {
          ++parent;
          if(parent->has_children())
            {
              set_token(parent->first_child());
              return;
            }
        }
      clear();
    }

    void word_editor::mark_stress()
    {
      const language_info& lang=utt.get_language().get_info();
      if(!lang.supports_stress_marks())
        return;
      stress.stress_syllable(std::count_if(text.begin(),text.end(),is_vowel_letter(lang))+1);
      if(stress.get_state()==stress_pattern::stressed)
        changed=true;
        }

    void word_editor::save_word()
    {
      if(!changed)
        return;
      if(text.empty())
        return;
      std::string name;
      std::copy(text.begin(),text.end(),str::utf8_inserter(std::back_inserter(name)));
      item& token=cursor.get_token();
      const language& lang=utt.get_language();
      if(initialism)
        lang.decode_as_letter_sequence(token,name);
      else
        {
          lang.decode_as_word(token,name);
          if(stress.get_state()!=stress_pattern::undefined)
            token.last_child().set("stress_pattern",stress);
        }
    }

    void word_editor::new_word()
    {
      text.clear();
      stress.reset();
      initialism=false;
      changed=false;
    }

    void ruleset::append(const ruleset* other)
    {
      if(other->empty())
        return;
      if(empty())
        rules=other->rules;
      else
        {
          list_of_rules new_rules;
          new_rules.reserve(rules.size()*other->rules.size());
          for(list_of_rules::const_iterator it1=rules.begin();it1!=rules.end();++it1)
            {
              for(list_of_rules::const_iterator it2=other->rules.begin();it2!=other->rules.end();++it2)
                {
                  rule r=*it1;
                  r.append(*it2);
                  new_rules.push_back(r);
                }
            }
          rules.swap(new_rules);
        }
    }

    void ruleset::append(const rule& r)
    {
      if(r.empty())
        return;
      if(empty())
        rules.push_back(r);
      else
        {
          list_of_rules new_rules=rules;
          for(list_of_rules::iterator it=new_rules.begin();it!=new_rules.end();++it)
            {
              it->append(r);
            }
          rules.swap(new_rules);
        }
    }

    namespace
    {
      class lexer
      {
      public:
        lexer(const language_info& lng,const chars32& input_):
          lang(lng),
          input(input_),
          pos(input.begin())
        {
        }

        std::unique_ptr<token> get_next_token();

      private:
        lexer(const lexer&);
        lexer& operator=(const lexer&);

        bool match_reserved_symbol(utf8::uint32_t chr,int type)
        {
          if(*pos==chr)
            {
              next_token->set_type(type);
              ++pos;
              return true;
            }
          else
            return false;
        }

        bool match_flag()
        {
          if(*pos!='/')
            return false;
          if((pos+1)==input.end())
            return false;
          return (match_stressed_syl_number_flag()||
                  match_unstressed_flag()||
                  match_initialism_flag());
        }

        bool match_stressed_syl_number_flag()
        {
          chars32::const_iterator new_pos=pos+1;
          if(*new_pos=='-')
            {
              ++new_pos;
              if(new_pos==input.end())
                return false;
            }
          if((*new_pos<'1')||(*new_pos>'9'))
            return false;
          ++new_pos;
          next_token->set_type(UDTK_STRESSED_SYL_NUMBER);
          next_token->append(pos+1,new_pos);
          pos=new_pos;
          return true;
        }

        bool match_unstressed_flag()
        {
          if(*(pos+1)=='u')
            {
              next_token->set_type(UDTK_UNSTRESSED);
              pos+=2;
              return true;
            }
          else
            return false;
        }

        bool match_initialism_flag()
        {
          if(*(pos+1)=='i')
            {
              next_token->set_type(UDTK_INITIALISM);
              pos+=2;
              return true;
            }
          else
            return false;
        }

        bool match_space()
        {
          chars32::const_iterator old_pos=pos;
          for(;((pos!=input.end())&&((*pos==' ')||(*pos=='\t')));++pos);
          if(pos==old_pos)
            return false;
          else
            {
              if((old_pos==input.begin())||(*(old_pos-1)=='\n')||(*(old_pos-1)=='\r')||(*pos=='\n')||(*pos=='\r')||(*pos==';'))
                next_token->set_type(0);
              else
                next_token->set_type(UDTK_SPACE);
              return true;
            }
        }

      bool match_newline()
        {
          chars32::const_iterator old_pos=pos;
          if(*pos=='\r')
            ++pos;
          if((pos!=input.end())&&(*pos=='\n'))
            ++pos;
          if(pos==old_pos)
            return false;
          else
            {
              next_token->set_type(UDTK_NEWLINE);
              return true;
            }
        }

        bool match_native_letters()
        {
          chars32::const_iterator old_pos=pos;
          for(;((pos!=input.end())&&(lang.is_letter(*pos)||lang.is_sign(*pos)));++pos)
            {
              next_token->append(*pos);
            }
          if(pos==old_pos)
            return false;
          else
            {
              next_token->set_type(UDTK_NATIVE_LETTERS);
              return true;
            }
        }

        bool match_english_letters()
        {
          chars32::const_iterator old_pos=pos;
          for(;((pos!=input.end())&&(((*pos>='a')&&(*pos<='z'))||(*pos>='A')&&(*pos<='Z')));++pos)
            {
              next_token->append(*pos);
            }
          if(pos==old_pos)
            return false;
          else
            {
              next_token->set_type(UDTK_ENGLISH_LETTERS);
              return true;
            }
        }

        bool match_comment()
        {
          if(*pos!=';')
            return false;
          for(++pos;((pos!=input.end())&&(*pos!='\n')&&(*pos!='\r'));++pos);
          next_token->set_type(0);
          return true;
        }

        bool match_symbol()
        {
          chars32::const_iterator end=pos+1;
          utf8::uint32_t sym=*pos;
          if(sym=='#')
            {
              chars32::const_iterator start=pos+1;
              if(start!=input.end())
                {
                  sym=*start;
                  end=std::find_if(start,input.end(),std::not1(str::is_adigit()));
                  if(start==end)
                    ++end;
                  else
                    {
                      std::string s;
                      utf8::utf32to8(start,end,std::back_inserter(s));
                      std::istringstream strm(s);
                      utf8::uint32_t c;
                      if(strm>>c)
                        if(utf::is_valid(c))
                          sym=c;
                    }
                }
            }
          next_token->append(sym);
          next_token->set_type(UDTK_SYM);
          pos=end;
          return true;
        }

        const language_info& lang;
        const chars32& input;
        std::unique_ptr<token> next_token;
        chars32::const_iterator pos;
      };

      class compiler
      {
      public:
        compiler(const language_info& lng,const std::string& file_path);
        ~compiler();
        std::unique_ptr<ruleset> compile();

      private:
        compiler(const compiler&);
        compiler& operator=(const compiler&);

        void read_source(const std::string& file_path);

        const language_info& lang;
        chars32 source;
        void* parser;
      };

      std::unique_ptr<token> lexer::get_next_token()
      {
        if(pos==input.end())
          return std::unique_ptr<token>();
        next_token.reset(new token);
        match_native_letters()||
          match_english_letters()||
        match_reserved_symbol('(',UDTK_LPAREN)||
          match_reserved_symbol(')',UDTK_RPAREN)||
          match_reserved_symbol('=',UDTK_EQUALS)||
          match_reserved_symbol('+',UDTK_PLUS)||
          match_reserved_symbol('*',UDTK_STAR)||
          match_reserved_symbol('{',UDTK_LBRACE)||
          match_reserved_symbol('}',UDTK_RBRACE)||
          match_reserved_symbol(',',UDTK_COMMA)||
          match_reserved_symbol('&',UDTK_AND)||
          match_flag()||
          match_space()||
          match_newline()||
          match_comment()||
          match_symbol();
        return std::move(next_token);
      }
    }

    compiler::compiler(const language_info& lng,const std::string& file_path):
      lang(lng),
      parser(0)
    {
      read_source(file_path);
      if(source.empty())
        throw std::invalid_argument("The file is empty");
      if((source.back()!='\n')&&(source.back()!='\r'))
        source.push_back('\n');
      parser=userdictParseAlloc(std::malloc);
    }

    compiler::~compiler()
    {
      userdictParseFree(parser,std::free);
    }

    std::unique_ptr<ruleset> compiler::compile()
    {
      lexer lxr(lang,source);
      parse_state ps;
      std::unique_ptr<token> tok;
      while((tok=lxr.get_next_token()).get()!=0)
        {
          int type=tok->get_type();
          if(type==0)
            continue;
          userdictParse(parser,type,tok.release(),&ps);
          if(type==UDTK_NEWLINE)
            {
              userdictParse(parser,0,0,&ps);
              ps.error=false;
            }
        }
      return std::move(ps.result);
    }

    void compiler::read_source(const std::string& file_path)
    {
      std::ifstream strm;
      io::open_ifstream(strm,file_path);
      std::istreambuf_iterator<char> start_of_file(strm);
      std::istreambuf_iterator<char> end_of_file;
      std::string text(start_of_file,end_of_file);
      std::string::const_iterator start_of_text=utf8::starts_with_bom(text.begin(),text.end())?(text.begin()+3):text.begin();
      utf8::utf8to32(text.begin(),text.end(),std::back_inserter(source));
    }

    dict::dict(const language_info& lng):
      lang(lng)
    {
      load_all();
      rules.sort();
    }

    void userdict::dict::load_all()
    {
      std::vector<std::string> paths(lang.get_userdict_paths());
      for(std::vector<std::string>::const_iterator it=paths.begin();it!=paths.end();++it)
        {
          load_dir(*it);
}
}

    void userdict::dict::load_dir(const std::string& dir_path)
    {
      if(!path::isdir(dir_path))
        return;
      std::vector<std::string> file_paths;
      for(path::directory dir(dir_path);!dir.done();dir.next())
        {
          std::string file_path=path::join(dir_path,dir.get());
          if(path::isfile(file_path))
            file_paths.push_back(file_path);
        }
      std::sort(file_paths.begin(),file_paths.end());
      for(std::vector<std::string>::const_iterator it=file_paths.begin();it!=file_paths.end();++it)
        {
          load_file(*it);
        }
    }

    void dict::load_file(const std::string& file_path)
    {
      try
        {
          compiler comp(lang,file_path);
          std::unique_ptr<ruleset> result(comp.compile());
          for(ruleset::iterator it=result->begin();it!=result->end();++it)
            {
              chars32 key=it->get_key();
              rules.insert(key.begin(),key.end(),*it);
            }
        }
      catch(const std::exception& e)
        {
        }
    }

    void dict::apply_rules(utterance& utt) const
    {
      word_editor ed(utt);
      position mark,end;
      while(ed.get_cursor()!=end)
        {
          if(should_ignore_token(ed.get_cursor()))
            {
              ed.forward_token();
              continue;
            }
          mark=ed.get_cursor();
          mark.forward_token();
          for(;(mark!=end)&&(!should_ignore_token(mark));mark.forward_token());
          while(ed.get_cursor()!=mark)
            {
              std::pair<position,rule*> result=rules.search(ed.get_cursor(),mark);
              if(result.second==0)
                ed.forward_char();
              else
                result.second->apply(ed);
            }
        }
    }

    bool dict::should_ignore_token(const position& pos) const
    {
      const item& tok=pos.get_token();
      if(tok.get("verbosity").as<verbosity_t>()==verbosity_silent)
        return true;
      const std::string& type=tok.get("pos").as<std::string>();
      if((type=="word")||(type=="sym")||(type=="char"))
        return false;
      return true;
    }
  }
}
