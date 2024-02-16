/* Copyright (C) 2013, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_USERDICT_HPP
#define RHVOICE_USERDICT_HPP

#include <stdexcept>
#include <memory>
#include <vector>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <map>

#include "str.hpp"
#include "item.hpp"
#include "relation.hpp"
#include "utterance.hpp"
#include "stress_pattern.hpp"
#include "trie.hpp"

namespace RHVoice
{
  class language_info;

  namespace userdict
  {
    typedef std::vector<utf8::uint32_t> chars32;

    enum
      {
        token_start=0x10ffff+1,
        token_end
      };

    class position: public std::iterator<std::forward_iterator_tag,utf8::uint32_t>
    {
    public:
      position():
        token(0),
        text(0),
        character(token_start)
      {
      }

      explicit position(item& token_)
      {
        set_token(token_);
      }

      explicit position(utterance& utt);

      utf8::uint32_t operator*() const
      {
        return character;
      }

      bool operator==(const position& other) const
      {
        if(token!=other.token)
          return false;
        if(token==0)
          return true;
        if(pos!=other.pos)
          return false;
        return (character==other.character);
      }

      bool operator!=(const position& other) const
      {
        return !(*this==other);
      }

      position& operator++()
      {
        if(token==0)
          return *this;
        if(character==token_end)
          forward_token();
        else if(pos==text->end())
          {
            character=token_end;
          }
        else
          character=utf8::next(pos,text->end());
        return *this;
      }

      position operator++(int)
      {
        position tmp=*this;
        ++(*this);
        return tmp;
      }

      item& get_token()
      {
        return *token;
      }

      item& get_token() const
      {
        return *token;
      }

      bool same_token(const position& other) const
      {
        return (token==other.token);
      }

      void forward_token();

    private:
      void clear()
      {
        token=0;
        text=0;
        pos=std::string::const_iterator();
        character=token_start;
      }

      void set_token(item& token_)
      {
        token=&token_;
        text=&(token->get("name").as<std::string>());
        pos=text->begin();
        character=token_start;
      }

      item* token;
      const std::string* text;
      std::string::const_iterator pos;
      utf8::uint32_t character;
    };

    class word_editor
    {
    public:
      explicit word_editor(utterance& u):
        utt(u),
        cursor(u),
        changed(false),
        initialism(false),
	foreign(false)
      {
      }

      utterance& get_utt()
      {
        return utt;
      }

      position get_cursor() const
      {
        return cursor;
      }

      void delete_char()
      {
        utf8::uint32_t c=*cursor;
        if(c>=token_start)
          throw std::logic_error("Nothing to delete");
        ++cursor;
        changed=true;
      }

      void insert_char(utf8::uint32_t c)
      {
        text.push_back(c);
        changed=true;
      }

      void forward_char()
      {
        utf8::uint32_t c=*cursor;
        switch(c)
          {
          case token_start:
            ++cursor;
            break;
          case token_end:
            save_word();
            new_word();
            ++cursor;
            break;
          default:
            text.push_back(c);
            ++cursor;
            break;
          }
      }

      void split_word()
      {
        changed=true;
        save_word();
        new_word();
        changed=true;
      }

      void mark_stress();

      void stress_syllable(int number)
      {
        stress.stress_syllable(number);
        changed=true;
      }

      void unstress_word()
      {
        stress.unstress();
        changed=true;
      }

      void decode_word_as_initialism()
      {
        initialism=true;
        changed=true;
      }

      void mark_word_as_foreign()
      {
        foreign=true;
        changed=true;
      }

      void mark_word_as_native()
      {
        foreign=false;
        changed=true;
      }


      void forward_token()
      {
        cursor.forward_token();
      }

    private:
      word_editor(const word_editor&);
      word_editor& operator=(const word_editor&);

      void save_word();
      void new_word();

      utterance& utt;
      position cursor;
      chars32 text;
      bool changed;
      stress_pattern stress;
      bool initialism;
      bool foreign;
    };

    class token
    {
    public:
      token():
        type(0)
      {
      }

      void append(utf8::uint32_t c)
      {
        text.push_back(c);
      }

      template<typename iterator>
      void append(iterator first,iterator last)
      {
        text.insert(text.end(),first,last);
      }

      int get_type() const
      {
        return type;
      }

      void set_type(int type_)
      {
        type=type_;
      }

      const chars32& get_text() const
      {
        return text;
      }

      std::string as_string() const
      {
        std::string result;
        std::copy(text.begin(),text.end(),str::utf8_inserter(std::back_inserter(result)));
        return result;
      }

      int as_number() const
      {
        int n=0;
        std::istringstream s(as_string());
        s >> n;
        return n;
      }

      void clear()
      {
	type=0;
	text.clear();
      }

    private:
      token(const token&);
      token& operator=(const token&);

      int type;
      chars32 text;
    };

    class correction
    {
    public:
      typedef std::shared_ptr<correction> pointer;

      virtual ~correction()
      {
      }

      virtual chars32 get_key() const
      {
        return chars32();
      }

      void virtual apply(word_editor& ed) const=0;
      std::string virtual describe() const=0;

    protected:
      correction()
      {
      }

    private:
      correction(const correction&);
      correction& operator=(const correction&);
    };

    class substring: public correction
    {
    public:
      explicit substring(const token* t):
        key(t->get_text())
      {
      }

      chars32 get_key() const
      {
        return key;
      }

      void apply(word_editor& ed) const
      {
        for(std::size_t i=0;i<key.size();++i)
          {
            ed.forward_char();
          }
      }

      std::string describe() const
      {
        std::string desc("(Substring ");
        utf8::utf32to8(key.begin(),key.end(),std::back_inserter(desc));
        desc+=')';
        return desc;
      }

    protected:
      chars32 key;
    };

    class symbol: public correction
    {
    public:
      explicit symbol(const token* t):
        chr(t->get_text()[0])
      {
      }

      chars32 get_key() const
      {
        return chars32(1,chr);
      }

      void apply(word_editor& ed) const
      {
        ed.delete_char();
      }

      std::string describe() const
      {
        std::string desc("(Symbol ");
        utf8::append(chr,std::back_inserter(desc));
        desc+=')';
        return desc;
      }

      private:
            utf8::uint32_t chr;
    };

    class deletion: public substring
    {
    public:
      explicit deletion(const token* t):
        substring(t)
      {
      }

      void apply(word_editor& ed) const
      {
        for(std::size_t i=0;i<key.size();++i)
          {
            ed.delete_char();
          }
      }

      std::string describe() const
      {
        std::string desc("(Delete ");
        utf8::utf32to8(key.begin(),key.end(),std::back_inserter(desc));
        desc+=')';
        return desc;
      }
    };

    class insertion: public correction
    {
    public:
      explicit insertion(const token* t):
        text(t->get_text())
      {
      }

      void apply(word_editor& ed) const
      {
        for(chars32::const_iterator it=text.begin();it!=text.end();++it)
          {
            ed.insert_char(*it);
          }
      }

      std::string describe() const
      {
        std::string desc("(Insert ");
        utf8::utf32to8(text.begin(),text.end(),std::back_inserter(desc));
        desc+=')';
        return desc;
      }

    private:
      chars32 text;
    };

    class empty_string: public correction
    {
    public:
      void apply(word_editor& ed) const
      {
      }

      std::string describe() const
      {
        return "EmptyString";
      }
    };

    class stress_mark: public correction
    {
    public:
      void apply(word_editor& ed) const
      {
        ed.mark_stress();
      }

      std::string describe() const
      {
        return "StressMark";
      }
    };

    class stressed_syl_number: public correction
    {
    public:
      explicit stressed_syl_number(const token* t):
        number(t->as_number())
      {
      }

      void apply(word_editor& ed) const
      {
        ed.stress_syllable(number);
      }

      std::string describe() const
      {
        std::ostringstream s;
        s << "(StressSyllable " << number << ")" << std::endl;
        return s.str();
      }

    private:
      int number;
    };

    class unstressed_flag: public correction
    {
    public:
      void apply(word_editor& ed) const
      {
        ed.unstress_word();
      }

      std::string describe() const
      {
        return "Unstressed";
      }
    };

    class initialism_flag: public correction
    {
    public:
      void apply(word_editor& ed) const
      {
        ed.decode_word_as_initialism();
      }

      std::string describe() const
      {
        return "Initialism";
      }
    };

    class foreign_flag: public correction
    {
    public:
      void apply(word_editor& ed) const
      {
        ed.mark_word_as_foreign();
      }

      std::string describe() const
      {
        return "Foreign";
      }
    };

    class native_flag: public correction
    {
    public:
      void apply(word_editor& ed) const
      {
        ed.mark_word_as_native();
      }

      std::string describe() const
      {
        return "Native";
      }
    };

    class start_of_token: public correction
    {
    public:
      chars32 get_key() const
      {
        return chars32(1,token_start);
      }

      void apply(word_editor& ed) const
      {
        ed.forward_char();
      }

      std::string describe() const
      {
        return "StartOfToken";
      }
    };

    class end_of_token: public correction
    {
    public:
      chars32 get_key() const
      {
        return chars32(1,token_end);
      }

      void apply(word_editor& ed) const
      {
        ed.forward_char();
      }

      std::string describe() const
      {
        return "EndOfToken";
      }
    };

    class word_break: public correction
    {
    public:
      void apply(word_editor& ed) const
      {
        ed.split_word();
      }

      std::string describe() const
      {
        return "WordBreak";
      }
    };

    class rule
    {
    private:
      typedef std::vector<correction::pointer> list_of_corrections;
      list_of_corrections corrections;

    public:
      bool empty() const
      {
        return corrections.empty();
      }

      chars32 get_key() const
      {
        chars32 result;
        for(list_of_corrections::const_iterator it=corrections.begin();it!=corrections.end();++it)
          {
            chars32 key=(*it)->get_key();
            result.insert(result.end(),key.begin(),key.end());
          }
        return result;
      }

      void apply(word_editor& ed) const
      {
        for(list_of_corrections::const_iterator it=corrections.begin();it!=corrections.end();++it)
          {
            (*it)->apply(ed);
          }
      }

      std::string describe() const
      {
        std::string desc;
        for(list_of_corrections::const_iterator it=corrections.begin();it!=corrections.end();++it)
          {
            desc+=(*it)->describe();
            desc+=' ';
          }
        return desc;
      }

      void append(const rule& other)
      {
        if(other.empty())
          return;
        corrections.reserve(corrections.size()+other.corrections.size());
        corrections.insert(corrections.end(),other.corrections.begin(),other.corrections.end());
      }

      template<class T>
      void append()
      {
        corrections.push_back(correction::pointer(new T));
      }

      template<class T,typename A>
      void append(const A& a)
      {
        corrections.push_back(correction::pointer(new T(a)));
      }
    };

    class ruleset
    {
    private:
      typedef std::vector<rule> list_of_rules;
      list_of_rules rules;

    public:
      ruleset() = default;

      typedef std::vector<rule>::const_iterator iterator;

      iterator begin() const
      {
        return rules.begin();
      }

      iterator end() const
      {
        return rules.end();
      }

      bool empty() const
      {
        return rules.empty();
      }

      std::string describe() const
      {
        std::string desc;
        for(iterator it=begin();it!=end();++it)
          {
            desc+=it->describe();
            desc+='\n';
          }
        return desc;
      }

      void append(const ruleset* other);

      template<class T>
      void append()
      {
        rule r;
        r.append<T>();
        append(r);
      }

      template<class T,typename A>
      void append(const A& a)
      {
        rule r;
        r.append<T,A>(a);
        append(r);
      }

      void extend(const ruleset* other)
      {
        if(other->empty())
          return;
        rules.reserve(rules.size()+other->rules.size());
        rules.insert(rules.end(),other->rules.begin(),other->rules.end());
      }

      template<class T>
      void extend()
      {
        rule r;
        r.append<T>();
        extend(r);
      }

      template<class T,typename A>
      void extend(const A& a)
      {
        rule r;
        r.append<T,A>(a);
        extend(r);
      }

      template<class T>
      static inline ruleset* create()
      {
        std::unique_ptr<ruleset> rs(new ruleset);
        rs->append<T>();
        return rs.release();
      }

      template<class T,typename A>
      static inline ruleset* create(const A& a)
      {
        std::unique_ptr<ruleset> rs(new ruleset);
        rs->append<T,A>(a);
        return rs.release();
      }

    private:
      ruleset(const ruleset&);
      ruleset& operator=(const ruleset&);

      void append(const rule& r);

      void extend(const rule& r)
      {
        rules.push_back(r);
      }
    };

    struct parse_state
    {
      std::unique_ptr<ruleset> result;
      bool error;

      parse_state():
        result(new ruleset),
        error(false)
      {
      }
    };

    class dict
  {
  public:
    explicit dict(const language_info& lng);
    void apply_rules(utterance& u) const;
    std::string simple_search(const std::string& w) const;

  private:
    dict(const dict&);
    dict& operator=(const dict&);

    struct to_lower
    {
      utf8::uint32_t operator()(utf8::uint32_t c) const
      {
        if(c>=token_start)
          return c;
        else
          return str::tolower(c);
      }
    };

    void load_all();
    void load_dir(const std::string& path);
    void load_file(const std::string& file_path);
    bool should_ignore_token(const position& pos) const;

    const language_info& lang;
    trie<utf8::uint32_t,rule,to_lower> rules;
    std::map<std::string, std::string> simple;
  };
  }
}
#endif
