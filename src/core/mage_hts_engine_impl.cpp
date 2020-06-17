/* Copyright (C) 2013, 2014, 2017, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <algorithm>
#include <cmath>
#include "core/str.hpp"
#include "core/voice.hpp"
#include "core/mage_hts_engine_impl.hpp"
#include "HTS106_engine.h"
#include "HTS_hidden.h"
#include "mage.h"

namespace RHVoice
{
  mage_hts_engine_impl::model_file_list::model_file_list(const std::string& voice_path,const std::string& type,int num_windows_):
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

  mage_hts_engine_impl::mage_hts_engine_impl(const voice_info& info):
    hts_engine_impl("mage",info)
  {
    bpf_init(&bpf);
  }

  mage_hts_engine_impl::~mage_hts_engine_impl()
  {
    bpf_clear(&bpf);
}

  hts_engine_impl::pointer mage_hts_engine_impl::do_create() const
  {
    return pointer(new mage_hts_engine_impl(info));
  }

  void mage_hts_engine_impl::do_initialize()
  {
    configure_for_sample_rate();
    std::string bpf_path(path::join(model_path,"bpf.txt"));
    if(!bpf_load(&bpf,bpf_path.c_str()))
      throw initialization_error();
    arg_list args;
    model_file_list dur_files(model_path,"dur");
    append_model_args(args,dur_files,"-td","-md");
    model_file_list mgc_files(model_path,"mgc",3);
    append_model_args(args,mgc_files,"-tm","-mm","-dm");
    model_file_list lf0_files(model_path,"lf0",3);
    append_model_args(args,lf0_files,"-tf","-mf","-df");
    model_file_list ap_files(model_path,"bap",3);
    append_model_args(args,ap_files,"-tl","-ml","-dl");
    args.push_back(arg("-s",str::to_string(sample_rate.get())));
    args.push_back(arg("-p",str::to_string(frame_shift)));
    args.push_back(arg("-a",str::to_string(alpha)));
    args.push_back(arg("-b",str::to_string(beta.get())));
    args.push_back(arg("-u","0.5"));
    std::vector<char*> c_args;
    char name[]="RHVoice";
    c_args.push_back(name);
    for(arg_list::const_iterator it=args.begin();it!=args.end();++it)
      {
        c_args.push_back(const_cast<char*>(it->first.c_str()));
        c_args.push_back(const_cast<char*>(it->second.c_str()));
      }
    mage.reset(new MAGE::Mage("default",c_args.size(),&c_args[0]));
    vocoder.reset(new HTS_Vocoder);
    num_frames=0;
  }

  void mage_hts_engine_impl::do_synthesize()
  {
    setup();
    int pos=0;
    int len=0;
    for(label_sequence::iterator label_iter=input->lbegin();label_iter!=input->lend();++label_iter)
      {
        label_iter->set_position(pos);
        label_iter->set_time(pos*MAGE::defaultFrameRate);
        generate_parameters(*label_iter);
        len=mage->getDuration();
        label_iter->set_length(len);
        label_iter->set_duration(len*MAGE::defaultFrameRate);
        pos+=len;
        generate_samples(*label_iter);
        if(output->is_stopped())
          break;
      }
    pitch_editor.finish();
    do_generate_samples();
  }

  void mage_hts_engine_impl::do_reset()
  {
    mage->reset();
    HTS_Vocoder_clear(vocoder.get());
    MAGE::FrameQueue* fq=mage->getFrameQueue();
    unsigned int n=fq->getNumOfItems();
    if(n!=0)
      fq->pop(n);
    MAGE::ModelQueue* mq=mage->getModelQueue();
    n=mq->getNumOfItems();
    if(n!=0)
      mq->pop(n);
    num_frames=0;
    while(!frames.empty())
      frames.pop();
  }

  void mage_hts_engine_impl::append_model_args(arg_list& args,const model_file_list& files,const std::string& tree_arg_name,const std::string& pdf_arg_name,const std::string& win_arg_name) const
  {
    args.push_back(arg(tree_arg_name,files.tree));
    args.push_back(arg(pdf_arg_name,files.pdf));
    for(int i=0;i<files.num_windows;++i)
      args.push_back(arg(win_arg_name,files.windows[i]));
  }

  void mage_hts_engine_impl::setup()
  {
    if(mage->getModelQueue()==0)
      {
        MAGE::ModelQueue* mq=new MAGE::ModelQueue(MAGE::maxModelQueueLen);
        mage->setModelQueue(mq);
        MAGE::FrameQueue* fq=new MAGE::FrameQueue(MAGE::maxFrameQueueLen);
        mage->setFrameQueue(fq);
      }
    HTS_Vocoder_initialize(vocoder.get(),mgc_order,0,1,sample_rate.get(),frame_shift);
  }

  void mage_hts_engine_impl::generate_parameters(hts_label& lab)
  {
    MAGE::Label mlab(lab.get_name());
    double dur_mod=rate;
    if(lab.get_segment().has_feature("dur_mod"))
      dur_mod/=lab.get_segment().get("dur_mod").as<double>();
    if(dur_mod!=1)
      mlab.setSpeed(dur_mod);
    if(lab.get_time()==0)
      {
        mlab.setEnd(250000);
        mlab.setDurationForced(true);
}
    mage->setLabel(mlab);
    mage->prepareModel();
    mage->computeDuration();
    mage->computeParameters();
    mage->optimizeParameters();
  }

  void mage_hts_engine_impl::generate_samples(hts_label& lab)
  {
    MAGE::FrameQueue* fq=mage->getFrameQueue();
    frame_t f;
    f.index=num_frames;
    while(!(output->is_stopped()||fq->isEmpty()))
      {
        MAGE::Frame* f0=fq->get();
        std::copy(f0->streams[MAGE::mgcStreamIndex],f0->streams[MAGE::mgcStreamIndex]+mgc_order+1,f.mgc);
  std::copy(f0->streams[MAGE::bapStreamIndex],f0->streams[MAGE::bapStreamIndex]+bap_order+1,f.ap);
  for(int i=0;i<=bap_order;++i)
    {
      if(f.ap[i]>0)
        f.ap[i]=0;
      f.ap[i]=std::pow(10.0,f.ap[i]/10.0);
    }
  f.voiced=f0->voiced;
        f.lf0=(f0->voiced)?(f0->streams[MAGE::lf0StreamIndex][0]):LZERO;
        fq->pop();
        if(pitch_editor.has_work())
          {
            if(f.voiced)
              pitch_editor.append(f.lf0);
            else
              pitch_editor.append();
            frames.push(f);
            do_generate_samples();
}
        else
          {
            if(f.voiced)
              f.lf0+=pitch_shift;
            do_generate_samples(f);
}
        ++f.index;
        ++num_frames;
      }
  }

  bool mage_hts_engine_impl::supports_quality(quality_t q) const
  {
    if(q>quality_std)
      return false;
    if(quality<=quality_none)
      return true;
    return (q==quality);
  }

  void mage_hts_engine_impl::configure_for_sample_rate()
  {
    sample_rate=get_sample_rate_for_quality(quality);
    switch(sample_rate.get())
      {
      case sample_rate_16k:
        frame_shift=80;
        alpha=0.42;
        mgc_order=24;
        bap_order=4;
        break;
      default:
        frame_shift=120;
        alpha=0.466;
        mgc_order=30;
        bap_order=(info.get_format()==3)?11:6;
        break;
}
    speech.resize(frame_shift,0);
}

  void mage_hts_engine_impl::do_generate_samples(frame_t& f)
  {
        HTS_Vocoder_synthesize(vocoder.get(),mgc_order,f.lf0,&(f.mgc[0]),&(f.ap[0]),&bpf,alpha,beta,1,&(speech[0]),0);
        for(int i=0;i<frame_shift;++i)
          {
            speech[i]/=32768.0;
          }
        output->process(&(speech[0]),frame_shift);
}

  void mage_hts_engine_impl::do_generate_samples()
  {
    while(!output->is_stopped()&&!frames.empty())
      {
        frame_t& f=frames.front();
        if(!pitch_editor.has_result(f.index))
          return;
        if(f.voiced)
          f.lf0=pitch_editor.get_result(f.index)+pitch_shift;
        do_generate_samples(f);
        frames.pop();
}
}
}
