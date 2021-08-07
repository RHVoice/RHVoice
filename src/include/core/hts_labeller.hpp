/* Copyright (C) 2012, 2016  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_HTS_LABELLER_HPP
#define RHVOICE_HTS_LABELLER_HPP

#include <string>
#include <vector>

#include "exception.hpp"

namespace RHVoice
{
  class item;
  class feature_function;

  class hts_feature_undefined: public lookup_error
  {
  public:
    hts_feature_undefined(const std::string& name):
      lookup_error("This hts feature has not been defined: "+name)
    {
    }
  };

  class hts_labeller
  {
  public:
    explicit hts_labeller(const std::string& file_path)
    {
      load_label_format_description(file_path);
      define_default_features();
    }

    void define_feature(const std::shared_ptr<feature_function>& f);
    void define_extra_phonetic_feature(const std::string& name);
    void define_ph_flag_feature(const std::string& name);
    std::string eval_segment_label(const item& seg) const;

  private:
    void load_label_format_description(const std::string& file_path);
    void define_default_features();

    struct hts_feature
    {
      std::string name,prefix;
      std::shared_ptr<feature_function> function;
    };

    std::vector<hts_feature> features;
  };
}
#endif
