/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <shlobj.h>
#include "core/path.hpp"
#include "registry.hpp"
#include "utils.hpp"
#include "tts_base.hpp"

namespace RHVoice
{
  namespace sapi
  {
    threading::mutex tts_base::engine_mutex;
    smart_ptr<engine> tts_base::engine_ptr;

    smart_ptr<engine> tts_base::get_engine()
    {
      threading::lock l(engine_mutex);
      if(!engine_ptr.empty())
        return engine_ptr;
      engine::init_params p;
      registry::key app_key(HKEY_LOCAL_MACHINE,L"Software\\RHVoice",KEY_QUERY_VALUE);
      p.data_path=utils::wstring_to_string(app_key.get(L"data_path"));
      wchar_t appdata_path[MAX_PATH];
      if(SHGetFolderPath(0,CSIDL_APPDATA,0,SHGFP_TYPE_CURRENT,appdata_path)==S_OK)
        p.config_path=path::join(utils::wstring_to_string(appdata_path),"RHVoice");
      engine_ptr=engine::create(p);
      return engine_ptr;
    }
  }
}
