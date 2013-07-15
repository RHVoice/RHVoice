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

#include "core/str.hpp"
#include "core/path.hpp"
#include "core/config.hpp"
#include "core/hts_engine_impl.hpp"
#include "HTS_engine.h"

extern "C"
{
  void HTS_Audio_initialize(HTS_Audio * audio, int sampling_rate, int max_buff_size)
  {
  }

  void HTS_Audio_set_parameter(HTS_Audio * audio, int sampling_rate, int max_buff_size)
  {
  }

  void HTS_Audio_write(HTS_Audio * audio, short sample)
  {
    static_cast<RHVoice::hts_engine_impl*>(audio->data)->on_new_sample(sample);
  }

  void HTS_Audio_flush(HTS_Audio * audio)
  {
  }

  void HTS_Audio_clear(HTS_Audio * audio)
  {
  }
}

namespace RHVoice
{
  hts_engine_impl::model_file_list::model_file_list(const std::string& voice_path,const std::string& type,int num_windows_):
    pdf(0),
    tree(0),
    num_windows(num_windows_)
  {
    file_names.push_back(path::join(voice_path,type+".pdf"));
    file_names.push_back(path::join(voice_path,"tree-"+type+".inf"));
    for(int i=0;i<num_windows;++i)
      {
        file_names.push_back(path::join(voice_path,type+".win"+str::to_string(i+1)));
      }
    pdf=const_cast<char*>(file_names[0].c_str());
    tree=const_cast<char*>(file_names[1].c_str());
    for(int i=0;i<num_windows;++i)
      {
        windows[i]=const_cast<char*>(file_names[i+2].c_str());
      }
  }

  hts_engine_impl::hts_engine_impl(const std::string& impl_name,const std::string& voice_path):
    data_path(voice_path),
    fperiod(80),
    alpha(0.42),
    msd_threshold("msd_threshold",0.5,0,1.0),
    beta("beta",0.4,-0.8,0.8),
    gain("gain",1.0,0.5,2.0),
    input(0),
    output(0),
    name(impl_name)
  {
    config cfg1;
    cfg1.register_setting(sample_rate);
    cfg1.load(path::join(data_path,"voice.info"));
    switch(sample_rate)
      {
      case sample_rate_22k:
        fperiod=110;
        alpha=0.45;
        break;
      case sample_rate_32k:
        fperiod=160;
        alpha=0.5;
        break;
      case sample_rate_44k:
        fperiod=220;
        alpha=0.54;
        break;
      case sample_rate_48k:
        fperiod=240;
        alpha=0.55;
        break;
      default:
        break;
      }
    config cfg2;
    cfg2.register_setting(msd_threshold);
    cfg2.register_setting(beta);
    cfg2.register_setting(gain);
    cfg2.load(path::join(data_path,"voice.params"));
  }

  void hts_engine_impl::synthesize()
  {
    if(input->lbegin()!=input->lend())
      do_synthesize();
    if(!output->is_stopped())
      output->finish();
  }

  void hts_engine_impl::reset()
  {
    if(input->lbegin()!=input->lend())
      do_reset();
    output=0;
    input=0;
  }

  void hts_engine_impl::load_configs()
  {
    config cfg1;
    cfg1.register_setting(sample_rate);
    cfg1.load(path::join(data_path,"voice.info"));
    switch(sample_rate)
      {
      case sample_rate_22k:
        fperiod=110;
        alpha=0.45;
        break;
      case sample_rate_32k:
        fperiod=160;
        alpha=0.5;
        break;
      case sample_rate_44k:
        fperiod=220;
        alpha=0.54;
        break;
      case sample_rate_48k:
        fperiod=240;
        alpha=0.55;
        break;
      default:
        break;
      }
    config cfg2;
    cfg2.register_setting(msd_threshold);
    cfg2.register_setting(beta);
    cfg2.load(path::join(data_path,"voice.params"));
  }

  hts_engine_impl::pointer hts_engine_impl::create() const
  {
    pointer p=do_create();
    p->load_configs();
    p->do_initialize();
    return p;
  }

  void hts_engine_impl::on_new_sample(short sample)
  {
    if(output->is_stopped())
      return;
    double s=(sample/32768.0)*gain;
    try
      {
        output->process(&s,1);
      }
    catch(...)
      {
        output->stop();
      }
  }
}
