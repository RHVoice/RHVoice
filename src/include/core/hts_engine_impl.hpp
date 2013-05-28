/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef RHVOICE_HTS_ENGINE_IMPL_HPP
#define RHVOICE_HTS_ENGINE_IMPL_HPP

#include <vector>
#include <string>
#include "exception.hpp"
#include "smart_ptr.hpp"
#include "sample_rate.hpp"
#include "hts_input.hpp"
#include "speech_processing_chain.hpp"

struct _HTS_Audio;
extern "C" void HTS_Audio_write(_HTS_Audio * audio, short sample);

namespace RHVoice
{
  class hts_engine_impl
  {
    friend void ::HTS_Audio_write(_HTS_Audio * audio, short sample);
  public:
    typedef smart_ptr<hts_engine_impl> pointer;

    class error: public exception
    {
    public:
      explicit error(const std::string& msg):
      exception(msg)
      {
      }
    };

    class initialization_error: public error
    {
    public:
      initialization_error():
        error("HTS Engine initialization error")
      {
      }
    };

    class synthesis_error: public error
    {
    public:
      synthesis_error():
        error("HTS synthesis error")
      {
      }
    };

    virtual ~hts_engine_impl()
    {
    }

    const std::string& get_name() const
    {
      return name;
    }

    pointer create() const;

    void set_input(hts_input& input_)
    {
      input=&input_;
    }

    void set_output(speech_processing_chain& output_)
    {
      output=&output_;
    }

    void synthesize();
    void reset();

    sample_rate_t get_sample_rate() const
    {
      return sample_rate;
    }

  protected:
    explicit hts_engine_impl(const std::string& name,const std::string& voice_path);

    std::string data_path;
    sample_rate_property sample_rate;
    int fperiod;
    double alpha;
    numeric_property<double> msd_threshold;
    numeric_property<double> beta;

    hts_input* input;
    speech_processing_chain* output;

    class model_file_list
    {
    public:
      model_file_list(const std::string& voice_path,const std::string& type,int num_windows_=0);

      char* pdf;
      char* tree;
      int num_windows;
      char* windows[3];
               
    private:
      model_file_list(const model_file_list&);
      model_file_list& operator=(const model_file_list&);

      std::vector<std::string> file_names;
    };

  private:
    hts_engine_impl(const hts_engine_impl&);
    hts_engine_impl& operator=(const hts_engine_impl&);

    void on_new_sample(short sample);

    void load_configs();

    virtual pointer do_create() const=0;
    virtual void do_initialize()=0;
    virtual void do_synthesize()=0;
    virtual void do_reset()=0;

    std::string name;
  };
}
#endif
