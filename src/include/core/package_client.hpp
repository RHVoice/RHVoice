/* Copyright (C) 2021  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_PACKAGE_CLIENT_HPP
#define RHVOICE_PACKAGE_CLIENT_HPP
#define BOOST_DATE_TIME_NO_LIB

#include <string>
#include <memory>
#include <mutex>
#include <map>
#include "boost/cstdint.hpp"
#include <vector>
#include "boost/json.hpp"
#include "boost/interprocess/sync/file_lock.hpp"
#include "boost/optional.hpp"

namespace RHVoice
{
  namespace pkg
  {
    namespace js=boost::json;
    namespace interp=boost::interprocess;

struct package_version final
{
  unsigned int major {1};
  unsigned int minor {0};
};

    package_version tag_invoke(js::value_to_tag<package_version>, const js::value& jv);

struct package
{
  std::string name;
  boost::optional<std::string> id;
  package_version version;
  std::string data_url;
};

    struct voice_package: public package
    {
      std::string ctry2code, ctry3code;
      boost::optional<std::string> accent;
      std::string demo_url;
    };

    voice_package tag_invoke(js::value_to_tag<voice_package>, const js::value& jv);

    struct language_package: public package
    {
      std::string lang2code, lang3code;
      std::string test_message;
      std::vector<voice_package> voices;
    };

    language_package tag_invoke(js::value_to_tag<language_package>, const js::value& jv);

struct package_directory final
{
  std::vector<language_package> languages;
  std::map<std::string, std::map<std::string, std::string>> default_voices;
  boost::optional<unsigned int> ttl;
};

    package_directory tag_invoke(js::value_to_tag<package_directory>, const js::value& jv);

    class package_client final
    {
    public:
      using ptr=std::shared_ptr<package_client>;
      package_client(const package_client&)=delete;
      package_client& operator=(const package_client&)=delete;

      static ptr get(const std::string& p);

      js::value get_cached_dir();
      js::value get_dir_from_server();
      js::value get_dir();

      std::string get_cached_dir_as_string();
      std::string get_dir_from_server_as_string();
      std::string get_dir_as_string();

    private:
      package_client(const std::string& p);
      void try_ensure_dir_file_lock();
      void save_dir(const js::value& new_dir);
      boost::int64_t get_local_ttl(const js::value& d) const;

      const std::string state_dir_path;
      const std::string dir_file_path;
      std::mutex dir_file_mutex;
      boost::optional<interp::file_lock> dir_file_lock;
      std::mutex dir_mutex;
      js::value dir;
      static std::mutex instances_mutex;
      static std::map<std::string, ptr> instances;
    };
  }
}
#endif
