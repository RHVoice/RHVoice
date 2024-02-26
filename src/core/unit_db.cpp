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


#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include "HTS_engine.h"
#include "HTS_hidden.h"
#include "core/unit_db.hpp"
#include "core/io.hpp"
#include "core/path.hpp"

namespace RHVoice
{
  unit_db::unit_db(const std::string& data_path, _HTS_Engine* eng)
  {
    std::ifstream lf, df;
    io::open_ifstream(lf, path::join(data_path, "units.lab"));
    io::open_ifstream(df, path::join(data_path, "units.data"), true);
    std::uint32_t ver;
    if(!df.read(reinterpret_cast<char*>(&ver), sizeof(ver)))
      throw file_format_error("Can't read the version number");
    if(ver!=0)
      throw file_format_error("Unsupported format version");
    const auto nstate=HTS_Engine_get_nstate(eng);
    const auto vlen=HTS_ModelSet_get_vector_length(&eng->ms, 0)*3;
    std::string lab;
    float val;
    while(lf >> lab)
      {
	unit_t unit;
	unit.label=lab;
	for (auto s=0; s<nstate; ++s)
	  {
	    state_t state;
	    state.mgc_mean.reserve(vlen);
	    state.mgc_var.reserve(vlen);
	    if(!df.read(reinterpret_cast<char*>(&val), sizeof(float)))
	      throw file_format_error("Error reading duration mean");
	    state.dur_mean=static_cast<std::size_t>(std::round(val));
	    for(auto i=0; i<vlen;++i)
	      {
		if(!df.read(reinterpret_cast<char*>(&val), sizeof(float)))
		  throw file_format_error("Error reading mgc mean");
		state.mgc_mean.push_back(val);
	      }
	    if(!df.read(reinterpret_cast<char*>(&state.dur_var), sizeof(float)))
	      throw file_format_error("Error reading duration variance");
	    for(auto i=0; i<vlen;++i)
	      {
		if(!df.read(reinterpret_cast<char*>(&val), sizeof(float)))
		  throw file_format_error("Error reading mgc variance");
		if(val==0)
		  val=0.0000001;
		state.mgc_var.push_back(val);
	      }
	    unit.states.push_back(state);
	  }
	units[lab]=unit;
      }
  }
}
