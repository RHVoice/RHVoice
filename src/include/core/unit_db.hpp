/* Copyright (C) 2024  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_UNIT_DB_HPP
#define RHVOICE_UNIT_DB_HPP

#include <vector>
#include <map>
#include <string>

struct _HTS_Engine;

namespace RHVoice
{
  class unit_db
  {
  public:
    using data_vector_t=std::vector<double>;

struct state_t
{
  std::size_t dur_mean;
  float dur_var;
  data_vector_t mgc_mean, mgc_var;
};

struct unit_t
{
  std::string label;
  std::vector<state_t> states;
};

    unit_db(const std::string& data_path, _HTS_Engine* eng);

    bool has_unit(const std::string& lab) const
    {
      return (units.find(lab)!=units.end());
    }

    const unit_t& get_unit(const std::string& lab) const
    {
      return units.at(lab);
    }

  private:
    std::map<std::string, unit_t> units;
  };
}
#endif
