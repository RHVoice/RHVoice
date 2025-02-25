/* Copyright (C) 2013, 2014, 2018, 2019, 2022  Olga Yakovleva <olga@rhvoice.org> */

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

#ifndef RHVOICE_HTS_ENGINE_IMPL_HPP
#define RHVOICE_HTS_ENGINE_IMPL_HPP

#include <vector>
#include <string>
#include <cmath>
#include "exception.hpp"

#include "sample_rate.hpp"
#include "hts_input.hpp"
#include "speech_processing_chain.hpp"
#include "quality_setting.hpp"
#include "pitch.hpp"
#include "equalizer.hpp"
#include "unit_db.hpp"

struct _HTS_Audio;
extern "C" void HTS_Audio_write(_HTS_Audio * audio, short sample);
namespace RHVoice
{
  class voice_info;

  class hts_engine_impl
  {
    friend void ::HTS_Audio_write(_HTS_Audio * audio, short sample);
  public:
    typedef std::shared_ptr<hts_engine_impl> pointer;

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

    pointer create(quality_t quality) const;

    void set_input(hts_input& input_)
    {
      input=&input_;
      pitch_shift=std::log(2.0)*base_pitch_shift/12.0;
      if(input->lbegin()!=input->lend())
        {
          pitch_shift+=std::log(input->lbegin()->get_pitch());
          if(input->lbegin()->get_segment().get_relation().get_utterance().get_utt_type()=="e")
            pitch_shift+=std::log(2.0)*emph_shift/12.0;
        }
      pitch_editor.init(input);
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

    void set_rate(double value)
    {
      rate=value;
    }

    double get_gain() const
    {
      return gain;
}

    double get_native_rate() const
    {
      return native_rate;
    }

    virtual bool supports_quality(quality_t q) const=0;

    bool uses_eq() const {return (eq!=nullptr);}

  protected:
    explicit hts_engine_impl(const std::string& name,const voice_info& info_);
    virtual sample_rate_t get_sample_rate_for_quality(quality_t q) const;

    const voice_info& info;
    std::string data_path;
    std::string model_path;
    sample_rate_property sample_rate;
    numeric_property<double> beta;
    numeric_property<double> gain;
    numeric_property<double> native_rate;
    numeric_property<double> voicing;
    quality_t quality;
    numeric_property<unsigned int> int_key;
    numeric_property<double> emph_shift;
    numeric_property<double> base_pitch_shift;
    std::unique_ptr<equalizer> eq;
    std::unique_ptr<unit_db> units;

    double get_emph_shift() const
    {
      return std::log(2.0)*emph_shift/12.0;
}

    hts_input* input;
    speech_processing_chain* output;
    double rate;
    double pitch_shift;
    pitch::editor pitch_editor;

  private:
    hts_engine_impl(const hts_engine_impl&);
    hts_engine_impl& operator=(const hts_engine_impl&);

    void on_new_sample(short sample);

    void load_configs();

    void set_quality(quality_t q);
    virtual pointer do_create() const=0;
    virtual void do_initialize()=0;
    virtual void do_synthesize()=0;
    virtual void do_reset()=0;

    virtual void do_stop()
    {
}

    std::string name;
  };
}
#endif
