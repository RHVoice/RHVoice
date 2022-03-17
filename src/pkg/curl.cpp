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

#include <stdexcept>
#include <string>
#include <array>
#include "curl.hpp"
#include <iostream>

namespace
{
struct curl_ok_t
{
  void check(CURLcode code)
  {
    if(code==CURLE_OK)
      return;
    if(buf.at(0)!='\0')
      throw std::runtime_error(buf.data());
    throw std::runtime_error(curl_easy_strerror(code));
  }

  curl_ok_t(RHVoice::curl::easy_handle& h):
    handle(h.get())
  {
    curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, buf.data());
  }

  curl_ok_t& operator=(CURLcode code) {check(code); return *this;}

  ~curl_ok_t()
  {
    curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, NULL);
  }

private:
  CURL* handle;
  std::array<char, CURL_ERROR_SIZE> buf{'\0', 0};
};

  constexpr auto curl_opt_yes{1l};
  constexpr auto curl_opt_no{0l};
  const std::string user_agent{"RHVoice"};
  constexpr auto connect_timeout{10l};
  constexpr auto low_speed_limit{1024l};
  constexpr auto low_speed_time{60l};

  extern "C" size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
  {
    const auto count=size*nmemb;
    if(count==0)
      return 0;
    auto& callback=*reinterpret_cast<RHVoice::curl::write_callback_t*>(userdata);
    bool res=callback(ptr, count);
    if(!res)
      return 0;
    return count;
  }

  extern "C" int progress_callback(void *clientp,   double dltotal,   double dlnow,   double ultotal,   double ulnow)
  {
    auto& callback=*reinterpret_cast<RHVoice::curl::progress_callback_t*>(clientp);
    auto cont=callback(dltotal, dlnow);
    return cont?0:-1;
  }
}

namespace RHVoice
{
  curl::curl()
  {
    curl_global_init(CURL_GLOBAL_ALL);
  }

  curl::~curl()
  {
    curl_global_cleanup();
  }

  curl& curl::get_instance()
  {
    static curl instance;
    return instance;
  }

  curl::easy_handle curl::easy_init()
  {
    CURL* p=curl_easy_init();
    if(!p)
      throw std::runtime_error("Cannot create curl easy handle");
    easy_handle h(p, curl_easy_cleanup);
    curl_ok_t ok(h);
    ok=curl_easy_setopt(h.get(), CURLOPT_FAILONERROR, curl_opt_yes);
    ok=curl_easy_setopt(h.get(), CURLOPT_FOLLOWLOCATION, curl_opt_yes);
    ok=curl_easy_setopt(h.get(), CURLOPT_USERAGENT, user_agent.c_str());
    ok=curl_easy_setopt(h.get(), CURLOPT_CONNECTTIMEOUT, connect_timeout);
    ok=curl_easy_setopt(h.get(), CURLOPT_LOW_SPEED_LIMIT, low_speed_limit);
    ok=curl_easy_setopt(h.get(), CURLOPT_LOW_SPEED_TIME, low_speed_time);
    return h;
  }

  void curl::do_http_get(easy_handle& h, const std::string& url, write_callback_t w_callback, progress_callback_t p_callback)
  {
    curl_ok_t ok(h);
    ok=curl_easy_setopt(h.get(), CURLOPT_URL, url.c_str());
    ok=curl_easy_setopt(h.get(), CURLOPT_WRITEFUNCTION, &write_callback);
    ok=curl_easy_setopt(h.get(), CURLOPT_WRITEDATA, &w_callback);
    if(p_callback)
      {
        ok=curl_easy_setopt(h.get(), CURLOPT_NOPROGRESS, curl_opt_no);
        ok=curl_easy_setopt(h.get(), CURLOPT_PROGRESSFUNCTION, &progress_callback);
        ok=curl_easy_setopt(h.get(), CURLOPT_PROGRESSDATA, &p_callback);
      }
    else
      ok=curl_easy_setopt(h.get(), CURLOPT_NOPROGRESS, curl_opt_yes);
    ok=curl_easy_perform(h.get());
  }
}
