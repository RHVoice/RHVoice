/* Copyright (C) 2012, 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_HTS_ENGINE_CALL_HPP
#define RHVOICE_HTS_ENGINE_CALL_HPP

#include "exception.hpp"
#include "client.hpp"
#include "utterance.hpp"
#include "hts_engine_pool.hpp"
#include "hts_input.hpp"
#include "speech_processing_chain.hpp"

namespace RHVoice
{
  class hts_engine_call
  {
  public:
    class client_error: public exception
    {
    public:
      explicit client_error(const std::string& msg):
        exception(msg)
      {
      }
    };

    hts_engine_call(hts_engine_pool& pool,const utterance& u,client& player);
    ~hts_engine_call();
    bool execute();

  private:
    hts_engine_call(const hts_engine_call&);
    hts_engine_call& operator=(const hts_engine_call&);

    void set_input();
    void set_output();

    void add_label(const item& seg);

    void add_labels(relation::const_iterator start,relation::const_iterator end)
    {
      for(relation::const_iterator it=start;it!=end;++it)
        add_label(*it);
    }

    const utterance& utt;
    client& player;
    hts_engine_pool& engine_pool;
    hts_engine_impl::pointer engine_impl;
    hts_input input;
    speech_processing_chain output;
  };
}
#endif
