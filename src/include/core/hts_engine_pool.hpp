/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
#include <list>
#include "threading.hpp"
#include "std_hts_engine_impl.hpp"

namespace RHVoice
{
  class hts_engine_pool
  {
  public:
    explicit hts_engine_pool(const std::string& voice_path)
    {
      prototypes.push_back(hts_engine_impl::pointer(new std_hts_engine_impl(voice_path)));
    }

    hts_engine_impl::pointer acquire(const std::string& name)
    {
      hts_engine_impl::pointer result(get_instance(name));
      if(result.empty())
        result=get_prototype(name)->create();
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

    hts_engine_impl::pointer get_instance(const std::string& name)
    {
      hts_engine_impl::pointer result;
      threading::lock l(inst_mutex);
      for(engine_list::iterator it=instances.begin();it!=instances.end();++it)
        {
          if((*it)->get_name()==name)
            {
              result=*it;
              instances.erase(it);
              break;
            }
        }
      return result;
    }

    hts_engine_impl::pointer get_prototype(const std::string& name) const
    {
      hts_engine_impl::pointer result;
      for(engine_list::const_iterator it=prototypes.begin();it!=prototypes.end();++it)
        {
          if((*it)->get_name()==name)
            {
              result=*it;
              break;
            }
        }
      return result;
    }

    engine_list prototypes,instances;
    threading::mutex inst_mutex;
  };
}
#endif
