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

#include <algorithm>
#include <cmath>
#include "core/str.hpp"
#include "core/mage_hts_engine_impl.hpp"
#include "HTS_engine.h"
#include "mage.h"

namespace RHVoice
{
  mage_hts_engine_impl::mage_hts_engine_impl(const std::string& voice_path):
    hts_engine_impl("mage",voice_path)
  {
  }

  hts_engine_impl::pointer mage_hts_engine_impl::do_create() const
  {
    return pointer(new mage_hts_engine_impl(data_path));
  }

  void mage_hts_engine_impl::do_initialize()
  {
    arg_list args;
    model_file_list dur_files(data_path,"dur");
    append_model_args(args,dur_files,"-td","-md");
    model_file_list mgc_files(data_path,"mgc",3);
    append_model_args(args,mgc_files,"-tm","-mm","-dm");
    model_file_list lf0_files(data_path,"lf0",3);
    append_model_args(args,lf0_files,"-tf","-mf","-df");
    model_file_list lpf_files(data_path,"lpf",1);
    append_model_args(args,lpf_files,"-tl","-ml","-dl");
    args.push_back(arg("-s",str::to_string(sample_rate.get())));
    args.push_back(arg("-p",str::to_string(fperiod)));
    args.push_back(arg("-a",str::to_string(alpha)));
    args.push_back(arg("-b",str::to_string(beta.get())));
    args.push_back(arg("-u",str::to_string(msd_threshold.get())));
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
  }

  void mage_hts_engine_impl::do_synthesize()
  {
    setup();
    int time=0;
    int dur=0;
    for(label_sequence::iterator label_iter=input->lbegin();label_iter!=input->lend();++label_iter)
      {
        label_iter->set_time(time);
        generate_parameters(*label_iter);
        dur=mage->getDuration()*fperiod;
        label_iter->set_duration(dur);
        time+=dur;
        generate_samples(*label_iter);
        if(output->is_stopped())
          return;
      }
  }

  void mage_hts_engine_impl::do_reset()
  {
    mage->reset();
    HTS_Vocoder_clear(vocoder.get());
    MAGE::FrameQueue* fq=mage->getFrameQueue();
    mage->setFrameQueue(0);
    delete fq;
    MAGE::ModelQueue* mq=mage->getModelQueue();
    mage->setModelQueue(0);
    delete mq;
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
    HTS_Vocoder_initialize(vocoder.get(),MAGE::nOfMGCs-1,0,1,sample_rate,fperiod);
  }

  void mage_hts_engine_impl::generate_parameters(hts_label& lab)
  {
    MAGE::Label mlab(lab.get_name());
    mage->setLabel(mlab);
    mage->prepareModel();
    mage->computeDuration();
    mage->computeParameters();
    mage->optimizeParameters();
  }

  void mage_hts_engine_impl::generate_samples(hts_label& lab)
  {
    double pitch=lab.get_pitch();
    double mgc[MAGE::nOfMGCs];
    double* lpf=0;
    int nlpf=(MAGE::nOfLPFs-1)/2;
    MAGE::FrameQueue* fq=mage->getFrameQueue();
    while(!(output->is_stopped()||fq->isEmpty()))
      {
        MAGE::Frame* f=fq->get();
        std::copy(f->streams[MAGE::mgcStreamIndex],f->streams[MAGE::mgcStreamIndex]+MAGE::nOfMGCs,mgc);
        double lf0=(f->voiced)?(f->streams[MAGE::lf0StreamIndex][0]):LZERO;
        if(f->voiced&&(pitch!=1))
          {
            double f0=std::exp(lf0)*pitch;
            if(f0<20)
              f0=20;
            lf0=std::log(f0);
          }
        lpf=f->streams[MAGE::lpfStreamIndex];
        fq->pop();
        HTS_Audio audio;
        audio.data=this;
        HTS_Vocoder_synthesize(vocoder.get(),MAGE::nOfMGCs-1,lf0,mgc,nlpf,lpf,alpha,beta,1,0,&audio);
      }
  }
}
