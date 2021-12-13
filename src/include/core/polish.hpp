/* Copyright (C) 2021 Grzegorz Zlotowicz  <grzezlo@gmail.com> */

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

#ifndef RHVOICE_POLISH_HPP
#define RHVOICE_POLISH_HPP

#include "str.hpp"
#include "property.hpp"
#include "fst.hpp"
#include "language.hpp"

namespace RHVoice
{
  class polish_info: public language_info
  {
  public:
    polish_info(const std::string& data_path,const std::string& userdict_path);

    #ifdef WIN32
    unsigned short get_id() const
    {
      return 1045;
    }
    #endif
    
    std::string get_country() const
    {
      return "POL/PL";
    }

    bool supports_pseudo_english() const
    {
      return true;
    }
    
  private:
    #ifdef WIN32
    void do_register_settings(config& cfg,const std::string& prefix);
    bool_property present_as_english;
    #endif
    std::shared_ptr<language> create_instance() const;
  };

  class polish: public language
  {
  public:
    explicit polish(const polish_info& info);

    const polish_info& get_info() const
    {
      return info;
    }

    std::vector<std::string> get_word_transcription(const item& word) const;
  private:
    void post_lex(utterance& u) const;

    const polish_info& info;
    const fst g2p_fst;
    const fst lseq_fst;
  };
}
#endif
