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

#ifndef RHVOICE_UTTERANCE_HPP
#define RHVOICE_UTTERANCE_HPP

#include <string>
#include <map>
#include <algorithm>
#include "exception.hpp"
#include "smart_ptr.hpp"
#include "relation.hpp"
#include "quality_setting.hpp"

namespace RHVoice
{
  class language;
  class voice;

  class relation_exists: public exception
  {
  public:
    explicit relation_exists():
      exception("Relation already exists")
    {
    }
  };

  class relation_not_found: public lookup_error
  {
  public:
    explicit relation_not_found():
      lookup_error("Relation not found")
    {
    }
  };

  class utterance
  {
  public:
    explicit utterance(const language& language_ref_):
    language_ref(language_ref_),
    voice_ptr(0),
    absolute_rate(0),
    relative_rate(1.0),
    absolute_pitch(0),
    relative_pitch(1.0),
    absolute_volume(0),
    relative_volume(1.0)
    {
    }

    const language& get_language() const
    {
      return language_ref;
    }

    bool has_voice() const
    {
      return voice_ptr;
    }

    void set_voice(const voice& voice_ref)
    {
      voice_ptr=&voice_ref;
    }

    const voice& get_voice() const
    {
      return *voice_ptr;
    }

    void set_quality(quality_t q)
    {
      quality=q;
    }

    quality_t get_quality() const
    {
      return quality;
    }

    double get_absolute_rate() const
    {
      return absolute_rate;
    }

    void set_absolute_rate(double val)
    {
      absolute_rate=std::min(std::max(-1.0,val),1.0);
    }

    double get_absolute_pitch() const
    {
      return absolute_pitch;
    }

    void set_absolute_pitch(double val)
    {
      absolute_pitch=std::min(std::max(-1.0,val),1.0);
    }

    double get_absolute_volume() const
    {
      return absolute_volume;
    }

    void set_absolute_volume(double val)
    {
      absolute_volume=std::min(std::max(-1.0,val),1.0);
    }

    double get_relative_rate() const
    {
      return relative_rate;
    }

    void set_relative_rate(double val)
    {
      if(val>0)
        relative_rate=val;
    }

    double get_relative_pitch() const
    {
      return relative_pitch;
    }

    void set_relative_pitch(double val)
    {
      if(val>0)
        relative_pitch=val;
    }

    double get_relative_volume() const
    {
      return relative_volume;
    }

    void set_relative_volume(double val)
    {
      if(val>0)
        relative_volume=val;
    }

    relation& add_relation(const std::string& name);
    void remove_relation(const std::string& name);

    bool has_relation(const std::string& name) const
    {
      return (relations.find(name)!=relations.end());
    }

    const relation& get_relation(const std::string& name) const
    {
      relation_map::const_iterator it(relations.find(name));
      if(it==relations.end())
        throw relation_not_found();
      return *(it->second);
    }

    relation& get_relation(const std::string& name,bool create=false)
    {
      relation_map::iterator it(relations.find(name));
      if(it==relations.end())
        {
          if(create)
            return add_relation(name);
          else
            throw relation_not_found();
        }
      else
        return *(it->second);
    }

  private:
    utterance(const utterance&);
    utterance& operator=(const utterance&);

    typedef std::map<std::string,smart_ptr<relation> > relation_map;

    const language& language_ref;
    const voice* voice_ptr;
    quality_t quality;
    relation_map relations;
    double absolute_rate,relative_rate,absolute_pitch,relative_pitch,absolute_volume,relative_volume;
  };

  class utterance_function
  {
  public:
    virtual ~utterance_function()
    {
    }

    virtual void execute(utterance& u) const=0;

  protected:
    utterance_function()
    {
    }

  private:
    utterance_function(const utterance_function&);
    utterance_function& operator=(const utterance_function&);
  };
}
#endif
