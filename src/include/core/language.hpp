/* Copyright (C) 2012, 2013, 2014, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_LANGUAGE_HPP
#define RHVOICE_LANGUAGE_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include "params.hpp"
#include "exception.hpp"
#include "smart_ptr.hpp"
#include "path.hpp"
#include "resource.hpp"
#include "value.hpp"
#include "utterance.hpp"
#include "relation.hpp"
#include "phoneme_set.hpp"
#include "hts_labeller.hpp"
#include "fst.hpp"
#include "dtree.hpp"
#include "userdict.hpp"
#include "str.hpp"
#include "pitch.hpp"

namespace RHVoice
{
  class utterance;
  class item;
  class language_info;
  class language_list;

  enum verbosity_flag
    {
      verbosity_silent=0,
      verbosity_name=1,
      verbosity_full_name=2,
      verbosity_spell=4,
      verbosity_pitch=8,
      verbosity_sound=16
    };
  typedef unsigned int verbosity_t;

  enum break_strength
    {
      break_default,
      break_none,
      break_phrase,
      break_sentence
    };

  class feature_function_not_found: public lookup_error
  {
  public:
    feature_function_not_found():
      lookup_error("Feature function not found")
    {
    }
  };

  class feature_function
  {
  public:
    virtual ~feature_function()
    {
    }

    const std::string& get_name() const
    {
      return name;
    }

    value virtual eval(const item& i) const=0;

  protected:
    explicit feature_function(const std::string& name_):
      name(name_)
    {
    }

  private:
    feature_function(const feature_function&);
    feature_function& operator=(const feature_function&);

    const std::string name;
  };

  class language_error: public exception
  {
  public:
    explicit language_error(const std::string& msg):
      exception(msg)
    {
    }
  };

  class tokenization_error: public language_error
  {
  public:
    tokenization_error():
      language_error("Tokenization failed")
    {
    }
  };

  class g2p_error: public language_error
  {
  public:
    g2p_error(const item& word):
      language_error("G2p failed: "+word.get("name").as<std::string>())
    {
    }
  };

    class syllabification_error: public language_error
    {
    public:
      syllabification_error():
        language_error("Syllabification failed")
      {
      }
    };

    class fst_error: public language_error
    {
    public:
      fst_error():
        language_error("Unexpected fst output")
      {
      }
    };

  class language
  {
  public:
    virtual ~language()
    {
    }

    virtual const language_info& get_info() const=0;

    const feature_function& get_feature_function(const std::string& name) const
    {
      std::map<std::string,smart_ptr<feature_function> >::const_iterator it(feature_functions.find(name));
      if(it==feature_functions.end())
        throw feature_function_not_found();
      else
        return *(it->second);
    }

    const feature_function* get_feature_function_ptr(const std::string& name) const
    {
      std::map<std::string,smart_ptr<feature_function> >::const_iterator it(feature_functions.find(name));
      if(it==feature_functions.end())
        return 0;
      else
        return (it->second.get());
    }

    const hts_labeller& get_hts_labeller() const
    {
      return labeller;
    }

    const phoneme_set& get_phoneme_set() const
    {
      return phonemes;
    }

    item& append_token(utterance& u,const std::string& text) const;

    bool supports_emoji() const
    {
      return (emoji_fst.get()!=0);
}

    item& append_emoji(utterance& u,const std::string& text) const;
    void do_text_analysis(utterance& u) const;
    void do_pos_tagging(utterance& u) const;
    void phrasify(utterance& u) const;
    void detect_utt_type(utterance& u) const;
    void do_g2p(utterance& u) const;
    void syllabify(utterance& u) const;
    void insert_pauses(utterance& u) const;
    void do_post_lexical_processing(utterance& u) const;
    void set_pitch_modifications(utterance& u) const;
    void set_duration_modifications(utterance& u) const;
    void stress_monosyllabic_words(utterance& u) const;
    void rename_palatalized_consonants(utterance& u) const;

    virtual void decode_as_word(item& token,const std::string& name) const;
    virtual void decode_as_letter_sequence(item& token,const std::string& name) const;

    virtual std::vector<std::string> get_word_transcription(const item& word) const=0;

    template<typename forward_iterator,typename output_iterator>
    void downcase(forward_iterator first,forward_iterator last,output_iterator output) const
    {
      downcase_fst.translate(first,last,output);
    }

  protected:
    explicit language(const language_info& info_);

    void register_feature(const smart_ptr<feature_function>& f)
    {
      feature_functions[f->get_name()]=f;
    }

    hts_labeller& get_hts_labeller()
    {
      return labeller;
    }

    virtual bool decode_as_known_character(item& token,const std::string& name) const;

    virtual void assign_pronunciation(item& word) const;

  private:
    language(const language&);
    language& operator=(const language&);

    void register_default_features();

    virtual void post_lex(utterance& u) const
    {
    }

    void decode(item& token) const;
    void default_decode_as_word(item& token,const std::string& name) const;
    void decode_as_number(item& token,const std::string& name) const;
    void decode_as_digit_string(item& token,const std::string& name) const;
    void decode_as_unknown_character(item& token,const std::string& name) const;
    void decode_as_character(item& token,const std::string& name) const;
    void decode_as_key(item& token,const std::string& name) const;

    virtual void decode_as_special_symbol(item& token,const std::string& name,const std::string& type) const
    {
    }

    void indicate_case_if_necessary(item& token) const;
    break_strength get_word_break(const item& word) const;
    bool should_break_emoji(const item& word) const;
    bool decode_as_english(item& tok) const;
    std::vector<std::string> get_english_word_transcription(const item& word) const;

    virtual void before_g2p(item& word) const
{
}
    bool check_for_f123(const item& tok,const std::string& name) const;

    bool translate_emoji(item& token,std::vector<utf8::uint32_t>::const_iterator start,std::vector<utf8::uint32_t>::const_iterator end) const;
    std::vector<utf8::uint32_t> remove_emoji_presentation_selectors(const std::string& text) const;
    void translate_emoji_element(item& token,std::vector<utf8::uint32_t>::const_iterator start,std::vector<utf8::uint32_t>::const_iterator end) const;
    void translate_emoji_sequence(item& token,const std::string& text) const;


    std::map<std::string,smart_ptr<feature_function> > feature_functions;
    const phoneme_set phonemes;
    hts_labeller labeller;
    const fst tok_fst;
    const fst key_fst;
    const fst numbers_fst;
    const fst gpos_fst;
    const dtree phrasing_dtree;
    const fst syl_fst;
    std::vector<std::string> msg_cap_letter,msg_char_code;
    std::map<utf8::uint32_t,std::string> whitespace_symbols;
    std::unique_ptr<fst> english_phone_mapping_fst;
    std::unique_ptr<fst> emoji_fst;
std::unique_ptr<fst> qst_fst;
    std::unique_ptr<dtree> pitch_mod_dtree;
    std::unique_ptr<dtree> dur_mod_dtree;
    pitch::targets_spec_parser pts_parser;
    userdict::dict udict;

  protected:
    const fst spell_fst;
    const fst downcase_fst;
  };

  class language_info: public resource_info<language>
  {
    friend class language_list;
  protected:
    language_info(const std::string& name,const std::string& data_path,const std::string& userdict_path);

    void set_alpha2_code(const std::string& code)
    {
      alpha2_code=code;
    }

    void set_alpha3_code(const std::string& code)
    {
      alpha3_code=code;
    }

  public:
    voice_params voice_settings;
    text_params text_settings;
    bool_property use_pseudo_english;

    void register_settings(config& cfg);

    bool is_enabled() const
    {
      return enabled;
    }

    bool is_letter(utf8::uint32_t cp) const
    {
      return (letters.find(cp)!=letters.end());
    }

    bool is_sign(utf8::uint32_t cp) const
    {
      return (signs.find(cp)!=signs.end());
    }

    bool has_common_letters(const language_info& other) const
    {
      for(std::set<utf8::uint32_t>::const_iterator iter=letters.begin();iter!=letters.end();++iter)
        {
          if(other.is_letter(*iter))
            return true;
        }
      return false;
    }

    template<typename input_iterator>
    std::size_t count_letters_in_text(input_iterator first,input_iterator last) const
    {
      std::size_t result=0;
      for(input_iterator iter=first;iter!=last;++iter)
        {
          if(is_letter(*iter))
            ++result;
        }
      return result;
    }

    template<typename input_iterator>
    bool are_all_letters(input_iterator first,input_iterator last) const
    {
      if(first==last)
        return false;
      for(input_iterator iter=first;iter!=last;++iter)
        {
          if(!is_letter(*iter))
            return false;
        }
      return true;
    }

    bool is_vowel_letter(utf8::uint32_t cp) const
    {
      return (vowel_letters.find(cp)!=vowel_letters.end());
    }

    virtual bool has_unicase_alphabet() const
    {
      return false;
    }

    const std::string& get_alpha2_code() const
    {
      return alpha2_code;
    }

    const std::string& get_alpha3_code() const
    {
      return alpha3_code;
    }

    #ifdef WIN32
    virtual unsigned short get_id() const=0;
    #endif

    virtual std::string get_country() const
    {
      return std::string();
    }

    virtual bool supports_stress_marks() const
    {
      return false;
    }

    virtual bool supports_pseudo_english() const
    {
      return false;
    }

  protected:
    virtual void do_register_settings(config& cfg,const std::string& prefix);

  private:
    std::string alpha2_code,alpha3_code;
    std::set<utf8::uint32_t> letters,vowel_letters,signs;
    bool_property enabled;

  protected:
    void register_letter(utf8::uint32_t cp)
    {
      letters.insert(cp);
    }

    void register_letter_range(utf8::uint32_t cp,std::size_t n)
    {
      for(std::size_t i=0;i<n;++i)
        {
          letters.insert(cp+i);
        }
    }

    void register_vowel_letter(utf8::uint32_t cp)
    {
      vowel_letters.insert(cp);
    }

    void register_sign(utf8::uint32_t cp)
    {
      signs.insert(cp);
    }

  public:
    const language_list& get_all_languages() const
    {
      return *all_languages;
    }

    std::vector<std::string> get_userdict_paths() const;

  private:
    const language_list* all_languages;
    std::string userdict_path;
  };

  class language_list: public resource_list<language_info>
  {
  public:
    language_list(const std::vector<std::string>& language_paths,const std::string& userdict_path,const event_logger& logger);

  private:
    class creator
    {
    public:
      creator()
      {
      }

      virtual ~creator()
      {
      }

      virtual smart_ptr<language_info> create(const std::string& data_path,const std::string& userdict_path) const=0;

    private:
      creator(const creator&);
      creator& operator=(const creator&);
    };

    template<class T>
    class concrete_creator: public creator
    {
    public:
      smart_ptr<language_info> create(const std::string& data_path,const std::string& userdict_path) const
      {
        return smart_ptr<language_info>(new T(data_path,userdict_path));
      }
    };

    typedef std::pair<std::string,unsigned int> language_id;
    typedef std::map<language_id,smart_ptr<creator> > Creators;

    template<class T>
    void register_language(const std::string& name,unsigned int format)
    {
      creators[language_id(name,format)]=smart_ptr<creator>(new concrete_creator<T>);
    }

    Creators creators;
  };

  class language_search_criteria: public std::unary_function<const language_info&,bool>
  {
  public:
    void set_name(const std::string& name_)
    {
      name=name_;
    }

    void clear_name()
    {
      name.clear();
    }

    void set_code(const std::string& code_)
    {
      code=code_;
    }

    void clear_code()
    {
      code.clear();
    }

    bool empty() const
    {
      return (name.empty()&&code.empty());
    }

    bool operator()(const language_info& info) const;

  private:
    std::string name,code;
  };

  struct is_letter: std::unary_function<utf8::uint32_t,bool>
  {
    explicit is_letter(const language_info& lang_):
      lang(&lang_)
    {
    }

    bool operator()(utf8::uint32_t cp) const
    {
      return lang->is_letter(cp);
    }

  private:
    const language_info* lang;
  };

  struct is_vowel_letter: std::unary_function<utf8::uint32_t,bool>
  {
    explicit is_vowel_letter(const language_info& lang_):
      lang(&lang_)
    {
    }

    bool operator()(utf8::uint32_t cp) const
    {
      return lang->is_vowel_letter(cp);
    }

  private:
    const language_info* lang;
  };
}
#endif
