/* Copyright (C) 2023  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_VIETNAMESE_HPP
#define RHVOICE_VIETNAMESE_HPP

#include "str.hpp"
#include "fst.hpp"
#include "data_only_language.hpp"

namespace RHVoice
{
  class vietnamese_info: public data_only_language_info
  {
  public:
    vietnamese_info(const std::string& data_path,const std::string& userdict_path);

  private:
    std::shared_ptr<language> create_instance() const;
  };

  class vietnamese: public data_only_language
  {
  public:
    explicit vietnamese(const vietnamese_info& info_);

    const vietnamese_info& get_info() const
    {
      return info;
    }

  private:
    bool is_word_in_tok_dict(const std::string& word) const;
    void decode_as_word(item& token,const std::string& name) const;

    const vietnamese_info& info;
    const fst tokdict_fst;
  };
}
#endif
