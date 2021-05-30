/* Copyright (C) 2012, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
    std::shared_ptr<engine> tts_base::engine_ptr;

    std::vector<std::string> tts_base::get_resource_paths(int id, std::string name) const
    {
      engine::init_params p;
      std::vector<std::string> paths, tmp_paths;
      wchar_t special_path[MAX_PATH];
      if(SHGetFolderPath(0,id,0,SHGFP_TYPE_CURRENT,special_path)==S_OK)
        {
          p.data_path=path::join(path::join(utils::wstring_to_string(special_path),name),"data");
          tmp_paths=p.get_language_paths();
          paths.insert(paths.end(),tmp_paths.begin(),tmp_paths.end());
          tmp_paths=p.get_voice_paths();
          paths.insert(paths.end(),tmp_paths.begin(),tmp_paths.end());
}
      return paths;
}

    std::vector<std::string> tts_base::get_resource_paths() const
    {
      std::vector<std::string> paths(get_resource_paths(CSIDL_PROGRAM_FILESX86,"RHVoice"));
      std::vector<std::string> new_paths(get_resource_paths(CSIDL_COMMON_APPDATA,"Olga Yakovleva\\RHVoice"));
      paths.insert(paths.end(),new_paths.begin(),new_paths.end());
      new_paths=get_resource_paths(CSIDL_COMMON_APPDATA,"rhvoice.org\\RHVoice");
      paths.insert(paths.end(),new_paths.begin(),new_paths.end());
      return paths;
}

    std::shared_ptr<engine> tts_base::get_engine()
    {
      threading::lock l(engine_mutex);
      if(engine_ptr)
        return engine_ptr;
      engine::init_params p;
      p.resource_paths=get_resource_paths();
      wchar_t appdata_path[MAX_PATH];
      if(SHGetFolderPath(0,CSIDL_APPDATA,0,SHGFP_TYPE_CURRENT,appdata_path)==S_OK)
        {
          std::wstring config_path=std::wstring(appdata_path)+std::wstring(L"\\RHVoice");
          p.config_path=utils::wstring_to_string(config_path);
          wchar_t common_appdata_path[MAX_PATH];
          if(SHGetFolderPath(0,CSIDL_COMMON_APPDATA,0,SHGFP_TYPE_CURRENT,common_appdata_path)==S_OK)
            {
              std::wstring in_file_path=std::wstring(common_appdata_path)+std::wstring(L"\\rhvoice.org\\RHVoice\\config\\RHVoice.ini");
              CreateDirectory(config_path.c_str(), nullptr);
              std::wstring out_file_path(config_path+L"\\RHVoice.ini");
              CopyFile(in_file_path.c_str(), out_file_path.c_str(), true);
            }
        }
      engine_ptr=engine::create(p);
      return engine_ptr;
    }
  }
}
