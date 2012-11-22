/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_HTS_ENGINE_POOL_HPP
#define RHVOICE_HTS_ENGINE_POOL_HPP

#include <string>
#include <vector>
#include "property.hpp"
#include "HTS_engine.h"
#include "smart_ptr.hpp"
#include "exception.hpp"
#include "threading.hpp"

namespace RHVoice
{
  class voice_info;

  enum sample_rate_t
    {
      sample_rate_16k=16000,
      sample_rate_44k=44100,
      sample_rate_48k=48000
    };

  class hts_engine_initialization_error: public exception
  {
  public:
    hts_engine_initialization_error():
      exception("Hts engine initialization failed")
    {
    }
  };

  class hts_engine_pool
  {
  public:
    explicit hts_engine_pool(const voice_info& info);

    smart_ptr<HTS_Engine> acquire()
    {
      smart_ptr<HTS_Engine> result(get_instance());
      return (result.empty()?create_instance():result);
    }

    void release(const smart_ptr<HTS_Engine>& engine)
    {
      threading::lock l(list_mutex);
      engine_list.push_back(engine);
    }

  private:
    hts_engine_pool(const hts_engine_pool&);
    hts_engine_pool& operator=(const hts_engine_pool&);

    struct hts_engine_deleter
    {
      void operator()(HTS_Engine* p) const
      {
        HTS_Engine_clear(p);
        delete p;
      }
    };

    smart_ptr<HTS_Engine> get_instance()
    {
      smart_ptr<HTS_Engine> result;
      threading::lock l(list_mutex);
      if(!engine_list.empty())
        {
          result=engine_list.back();
          engine_list.pop_back();
        }
      return result;
    }

    smart_ptr<HTS_Engine> create_instance();

    std::vector<smart_ptr<HTS_Engine> > engine_list;
    threading::mutex list_mutex;
    std::string data_path;

    sample_rate_t sample_rate;
    bool_property use_gv;
    int fperiod;
    double alpha;
    numeric_property<double> beta;
    numeric_property<double> msd_threshold;
  };
}
#endif
