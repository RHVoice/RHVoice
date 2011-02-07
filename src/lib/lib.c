/* Copyright (C) 2009, 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <HTS_engine.h>
#include "sonic.h"
#include "lib.h"

static const char *lib_version=VERSION;

static cst_voice *voice=NULL;
static HTS_Engine ru_engine;
static RHVoice_callback user_callback=NULL;

typedef struct _RHVoice_callback_info {
  RHVoice_callback user_callback;
  short *buff;
  int user_buff_size;
  int max_buff_size;
  int cur_buff_size;
  sonicStream stream;
  cst_item *cur_seg;
  int cur_seg_nsamples;
  int result;
} RHVoice_callback_info;

static int synth_callback(const short *samples,int nsamples,void *user_data)
{
  RHVoice_callback_info *info;
  int i,sonic_nsamples;
  float volume,s;
  cst_item *t;
  if(user_data==NULL)
    return 0;
  info=(RHVoice_callback_info*)user_data;
  if(info->cur_seg_nsamples==item_feat_int(info->cur_seg,"nsamples"))
    {
      info->cur_seg=item_next(info->cur_seg);
      info->cur_seg_nsamples=nsamples;
    }
  else
    info->cur_seg_nsamples+=nsamples;
  if(cst_streq(item_name(info->cur_seg),"pau")&&item_feat_present(info->cur_seg,"max_nsamples"))
    {
      if(!item_prev(info->cur_seg))
        {
          if((item_feat_int(info->cur_seg,"nsamples")-info->cur_seg_nsamples)>=item_feat_int(info->cur_seg,"max_nsamples"))
            return 1;
        }
      else
        if(!item_next(info->cur_seg))
          {
            if(info->cur_seg_nsamples>item_feat_int(info->cur_seg,"max_nsamples"))
              return 0;
          }
    }
  volume=get_param_float(item_utt(info->cur_seg)->features,"volume",1.0);
  t=path_to_item(info->cur_seg,"R:Transcription.parent.R:Token.parent");
  if(t&&item_feat_present(t,"volume"))
    volume=item_feat_float(t,"volume");
  if(volume<=0)
    volume=1.0;
  for(i=0;i<nsamples;i++)
    {
      if(volume==1.0)
        s=samples[i];
      else
        {
          s=(float)samples[i]*volume;
          if(s<-32766)
            s=-32766;
          else
            if(s>32766)
              s=32766;
            else
              s=(short)(s+0.5);
        }
      info->buff[info->cur_buff_size+i]=s;
    }
  info->cur_buff_size+=nsamples;
  if(info->cur_buff_size>=info->user_buff_size)
    {
      if(info->stream==NULL)
        {
          info->result=info->user_callback(info->buff,info->cur_buff_size,NULL,0);
          info->cur_buff_size=0;
        }
      else
        {
          sonicWriteShortToStream(info->stream,info->buff,info->cur_buff_size);
          info->cur_buff_size=0;
          sonic_nsamples=sonicSamplesAvailable(info->stream);
          if(sonic_nsamples>0)
            {
              if(sonic_nsamples>info->max_buff_size)
                {
                  info->buff=realloc(info->buff,sonic_nsamples*sizeof(short));
                  info->max_buff_size=sonic_nsamples;
                }
              sonicReadShortFromStream(info->stream,info->buff,sonic_nsamples);
              info->result=info->user_callback(info->buff,sonic_nsamples,NULL,0);
            }
        }
    }
  return info->result;
}

#define num_hts_data_files 17

static int open_hts_data_files(const char *path,FILE *file_list[num_hts_data_files]);
static void close_hts_data_files(FILE *file_list[num_hts_data_files]);
static cst_utterance *hts_synth(cst_utterance *utt);

typedef struct
{
  const char *name;
  const char *open_mode;
  int is_optional;
} hts_data_file_info;

static const hts_data_file_info hts_data_list[num_hts_data_files]={
  {"dur.pdf","rb",0},
  {"tree-dur.inf","r",0},
  {"mgc.pdf","rb",0},
  {"tree-mgc.inf","r",0},
  {"mgc.win1","r",0},
  {"mgc.win2","r",0},
  {"mgc.win3","r",0},
  {"lf0.pdf","rb",0},
  {"tree-lf0.inf","r",0},
  {"lf0.win1","r",0},
  {"lf0.win2","r",0},
  {"lf0.win3","r",0},
  {"gv-mgc.pdf","rb",1},
  {"tree-gv-mgc.inf","r",1},
  {"gv-lf0.pdf","rb",1},
  {"tree-gv-lf0.inf","r",1},
  {"gv-switch.inf","r",1}};

static int open_hts_data_files(const char *path,FILE *file_list[num_hts_data_files])
{
  int result=1;
  int i,l;
  char sep='/';
  char *full_path;
  char *name;
  memset(file_list,0,num_hts_data_files*sizeof(FILE*));
  l=strlen(path);
  if(l==0)
    return 0;
  full_path=calloc(l+20,sizeof(char));
  if(full_path==NULL)
    return 0;
  memcpy(full_path,path,l);
  if(full_path[l-1]!=sep)
    {
      full_path[l]=sep;
      name=full_path+l+1;
    }
  else
    name=full_path+l;
  for(i=0;i<num_hts_data_files;i++)
    {
      strcpy(name,hts_data_list[i].name);
      file_list[i]=fopen(full_path,hts_data_list[i].open_mode);
      if((file_list[i]==NULL)&&(!hts_data_list[i].is_optional))
        {
          result=0;
          break;
        }
    }
  free(full_path);
  if(!result)
    close_hts_data_files(file_list);
  return result;
}

static void close_hts_data_files(FILE *file_list[num_hts_data_files])
{
  int i;
  for(i=0;i<num_hts_data_files;i++)
    {
      if(file_list[i]!=NULL)
        {
          fclose(file_list[i]);
          file_list[i]=NULL;
        }
    }
}

int RHVoice_initialize(const char *path,RHVoice_callback callback)
{
  int i,j;
  FILE *fp[num_hts_data_files];
  int use_gv=1;
  int sampling_rate=16000;
  int fperiod=80;
  float alpha=0.42;
  float stage=0.0;
  float beta=0.4;
  float uv_threshold=0.5;
  float gv_weight_lf0=0.7;
  float gv_weight_mcp=1.0;
  HTS_Boolean use_log_gain=TRUE;
  if((path==NULL)||(callback==NULL)) return 0;
  user_callback=callback;
  voice = new_voice();
  if(voice==NULL) return 0;
  HTS_Engine_initialize(&ru_engine,2);
  HTS_Engine_set_SynthCallback(&ru_engine,synth_callback);
  russian_init(voice);
  feat_set_string(voice->features,"name","Russian HTS voice");
  feat_set_string(voice->features,"no_segment_duration_model","1");
  feat_set_string(voice->features,"no_f0_target_model","1");
  feat_set(voice->features,"wave_synth_func",uttfunc_val(&hts_synth));
  if(!open_hts_data_files(path,fp))
    {
      RHVoice_terminate();
      return 0;
    }
  i=0;
  HTS_Engine_load_duration_from_fp(&ru_engine,&fp[i],&fp[i+1],1);
  i+=2;
  HTS_Engine_load_parameter_from_fp(&ru_engine,&fp[i],&fp[i+1],&fp[i+2],0,FALSE,3,1);
  i+=5;
  HTS_Engine_load_parameter_from_fp(&ru_engine,&fp[i],&fp[i+1],&fp[i+2],1,TRUE,3,1);
  i+=5;
  for(j=i;j<num_hts_data_files;j++)
    {
      if(fp[j]==NULL)
        {
        use_gv=0;
        break;
        }
    }
  if(use_gv)
    {
      beta=0;
      HTS_Engine_load_gv_from_fp(&ru_engine,&fp[i],&fp[i+1],0,1);
      i+=2;
      HTS_Engine_load_gv_from_fp(&ru_engine,&fp[i],&fp[i+1],1,1);
      HTS_Engine_load_gv_switch_from_fp(&ru_engine,fp[i]);
    }
  close_hts_data_files(fp);
  HTS_Engine_set_sampling_rate(&ru_engine,sampling_rate);
  HTS_Engine_set_fperiod(&ru_engine,fperiod);
  HTS_Engine_set_alpha(&ru_engine,alpha);
  HTS_Engine_set_gamma(&ru_engine,stage);
  HTS_Engine_set_log_gain(&ru_engine,use_log_gain);
  HTS_Engine_set_beta(&ru_engine,beta);
  HTS_Engine_set_msd_threshold(&ru_engine,1,uv_threshold);
  if(use_gv)
    {
      HTS_Engine_set_gv_weight(&ru_engine,0,gv_weight_mcp);
      HTS_Engine_set_gv_weight(&ru_engine,1,gv_weight_lf0);
    }
  HTS_Engine_set_duration_interpolation_weight(&ru_engine,0,1.0);
  HTS_Engine_set_parameter_interpolation_weight(&ru_engine,0,0,1.0);
  HTS_Engine_set_parameter_interpolation_weight(&ru_engine,1,0,1.0);
  if(use_gv)
    {
      HTS_Engine_set_gv_interpolation_weight(&ru_engine,0,0,1.0);
      HTS_Engine_set_gv_interpolation_weight(&ru_engine,1,0,1.0);
    }
  return sampling_rate;
}

void RHVoice_terminate()
{
  if(voice==NULL) return;
  HTS_Engine_clear(&ru_engine);
  delete_voice(voice);
  voice=NULL;
}

int RHVoice_load_user_dict(cst_voice *vox,const char *path)
{
  ru_user_dict *old_dict=NULL;
  ru_user_dict *new_dict=NULL;
  if(vox==NULL)
    return FALSE;
  if(path!=NULL)
    {
      new_dict=ru_user_dict_load(path);
      if(new_dict==NULL)
        return FALSE;
    }
  if(feat_present(vox->features,"user_dict"))
    {
      old_dict=(ru_user_dict*)val_userdata(feat_val(vox->features,"user_dict"));
      feat_remove(vox->features,"user_dict");
      ru_user_dict_free(old_dict);
    }
  if(new_dict!=NULL)
    feat_set(vox->features,"user_dict",userdata_val(new_dict));
  return TRUE;
}

const char *RHVoice_get_version()
{
  return lib_version;
}

static cst_utterance *hts_synth(cst_utterance *u)
{
  HTS_LabelString *lstring=NULL;
  double *dur_mean,*dur_vari,local_dur;
  char **labels=NULL;
  int i,j,nsamples,nlabels;
  cst_item *s,*t;
  float f0_shift,dur_stretch,local_f0_shift,local_dur_stretch,hts_local_f0;
  const char *last_punc=NULL;
  float final_pause_max_len=0.0;
  RHVoice_callback_info ci;
  dur_stretch=get_param_float(u->features,"duration_stretch",1.0);
  f0_shift=get_param_float(u->features,"f0_shift",1.0);
  t=relation_tail(utt_relation(u,"Token"));
  last_punc=item_feat_string(t,"punc");
  if(dur_stretch>=0.5)
    {
      if(item_feat_present(t,"duration_stretch"))
        {
          local_dur_stretch=item_feat_float(t,"duration_stretch");
          if(local_dur_stretch<0)
            local_dur_stretch=1;
        }
      else
        local_dur_stretch=dur_stretch;
    }
  else
    local_dur_stretch=1.0;
  if(last_punc)
    {
      if(strchr(last_punc,'.')||
         strchr(last_punc,'?')||
         strchr(last_punc,'!'))
        final_pause_max_len=0.25*local_dur_stretch;
      else
        if(strchr(last_punc,':')||
           strchr(last_punc,';'))
          final_pause_max_len=0.175*local_dur_stretch;
        else
          if(strchr(last_punc,',')||
             strchr(last_punc,'-'))
            final_pause_max_len=0.1*local_dur_stretch;
    }
  for(nlabels=0,s=relation_head(utt_relation(u,"Segment"));s;nlabels++,s=item_next(s)) {}
  if(nlabels<=2) return NULL;
  create_hts_labels(u);
  labels=(char**)calloc(nlabels,sizeof(char*));
  for(i=0,s=relation_head(utt_relation(u,"Segment"));s;s=item_next(s),i++)
    {
      labels[i]=(char*)item_feat_string(s,"hts_label");
    }
  HTS_Engine_load_label_from_string_list(&ru_engine,labels,nlabels);
  free(labels);
  if(dur_stretch>=0.5)
    {
      dur_mean=(double*)calloc(ru_engine.ms.nstate,sizeof(double));
      dur_vari=(double*)calloc(ru_engine.ms.nstate,sizeof(double));
      ru_engine.label.head->start=0;
      for(s=relation_head(utt_relation(u,"Segment")),lstring=ru_engine.label.head;s;s=item_next(s),lstring=lstring->next)
        {
          HTS_ModelSet_get_duration(&ru_engine.ms,lstring->name,dur_mean,dur_vari,ru_engine.global.duration_iw);
          local_dur_stretch=dur_stretch;
          if(cst_streq(item_name(s),"pau"))
            t=path_to_item(s,"p.R:Transcription.parent.R:Token.parent");
          else
            t=path_to_item(s,"R:Transcription.parent.R:Token.parent");
          if(t&&item_feat_present(t,"duration_stretch"))
            local_dur_stretch=item_feat_float(t,"duration_stretch");
          if(local_dur_stretch<=0)
            local_dur_stretch=1.0;
          local_dur=0;
          for(i=0;i<ru_engine.ms.nstate;i++)
            {
              local_dur+=dur_mean[i];
            }
          lstring->end=lstring->start+local_dur*local_dur_stretch;
          if(lstring->next)
            lstring->next->start=lstring->end;
        }
      free(dur_vari);
      free(dur_mean);
      HTS_Label_set_frame_specified_flag(&ru_engine.label,TRUE);
    }
  HTS_Engine_create_sstream(&ru_engine);
  for(s=relation_head(utt_relation(u,"Segment")),i=0;s;s=item_next(s),i++)
    {
      nsamples=0;
      local_f0_shift=f0_shift;
      t=path_to_item(s,"R:Transcription.parent.R:Token.parent");
      if(t&&item_feat_present(t,"f0_shift"))
        local_f0_shift=item_feat_float(t,"f0_shift");
      if(local_f0_shift<0.1)
        local_f0_shift=0.1;
      if(local_f0_shift>1.9)
        local_f0_shift=1.9;
      for(j=0;j<ru_engine.sss.nstate;j++)
        {
          nsamples+=HTS_SStreamSet_get_duration(&ru_engine.sss,i*ru_engine.sss.nstate+j)*ru_engine.global.fperiod;
          hts_local_f0=exp(HTS_SStreamSet_get_mean(&ru_engine.sss,1,i*ru_engine.sss.nstate+j,0));
          hts_local_f0*=local_f0_shift;
          if(hts_local_f0<10.0)
            hts_local_f0=10.0;
          HTS_SStreamSet_set_mean(&ru_engine.sss,1,i*ru_engine.sss.nstate+j,0,log(hts_local_f0));
        }
        item_set_int(s,"nsamples",nsamples);
    }
  item_set_int(relation_head(utt_relation(u,"Segment")),"max_nsamples",0);
  item_set_int(relation_tail(utt_relation(u,"Segment")),
               "max_nsamples",
               (int)(ru_engine.global.sampling_rate*final_pause_max_len));
  HTS_Engine_create_pstream(&ru_engine);
  ci.user_callback=user_callback;
  ci.user_buff_size=3600;
  ci.buff=(short*)calloc(ci.user_buff_size,sizeof(short));
  ci.max_buff_size=ci.user_buff_size;
  ci.cur_buff_size=0;
  if(dur_stretch>=0.5)
    ci.stream=NULL;
  else
    {
      ci.stream=sonicCreateStream(ru_engine.global.sampling_rate,1);
      if(ci.stream!=NULL)
        {
        sonicSetSpeed(ci.stream,1.0/dur_stretch);
        }
    }
  ci.cur_seg=relation_head(utt_relation(u,"Segment"));;
  ci.cur_seg_nsamples=0;
  ci.result=1;
  HTS_Engine_set_user_data(&ru_engine,&ci);
  HTS_Engine_create_gstream(&ru_engine);
  if(ci.result)
    {
      if(ci.stream==NULL)
        {
          if(ci.cur_buff_size!=0)
            ci.result=ci.user_callback(ci.buff,ci.cur_buff_size,NULL,0);
        }
      else
        {
          sonicFlushStream(ci.stream);
          nsamples=sonicSamplesAvailable(ci.stream);
          if(nsamples>0)
            {
              if(nsamples>ci.max_buff_size)
                ci.buff=realloc(ci.buff,nsamples*sizeof(short));
              sonicReadShortFromStream(ci.stream,ci.buff,nsamples);
              ci.result=ci.user_callback(ci.buff,nsamples,NULL,0);
            }
        }
    }
  HTS_Engine_set_user_data(&ru_engine,NULL);
  HTS_Engine_refresh(&ru_engine);
  feat_set_int(u->features,"last_user_callback_result",ci.result);
  if(ci.stream!=NULL)
    sonicDestroyStream(ci.stream);
  free(ci.buff);
  return u;
}

int RHVoice_speak(RHVoice_message msg)
{
  int last_user_callback_result=1;
  if(msg==NULL) return 0;
  if(voice==NULL) return 0;
  cst_utterance *u;
  for(u=next_utt_from_message(msg);u;u=next_utt_from_message(msg))
    {
      utt_init(u,voice);
      utt_synth_tokens(u);
      last_user_callback_result=get_param_int(u->features,"last_user_callback_result",1);
      delete_utterance(u);
      if(!last_user_callback_result) break;
    }
  return 1;
}


