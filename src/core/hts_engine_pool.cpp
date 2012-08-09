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

#include "core/io.hpp"
#include "core/path.hpp"
#include "core/config.hpp"
#include "core/voice.hpp"
#include "core/hts_engine_pool.hpp"

namespace RHVoice
{
  hts_engine_pool::hts_engine_pool(const voice_info& info):
  data_path(info.get_data_path()),
    sample_rate(info.get_sample_rate()),
  use_gv("use_gv",false),
    fperiod(80),
    alpha(0.42),
    beta("beta",0.4,-0.8,0.8),
    msd_threshold("msd_threshold",0.5,0,1.0)
  {
    config cfg(path::join(data_path,"voice.params"));
    cfg.register_setting(use_gv);
    cfg.register_setting(beta);
    cfg.register_setting(msd_threshold);
    cfg.load();
    switch(sample_rate)
      {
      case sample_rate_48k:
        fperiod=240;
        alpha=0.55;
        break;
      case sample_rate_44k:
        fperiod=220;
        alpha=0.53;
        break;
      default:
        break;
      }
    if(use_gv)
      beta=0;
  }

  smart_ptr<HTS_Engine> hts_engine_pool::create_instance()
  {
    hts_engine_deleter d;
    HTS_Engine* p=new HTS_Engine;
    HTS_Engine_initialize(p,3);
    smart_ptr<HTS_Engine> engine(p,d);
    io::file_handle fh[5];
    FILE* fp[5];
    fh[0]=io::open_file(path::join(data_path,"dur.pdf"),"rb");
    fp[0]=fh[0].get();
    fh[1]=io::open_file(path::join(data_path,"tree-dur.inf"),"r");
    fp[1]=fh[1].get();
    if(!HTS_Engine_load_duration_from_fp(engine.get(),&fp[0],&fp[1],1))
      throw hts_engine_initialization_error();
    fh[0]=io::open_file(path::join(data_path,"mgc.pdf"),"rb");
    fp[0]=fh[0].get();
    fh[1]=io::open_file(path::join(data_path,"tree-mgc.inf"),"r");
    fp[1]=fh[1].get();
    fh[2]=io::open_file(path::join(data_path,"mgc.win1"),"r");
    fp[2]=fh[2].get();
    fh[3]=io::open_file(path::join(data_path,"mgc.win2"),"r");
    fp[3]=fh[3].get();
    fh[4]=io::open_file(path::join(data_path,"mgc.win3"),"r");
    fp[4]=fh[4].get();
    if(!HTS_Engine_load_parameter_from_fp(engine.get(),&fp[0],&fp[1],&fp[2],0,false,3,1))
      throw hts_engine_initialization_error();
    fh[0]=io::open_file(path::join(data_path,"lf0.pdf"),"rb");
    fp[0]=fh[0].get();
    fh[1]=io::open_file(path::join(data_path,"tree-lf0.inf"),"r");
    fp[1]=fh[1].get();
    fh[2]=io::open_file(path::join(data_path,"lf0.win1"),"r");
    fp[2]=fh[2].get();
    fh[3]=io::open_file(path::join(data_path,"lf0.win2"),"r");
    fp[3]=fh[3].get();
    fh[4]=io::open_file(path::join(data_path,"lf0.win3"),"r");
    fp[4]=fh[4].get();
    if(!HTS_Engine_load_parameter_from_fp(engine.get(),&fp[0],&fp[1],&fp[2],1,true,3,1))
      throw hts_engine_initialization_error();
    fh[0]=io::open_file(path::join(data_path,"lpf.pdf"),"rb");
    fp[0]=fh[0].get();
    fh[1]=io::open_file(path::join(data_path,"tree-lpf.inf"),"r");
    fp[1]=fh[1].get();
    fh[2]=io::open_file(path::join(data_path,"lpf.win1"),"r");
    fp[2]=fh[2].get();
    if(!HTS_Engine_load_parameter_from_fp(engine.get(),&fp[0],&fp[1],&fp[2],2,false,1,1))
      throw hts_engine_initialization_error();
    if(use_gv)
      {
        fh[0]=io::open_file(path::join(data_path,"gv-mgc.pdf"),"rb");
        fp[0]=fh[0].get();
        fh[1]=io::open_file(path::join(data_path,"tree-gv-mgc.inf"),"r");
        fp[1]=fh[1].get();
        if(!HTS_Engine_load_gv_from_fp(engine.get(),&fp[0],&fp[1],0,1))
          throw hts_engine_initialization_error();
        fh[0]=io::open_file(path::join(data_path,"gv-lf0.pdf"),"rb");
        fp[0]=fh[0].get();
        fh[1]=io::open_file(path::join(data_path,"tree-gv-lf0.inf"),"r");
        fp[1]=fh[1].get();
        if(!HTS_Engine_load_gv_from_fp(engine.get(),&fp[0],&fp[1],1,1))
          throw hts_engine_initialization_error();
        fh[0]=io::open_file(path::join(data_path,"gv-switch.inf"),"r");
        if(!HTS_Engine_load_gv_switch_from_fp(engine.get(),fh[0].get()))
          throw hts_engine_initialization_error();
      }
    HTS_Engine_set_sampling_rate(engine.get(),sample_rate);
    HTS_Engine_set_fperiod(engine.get(),fperiod);
    HTS_Engine_set_alpha(engine.get(),alpha);
    HTS_Engine_set_gamma(engine.get(),0);
    HTS_Engine_set_log_gain(engine.get(),true);
    HTS_Engine_set_beta(engine.get(),beta);
    HTS_Engine_set_audio_buff_size(engine.get(),1600);
    HTS_Engine_set_msd_threshold(engine.get(),1,msd_threshold);
    HTS_Engine_set_duration_interpolation_weight(engine.get(),0,1.0);
    HTS_Engine_set_parameter_interpolation_weight(engine.get(),0,0,1.0);
    HTS_Engine_set_parameter_interpolation_weight(engine.get(),1,0,1.0);
    HTS_Engine_set_parameter_interpolation_weight(engine.get(),2,0,1.0);
    if(use_gv)
      {
        HTS_Engine_set_gv_weight(engine.get(),0,1.0);
        HTS_Engine_set_gv_weight(engine.get(),1,1.0);
        HTS_Engine_set_gv_interpolation_weight(engine.get(),0,0,1.0);
        HTS_Engine_set_gv_interpolation_weight(engine.get(),1,0,1.0);
      }
    return engine;
  }
}
