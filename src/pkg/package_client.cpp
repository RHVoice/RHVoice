/* Copyright (C) 2021, 2022  Olga Yakovleva <olga@rhvoice.org> */

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

#include "core/package_client.hpp"
#include <iterator>
#include <algorithm>
#include "boost/json/src.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/interprocess/sync/scoped_lock.hpp"
#include "boost/interprocess/sync/sharable_lock.hpp"
#include "boost/nowide/fstream.hpp"
#include <exception>
#include "core/path.hpp"
#include "curl.hpp"
#include "url_builder.hpp"

#ifndef PKG_DIR_URL
#define PKG_DIR_URL "https://rhvoice.org/download/packages-1.16.json"
#endif

namespace RHVoice
{
  namespace pkg
  {
    namespace ptime=boost::posix_time;
    namespace nw=boost::nowide;

namespace
{
  const std::string k_last_check_time("localLastCheckTime");
  const std::string k_ttl("ttl");
  const std::string k_l_ttl("localTtl");
  const ptime::time_duration def_ttl=ptime::hours(24);
  const ptime::time_duration second=ptime::seconds(1);

  void json_to_package(const js::value& jv, package& p)
  {
    p.name=js::value_to<decltype(p.name)>(jv.at("name"));
    if(auto i=jv.get_object().if_contains("id"))
      p.id=js::value_to<std::string>(*i);
    p.version=js::value_to<decltype(p.version)>(jv.at("version"));
    p.data_url=js::value_to<decltype(p.data_url)>(jv.at("dataUrl"));
  }
}

    decltype(package_client::instances_mutex) package_client::instances_mutex;
    decltype(package_client::instances) package_client::instances;

    void package_client::try_ensure_dir_file_lock()
    {
      if(dir_file_lock)
        return;
      try
        {
          #ifdef WIN32
          dir_file_lock.emplace(nw::widen(dir_file_path).c_str());
          #else
          dir_file_lock.emplace(dir_file_path.c_str());
          #endif
        }
      catch(const interp::interprocess_exception& e) {}
    }

    package_client::package_client(const std::string& p):
      state_dir_path(p),
      dir_file_path(path::join(state_dir_path, "packages.json"))
    {
      try_ensure_dir_file_lock();
      if(!dir_file_lock)
        return;
      std::string text;
      {
        interp::sharable_lock<interp::file_lock> lg(*dir_file_lock);
        nw::ifstream f(dir_file_path);
        if(!f)
          return;
        std::istreambuf_iterator<char> it{f}, end;
        text.assign(it, end);
      }
      js::error_code ec;
      dir=js::parse(text, ec);
      if(ec)
        return;
    }      

    package_client::ptr package_client::get(const std::string& p)
    {
      std::lock_guard<decltype(instances_mutex)> lg(instances_mutex);
      auto it=instances.find(p);
      if(it!=instances.end())
        {
          return it->second;
}
      ptr pc(new package_client(p));
      instances.emplace(p, pc);
      return pc;
    }

    js::value package_client::get_dir_from_server()
    {
      auto& cu=curl::get_instance();
      auto h=cu.easy_init();
      #ifdef ANDROID
      std::string ca_cert_path{path::join(state_dir_path, "cacert.pem")};
      curl_easy_setopt(h.get(), CURLOPT_CAINFO, ca_cert_path.c_str());
      #endif
      js::stream_parser parser;
      js::error_code ec;
      auto t=ptime::second_clock::universal_time();
      cu.do_http_get(
                     h,
                     PKG_DIR_URL,
                     [&parser, &ec] (const char* data, std::size_t size) {
                       parser.write(data, size, ec);
                       return !ec;});
      parser.finish();
      auto new_dir=parser.release();
      {
        package_directory pd(js::value_to<package_directory>(new_dir));
      }
      new_dir.as_object().emplace(k_last_check_time, ptime::to_iso_string(t));
      save_dir(new_dir);
      {
        std::lock_guard<decltype(dir_mutex)> lg(dir_mutex);
        dir=new_dir;
      }
      new_dir.as_object().emplace(k_l_ttl, get_local_ttl(new_dir));
      return new_dir;
    }

