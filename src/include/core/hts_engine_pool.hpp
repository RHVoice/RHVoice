/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_HTS_ENGINE_POOL_HPP
#define RHVOICE_HTS_ENGINE_POOL_HPP

#include <string>
#include <list>
#include "threading.hpp"
#include "std_hts_engine_impl.hpp"
#ifdef ENABLE_MAGE
#include "mage_hts_engine_impl.hpp"
#endif
#include "quality_setting.hpp"


namespace RHVoice
{
  class voice_info;

  class hts_engine_pool
  {
  public:
    explicit hts_engine_pool(const voice_info& info_):
      info(info_)
    {
      prototypes.push_back(hts_engine_impl::pointer(new std_hts_engine_impl(info_)));
#ifdef ENABLE_MAGE
      prototypes.push_back(hts_engine_impl::pointer(new mage_hts_engine_impl(info_)));
#endif
    }

    hts_engine_impl::pointer acquire(quality_t quality)
    {
      hts_engine_impl::pointer result(get_instance(quality));
      if(result.empty())
        result=get_prototype(quality)->create(quality);
      return result;
    }

    void release(const hts_engine_impl::pointer& engine)
    {
      threading::lock l(inst_mutex);
      instances.push_front(engine);
    }

  private:
    hts_engine_pool(const hts_engine_pool&);
    hts_engine_pool& operator=(const hts_engine_pool&);

    typedef std::list<hts_engine_impl::pointer> engine_list;

    hts_engine_impl::pointer get_instance(quality_t quality )
    {
      hts_engine_impl::pointer result;
      threading::lock l(inst_mutex);
      for(engine_list::iterator it=instances.begin();it!=instances.end();++it)
        {
          if((*it)->supports_quality(quality))
            {
              result=*it;
              instances.erase(it);
              break;
            }
        }
      return result;
    }

    hts_engine_impl::pointer get_prototype(quality_t quality) const
    {
      hts_engine_impl::pointer result;
      for(engine_list::const_iterator it=prototypes.begin();it!=prototypes.end();++it)
        {
          if((*it)->supports_quality(quality))
            {
              result=*it;
              break;
            }
        }
      return result;
    }

    engine_list prototypes,instances;
    threading::mutex inst_mutex;
    const voice_info& info;
  };
}
#endif