    void package_client::save_dir(const js::value& new_dir)
    {
      boost::optional<interp::scoped_lock<interp::file_lock>> lg;
      try_ensure_dir_file_lock();
      if(dir_file_lock)
        lg.emplace(*dir_file_lock);
      nw::ofstream f(dir_file_path);
      if(!f)
        return;
      if(!lg)
        {
          try_ensure_dir_file_lock();
          if(!dir_file_lock)
            return;
          lg.emplace(*dir_file_lock);
        }
      f << new_dir << std::endl;
      f.flush();
    }

    js::value package_client::get_cached_dir()
    {
      js::value res_dir;
      {
        std::lock_guard<decltype(dir_mutex)> lock(dir_mutex);
        res_dir=dir;
      }
      if(auto* o=res_dir.if_object())
        o->emplace(k_l_ttl, get_local_ttl(res_dir));
      return res_dir;
    }

    js::value package_client::get_dir()
    {
      js::value res_dir=get_cached_dir();
      if(const auto* o=res_dir.if_object())
        if(o->at(k_l_ttl).as_int64()>0)
          return res_dir;
      try
        {
          return get_dir_from_server();
        }
      catch(...) {}
      if(auto* o=res_dir.if_object())
        (*o)[k_l_ttl]=get_local_ttl(res_dir);
      return res_dir;
    }

    boost::int64_t package_client::get_local_ttl(const js::value& d) const
    {
            if(const auto* o=d.if_object())
        if(const auto* v=o->if_contains(k_last_check_time))
          if(const auto* s=v->if_string())
            {
              try
                {
                  const auto t0=ptime::from_iso_string(std::string(s->cbegin(), s->cend()));
                  ptime::time_duration i=def_ttl;
                  if(const auto* p=o->if_contains(k_ttl))
                    i=second*(p->to_number<int>());
                  const auto t1=t0+i;
                  const auto t=ptime::second_clock::universal_time();
                  if(t<t1)
                    {
                      const auto ttl=t1-t;
                      return ttl.total_seconds();
                    }
                  }
              catch(...) {}
            }
            return 0;
    }

    std::string package_client::get_cached_dir_as_string()
    {
      const auto d=get_cached_dir();
      if(d.is_null())
        return "";
      return js::serialize(d);
    }

    std::string package_client::get_dir_from_server_as_string()
    {
      const auto d=get_dir_from_server();
      if(d.is_null())
        return "";
      return js::serialize(d);
    }

    std::string package_client::get_dir_as_string()
    {
      const auto d=get_dir();
      if(d.is_null())
        return "";
      return js::serialize(d);
    }

    package_version tag_invoke(js::value_to_tag<package_version>, const js::value& jv)
    {
      package_version ver;
      ver.major=js::value_to<decltype(ver.major)>(jv.at("major"));
      ver.minor=js::value_to<decltype(ver.minor)>(jv.at("minor"));
      return ver;
    }

    voice_package tag_invoke(js::value_to_tag<voice_package>, const js::value& jv)
    {
      voice_package p;
      json_to_package(jv, p);
      p.ctry2code=js::value_to<decltype(p.ctry2code)>(jv.at("ctry2code"));
      p.ctry3code=js::value_to<decltype(p.ctry3code)>(jv.at("ctry3code"));
      if(auto a=jv.get_object().if_contains("accent"))
        p.accent=js::value_to<std::string>(*a);
      p.demo_url=js::value_to<decltype(p.demo_url)>(jv.at("demoUrl"));
      return p;
    }

    language_package tag_invoke(js::value_to_tag<language_package>, const js::value& jv)
    {
      language_package p;
      json_to_package(jv, p);
      p.lang2code=js::value_to<decltype(p.lang2code)>(jv.at("lang2code"));
      p.lang3code=js::value_to<decltype(p.lang3code)>(jv.at("lang3code"));
      p.test_message=js::value_to<decltype(p.test_message)>(jv.at("testMessage"));
      p.voices=js::value_to<decltype(p.voices)>(jv.at("voices"));
      return p;
    }

    package_directory tag_invoke(js::value_to_tag<package_directory>, const js::value& jv)
    {
      package_directory pd;
      pd.languages=js::value_to<decltype(pd.languages)>(jv.at("languages"));
      pd.default_voices=js::value_to<decltype(pd.default_voices)>(jv.at("defaultVoices"));
      if(auto t=jv.get_object().if_contains(k_ttl))
        pd.ttl=js::value_to<unsigned int>(*t);
      return pd;
    }
  }
}
