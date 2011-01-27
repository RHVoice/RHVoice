/* Copyright (C) 2009, 2010,2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
#include "russian.h"
#include "lib.h"

/* We will need these internal functions of hts_engine API */
char *HTS_calloc(const size_t num, const size_t size);
char *HTS_strdup(const char *string);

static const char *sep=path_sep;
static const char *lib_version=VERSION;

typedef struct _RHVoice_callback_info {
  int min_buff_size;
  RHVoice_callback user_callback;
  cst_wave *wav;
  int start;
  int total_nsamples;
  cst_item *first_seg;
  int first_seg_start;
  cst_item *cur_seg;
  int cur_seg_start;
  int result;
} RHVoice_callback_info;

static int synth_callback(const short *samples,int nsamples,void *user_data)
{
  RHVoice_callback_info *info;
  int i,size;
  float volume,s;
  cst_item *t;
  if(user_data==NULL)
    return 0;
  info=(RHVoice_callback_info*)user_data;
  if(info->wav->num_samples-info->cur_seg_start==item_feat_int(info->cur_seg,"nsamples"))
    {
      info->cur_seg_start=info->wav->num_samples;
      info->cur_seg=item_next(info->cur_seg);
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
        info->wav->samples[info->wav->num_samples+i]=samples[i];
      else
        {
          s=(float)samples[i]*volume;
          if(s<-32766)
            info->wav->samples[info->wav->num_samples+i]=-32766;
          else
            if(s>32766)
              info->wav->samples[info->wav->num_samples+i]=32766;
            else
              info->wav->samples[info->wav->num_samples+i]=(short)(s+0.5);
        }
    }
  info->wav->num_samples+=nsamples;
  size=info->wav->num_samples-info->start;
  if(info->user_callback&&(size>=info->min_buff_size))
    {
      info->result=info->user_callback(&info->wav->samples[info->start],size,info->first_seg,info->start-info->first_seg_start);
      info->start=info->wav->num_samples;
      if(info->start-info->cur_seg_start==item_feat_int(info->cur_seg,"nsamples"))
        {
          info->cur_seg_start=info->start;
          info->cur_seg=item_next(info->cur_seg);
        }
      info->first_seg=info->cur_seg;
      info->first_seg_start=info->cur_seg_start;
    }
  return info->result;
}

static cst_utterance *hts_synth(cst_utterance *utt);
static cst_utterance *create_hts_labels(cst_utterance *u);

cst_voice *RHVoice_create_voice(const char *voxdir,RHVoice_callback callback)
{
  const cst_val *engine_val;
  HTS_Engine *engine=NULL;
  char *buff=NULL;
  int size=0;
  const int n=5;
  int i,j;
  char *fn[n];
  FILE *fp[n];
  int sampling_rate=16000;
  int fperiod=80;
  float alpha=0.42;
  float stage=0.0;
  float beta=0.4;
  float uv_threshold=0.5;
#ifdef USE_GV
  float gv_weight_lf0=0.7;
  float gv_weight_mcp=1.0;
#endif
  HTS_Boolean use_log_gain=TRUE;
  cst_voice *vox;
  if((voxdir==NULL)||(callback==NULL))
    return NULL;
  vox = new_voice();
  if(vox==NULL)
    return NULL;
  size=strlen(voxdir)+20;
  buff=calloc(n,size);
  if(buff==NULL)
    {
      delete_voice(vox);
      return NULL;
    }
  for(i=0;i<n;i++)
    {
      fn[i]=buff+i*size;
    }
  engine=malloc(sizeof(HTS_Engine));
  if(engine==NULL)
    {
      free(buff);
      delete_voice(vox);
      return NULL;
    }
  HTS_Engine_initialize(engine,2);
  HTS_Engine_set_SynthCallback(engine,synth_callback);
  vox->name = "Russian HTS voice";
  russian_init(vox);
  feat_set_string(vox->features,"name","Russian HTS voice");
  feat_set_string(vox->features,"no_segment_duration_model","1");
  feat_set_string(vox->features,"no_f0_target_model","1");
  feat_set(vox->features,"wave_synth_func",uttfunc_val(&hts_synth));
  engine_val=userdata_val(engine);
  feat_set(vox->features,"engine",engine_val);
  feat_set_int(vox->features,"sample_rate",sampling_rate);
  feat_set(vox->features,"audio_callback",uttfunc_val((cst_uttfunc)callback));
  sprintf(fn[0],"%s%sdur.pdf",voxdir,sep);
  sprintf(fn[1],"%s%stree-dur.inf",voxdir,sep);
  fp[1]=NULL;
  fp[0]=fopen(fn[0],"rb");
  if(fp[0])
    {
      fp[1]=fopen(fn[1],"r");
      if(fp[1]==NULL)
        fclose(fp[0]);
    }
  if(fp[1]==NULL)
    {
      free(buff);
      RHVoice_delete_voice(vox);
      return NULL;
    }
  HTS_Engine_load_duration_from_fp(engine,&fp[0],&fp[1],1);
  fclose(fp[1]);
  fclose(fp[0]);
  fp[4]=NULL;
  sprintf(fn[0],"%s%smgc.pdf",voxdir,sep);
  sprintf(fn[1],"%s%stree-mgc.inf",voxdir,sep);
  for(i=1;i<=3;i++)
    {
      sprintf(fn[i+1],"%s%smgc.win%d",voxdir,sep,i);
    }
  fp[0]=fopen(fn[0],"rb");
  if(fp[0])
    {
      fp[1]=fopen(fn[1],"r");
      if(fp[1]==NULL)
        fclose(fp[0]);
      else
        {
          for(i=2;i<=4;i++)
            {
              fp[i]=fopen(fn[i],"r");
              if(fp[i]==NULL)
                {
                  for(j=0;j<i;j++)
                    {
                      fclose(fp[j]);
                    }
                  break;
                }
            }
        }
    }
  if(fp[4]==NULL)
    {
      free(buff);
      RHVoice_delete_voice(vox);
      return NULL;
    }
  HTS_Engine_load_parameter_from_fp(engine,&fp[0],&fp[1],&fp[2],0,FALSE,3,1);
  for(i=0;i<=4;i++)
    {
      fclose(fp[i]);
    }
  fp[4]=NULL;
  sprintf(fn[0],"%s%slf0.pdf",voxdir,sep);
  sprintf(fn[1],"%s%stree-lf0.inf",voxdir,sep);
  for(i=1;i<=3;i++)
    {
      sprintf(fn[i+1],"%s%slf0.win%d",voxdir,sep,i);
    }
  fp[0]=fopen(fn[0],"rb");
  if(fp[0])
    {
      fp[1]=fopen(fn[1],"r");
      if(fp[1]==NULL)
        fclose(fp[0]);
      else
        {
          for(i=2;i<=4;i++)
            {
              fp[i]=fopen(fn[i],"r");
              if(fp[i]==NULL)
                {
                  for(j=0;j<i;j++)
                    {
                      fclose(fp[j]);
                    }
                  break;
                }
            }
        }
    }
  if(fp[4]==NULL)
    {
      free(buff);
      RHVoice_delete_voice(vox);
      return NULL;
    }
  HTS_Engine_load_parameter_from_fp(engine,&fp[0],&fp[1],&fp[2],1,TRUE,3,1);
#ifdef USE_GV
  beta=0;
  fp[1]=NULL;
  sprintf(fn[0],"%s%sgv-mgc.pdf",voxdir,sep);
  sprintf(fn[1],"%s%stree-gv-mgc.inf",voxdir,sep);
  fp[0]=fopen(fn[0],"rb");
  if(fp[0])
    {
      fp[1]=fopen(fn[1],"r");
      if(fp[1]==NULL)
        fclose(fp[0]);
    }
  if(fp[1]==NULL)
    {
      free(buff);
      RHVoice_delete_voice(vox);
      return NULL;
    }
  HTS_Engine_load_gv_from_fp(engine,&fp[0],&fp[1],0,1);
  fclose(fp[1]);
  fclose(fp[0]);
  fp[1]=NULL;
  sprintf(fn[0],"%s%sgv-lf0.pdf",voxdir,sep);
  sprintf(fn[1],"%s%stree-gv-lf0.inf",voxdir,sep);
  fp[0]=fopen(fn[0],"rb");
  if(fp[0])
    {
      fp[1]=fopen(fn[1],"r");
      if(fp[1]==NULL)
        fclose(fp[0]);
    }
  if(fp[1]==NULL)
    {
      free(buff);
      RHVoice_delete_voice(vox);
      return NULL;
    }
  HTS_Engine_load_gv_from_fp(engine,&fp[0],&fp[1],1,1);
  fclose(fp[1]);
  fclose(fp[0]);
  sprintf(fn[0],"%s%sgv-switch.inf",voxdir,sep);
  fp[0]=fopen(fn[0],"r");
  if(fp[0]==NULL)
    {
      free(buff);
      RHVoice_delete_voice(vox);
      return NULL;
    }
  HTS_Engine_load_gv_switch_from_fp(engine,fp[0]);
  fclose(fp[0]);
#endif
  HTS_Engine_set_sampling_rate(engine,sampling_rate);
  HTS_Engine_set_fperiod(engine,fperiod);
  HTS_Engine_set_alpha(engine,alpha);
  HTS_Engine_set_gamma(engine,stage);
  HTS_Engine_set_log_gain(engine,use_log_gain);
  HTS_Engine_set_beta(engine,beta);
  HTS_Engine_set_msd_threshold(engine,1,uv_threshold);
#ifdef USE_GV
  HTS_Engine_set_gv_weight(engine,0,gv_weight_mcp);
  HTS_Engine_set_gv_weight(engine,1,gv_weight_lf0);
#endif
  HTS_Engine_set_duration_interpolation_weight(engine,0,1.0);
  HTS_Engine_set_parameter_interpolation_weight(engine,0,0,1.0);
  HTS_Engine_set_parameter_interpolation_weight(engine,1,0,1.0);
#ifdef USE_GV
  HTS_Engine_set_gv_interpolation_weight(engine,0,0,1.0);
  HTS_Engine_set_gv_interpolation_weight(engine,1,0,1.0);
#endif
  free(buff);
  return vox;
}

void RHVoice_delete_voice(cst_voice *vox)
{
  HTS_Engine *engine;
  ru_user_dict *dict;
  if(vox==NULL)
    return;
  engine=(HTS_Engine*)val_userdata(feat_val(vox->features,"engine"));
  if(engine!=NULL)
    {
      HTS_Engine_clear(engine);
      free(engine);
    }
  if(feat_present(vox->features,"user_dict"))
    {
      dict=(ru_user_dict*)val_userdata(feat_val(vox->features,"user_dict"));
      ru_user_dict_free(dict);
    }
  delete_voice(vox);
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
  HTS_Engine *engine;
  HTS_LabelString *lstring=NULL;
  double *dur_mean,*dur_vari;
  double end=0.0;
  int i,j,nsamples_in_seg,orig_total_nframes;
  cst_item *s,*s1,*s2,*t;
  float f0_shift,dur_stretch,local_f0_shift,local_dur_stretch,hts_local_f0;
  const char *last_punc=NULL;
  float final_pause_len=0.0;
  orig_total_nframes=0;
  RHVoice_callback_info callback_info;
  if(!feat_present(u->features,"audio_callback"))
    return NULL;
  callback_info.user_callback=(RHVoice_callback)val_uttfunc(feat_val(u->features,"audio_callback"));
  callback_info.min_buff_size=(int)(get_param_int(u->features,"audio_buff_len",150)*(get_param_int(u->features,"sample_rate",16000)/1000.0));
  engine=(HTS_Engine*)val_userdata(feat_val(u->features,"engine"));
  dur_stretch=get_param_float(u->features,"duration_stretch",1.0);
  f0_shift=get_param_float(u->features,"f0_shift",1.0);
  s=relation_head(utt_relation(u,"Segment"));
  if(!s)
    return NULL;
  s1=item_next(s);
  if(!s1)
    return NULL;
  s2=relation_tail(utt_relation(u,"Segment"));
  if(s1==s2)
    return NULL;
  create_hts_labels(u);
  last_punc=item_feat_string(relation_tail(utt_relation(u,"Token")),"punc");
  if(last_punc)
    {
      if(strchr(last_punc,'.')||
         strchr(last_punc,'?')||
         strchr(last_punc,'!'))
        final_pause_len=0.25;
      else
        if(strchr(last_punc,':')||
           strchr(last_punc,';'))
          final_pause_len=0.175;
        else
          if(strchr(last_punc,',')||
             strchr(last_punc,'-'))
            final_pause_len=0.01;
    }
  dur_mean=(double*)calloc(engine->ms.nstate,sizeof(double));
  dur_vari=(double*)calloc(engine->ms.nstate,sizeof(double));
  for(s=s1;s!=s2;s=item_next(s),engine->label.size++)
    {
      if(lstring)
        {
          lstring->next=(HTS_LabelString*)HTS_calloc(1,sizeof(HTS_LabelString));
          lstring=lstring->next;
        }
      else
        {
          lstring=(HTS_LabelString*)HTS_calloc(1,sizeof(HTS_LabelString));
          engine->label.head=lstring;
        }
      lstring->name=HTS_strdup(item_feat_string(s,"hts_label"));
      lstring->start=end;
      HTS_ModelSet_get_duration(&engine->ms,lstring->name,dur_mean,dur_vari,engine->global.duration_iw);
      local_dur_stretch=dur_stretch;
      t=path_to_item(s,"R:Transcription.parent.R:Token.parent");
      if(t&&item_feat_present(t,"duration_stretch"))
        local_dur_stretch=item_feat_float(t,"duration_stretch");
      if(local_dur_stretch<=0)
        local_dur_stretch=1.0;
      for(i=0;i<engine->ms.nstate;i++)
        {
          end+=dur_mean[i]*local_dur_stretch;
          orig_total_nframes+=dur_mean[i];
        }
      lstring->end=end;
      lstring->next=NULL;
    }
  free(dur_vari);
  free(dur_mean);
  HTS_Label_set_frame_specified_flag(&engine->label,TRUE);
  HTS_Engine_create_sstream(engine);
  for(s=s1,i=0;s!=s2;s=item_next(s),i++)
    {
      nsamples_in_seg=0;
      local_f0_shift=f0_shift;
      t=path_to_item(s,"R:Transcription.parent.R:Token.parent");
      if(t&&item_feat_present(t,"f0_shift"))
        local_f0_shift=item_feat_float(t,"f0_shift");
      if(local_f0_shift<0.1)
        local_f0_shift=0.1;
      if(local_f0_shift>1.9)
        local_f0_shift=1.9;
      for(j=0;j<engine->sss.nstate;j++)
        {
          nsamples_in_seg+=HTS_SStreamSet_get_duration(&engine->sss,i*engine->sss.nstate+j)*engine->global.fperiod;
          hts_local_f0=exp(HTS_SStreamSet_get_mean(&engine->sss,1,i*engine->sss.nstate+j,0));
          hts_local_f0*=local_f0_shift;
          if(hts_local_f0<10.0)
            hts_local_f0=10.0;
          HTS_SStreamSet_set_mean(&engine->sss,1,i*engine->sss.nstate+j,0,log(hts_local_f0));
        }
        item_set_int(s,"nsamples",nsamples_in_seg);
    }
  item_set_int(item_prev(s1),"nsamples",0);
  item_set_int(s2,"nsamples",
               (int)(engine->global.sampling_rate*final_pause_len*(HTS_SStreamSet_get_total_frame(&engine->sss)/orig_total_nframes)));
  HTS_Engine_create_pstream(engine);
  callback_info.wav=new_wave();
  callback_info.wav->sample_rate=engine->global.sampling_rate;
  callback_info.total_nsamples=HTS_SStreamSet_get_total_frame(&engine->sss)*engine->global.fperiod+item_feat_int(s2,"nsamples");
  cst_wave_resize(callback_info.wav,callback_info.total_nsamples,1);
  callback_info.wav->num_samples=0;
  memset(callback_info.wav->samples,0,callback_info.total_nsamples*sizeof(short));
  callback_info.first_seg=s1;
  callback_info.cur_seg=s1;
  callback_info.first_seg_start=0;
  callback_info.cur_seg_start=0;
  callback_info.start=0;
  callback_info.result=1;
  HTS_Engine_set_user_data(engine,&callback_info);
  HTS_Engine_create_gstream(engine);
  HTS_Engine_set_user_data(engine,NULL);
  if(callback_info.result&&(callback_info.start<callback_info.total_nsamples))
    {
      callback_info.result=callback_info.user_callback(&callback_info.wav->samples[callback_info.start],
                                                       callback_info.total_nsamples-callback_info.start,
                                                       callback_info.first_seg,
                                                       callback_info.start-callback_info.first_seg_start);
      callback_info.wav->num_samples=callback_info.total_nsamples;
    }
  utt_set_wave(u,callback_info.wav);
  HTS_Engine_refresh(engine);
  feat_set_int(u->features,"last_audio_callback_result",callback_info.result);
  return u;
}

static cst_utterance *create_hts_labels(cst_utterance *u)
{
  const int size=1024;
  char label[size];
  char *ptr=label;
  cst_item *s,*ps,*pps,*ns,*nns,*ss,*tmp,*ssyl,*sw,*syl,*w,*phr;
  int total_phrases=0;
  int total_words=0;
  int total_syls=0;
  int is_pau=0;
  int count=0;
  int dist=0;
  int seg_pos_in_syl=0;
  int num_segs_in_syl=0;
  int syl_pos_in_word=0;
  int num_syls_in_word=0;
  int syl_pos_in_phrase=0;
  int num_syls_in_phrase=0;
  int num_words_in_phrase=0;
  int word_pos_in_phrase=0;
  int phrase_pos_in_utt=0;
  int is_content=0;
  int is_stressed=0;

  for(phr=relation_head(utt_relation(u,"Phrase"));phr;phr=item_next(phr),total_phrases++)
    {
      num_syls_in_phrase=0;
      for(num_words_in_phrase=0,w=item_daughter(phr);w;w=item_next(w),num_words_in_phrase++,total_words++)
        {
          sw=item_as(w,"SylStructure");
          for(num_syls_in_word=0,ssyl=item_daughter(sw);
              ssyl;
              ssyl=item_next(ssyl),num_syls_in_word++,num_syls_in_phrase++,total_syls++)
            {
              for(num_segs_in_syl=0,ss=item_daughter(ssyl);ss;ss=item_next(ss),num_segs_in_syl++)
                {
                  item_set_int(ss,"pos_in_syl",num_segs_in_syl);
                }
              item_set_int(ssyl,"num_phones",num_segs_in_syl);
              item_set_int(ssyl,"pos_in_word",num_syls_in_word);
              item_set_int(ssyl,"pos_in_phrase",num_syls_in_phrase);
              item_set_int(ssyl,"pos_in_utt",total_syls);
            }
          item_set_int(w,"num_syls",num_syls_in_word);
          item_set_int(w,"pos_in_phrase",num_words_in_phrase);
          item_set_int(w,"pos_in_utt",total_words);
        }
      item_set_int(phr,"num_syls",num_syls_in_phrase);
      item_set_int(phr,"pos_in_utt",total_phrases);
      item_set_int(phr,"num_words",num_words_in_phrase);
      for(count=0,dist=0,w=item_daughter(phr);w;w=item_next(w))
        {
          item_set_int(w,"num_content_words_before",count);
          item_set_int(w,"dist_to_prev_content_word",dist);
          is_content=cst_streq(ffeature_string(w,"gpos"),"content");
          if(is_content)
            {
              count++;
              dist=1;
            }
          else
            {
              dist++;
            }
        }
      for(count=0,dist=0,w=item_last_daughter(phr);w;w=item_prev(w))
        {
          item_set_int(w,"num_content_words_after",count);
          item_set_int(w,"dist_to_next_content_word",dist);
          is_content=cst_streq(ffeature_string(w,"gpos"),"content");
          if(is_content)
            {
              count++;
              dist=1;
            }
          else
            {
              dist++;
            }
        }
      tmp=path_to_item(phr,"daughtern.R:SylStructure.daughtern.R:Syllable");
      for(count=0,dist=0,syl=path_to_item(phr,"daughter.R:SylStructure.daughter.R:Syllable");
          syl;
          syl=item_next(syl))
        {
          item_set_int(syl,"num_stressed_syls_before",count);
          item_set_int(syl,"dist_to_prev_stress",dist);
          is_stressed=!cst_streq(item_feat_string(syl,"stress"),"0");
          if(is_stressed)
            {
              count++;
              dist=1;
            }
          else
            {
              dist++;
            }
          if(item_equal(syl,tmp))
            break;
        }
      tmp=path_to_item(phr,"daughter.R:SylStructure.daughter.R:Syllable");
      for(count=0,dist=0,syl=path_to_item(phr,"daughtern.R:SylStructure.daughtern.R:Syllable");
          syl;
          syl=item_prev(syl))
        {
          item_set_int(syl,"num_stressed_syls_after",count);
          item_set_int(syl,"dist_to_next_stress",dist);
          is_stressed=!cst_streq(item_feat_string(syl,"stress"),"0");
          if(is_stressed)
            {
              count++;
              dist=1;
            }
          else
            {
              dist++;
            }
          if(item_equal(syl,tmp))
            break;
        }
    }

  for(s=relation_head(utt_relation(u,"Segment"));s;s=item_next(s))
    {
      is_pau=cst_streq(item_name(s),"pau");
      if(!is_pau)
        {
          ss=item_as(s,"SylStructure");
          ssyl=item_parent(ss);
          syl=item_as(ssyl,"Syllable");
          sw=item_parent(ssyl);
          phr=item_parent(item_as(sw,"Phrase"));
        }
      else
        {
          sw=NULL;
          syl=NULL;
        }
      ns=item_next(s);
      ps=item_prev(s);
      nns=ns?item_next(ns):NULL;
      pps=ps?item_prev(ps):NULL;
      ptr+=sprintf(ptr,"%s",(pps?item_name(pps):"x"));
      ptr+=sprintf(ptr,"^%s",ps?item_name(ps):"x");
      ptr+=sprintf(ptr,"-%s",item_name(s));
      ptr+=sprintf(ptr,"+%s",ns?item_name(ns):"x");
      ptr+=sprintf(ptr,"=%s",nns?item_name(nns):"x");
      if(is_pau)
        ptr+=sprintf(ptr,"@x_x");
      else
        {
          seg_pos_in_syl=item_feat_int(s,"pos_in_syl");
          num_segs_in_syl=item_feat_int(syl,"num_phones");
          ptr+=sprintf(ptr,"@%d_%d",seg_pos_in_syl+1,num_segs_in_syl-seg_pos_in_syl);
        }
      ptr+=sprintf(ptr,"/A:%s",ffeature_string(s,is_pau?"p.R:SylStructure.parent.R:Syllable.stress":"R:SylStructure.parent.R:Syllable.p.stress"));
      ptr+=sprintf(ptr,"_x");
      tmp=path_to_item(s,is_pau?"p.R:SylStructure.parent.R:Syllable":"R:SylStructure.parent.R:Syllable.p");
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_phones"):0);
      ptr+=sprintf(ptr,"/B:%s",is_pau?"x":ffeature_string(s,"R:SylStructure.parent.R:Syllable.stress"));
      ptr+=sprintf(ptr,"-x");
      ptr+=(is_pau?sprintf(ptr,"-x"):sprintf(ptr,"-%d",num_segs_in_syl));
      if(is_pau)
        ptr+=sprintf(ptr,"@x-x");
      else
        {
          syl_pos_in_word=item_feat_int(syl,"pos_in_word");
          num_syls_in_word=item_feat_int(sw,"num_syls");
          ptr+=sprintf(ptr,"@%d-%d",syl_pos_in_word+1,num_syls_in_word-syl_pos_in_word);
        }
      if(is_pau)
        ptr+=sprintf(ptr,"&x-x");
      else
        {
          syl_pos_in_phrase=item_feat_int(syl,"pos_in_phrase");
          num_syls_in_phrase=item_feat_int(phr,"num_syls");
          ptr+=sprintf(ptr,"&%d-%d",syl_pos_in_phrase+1,num_syls_in_phrase-syl_pos_in_phrase);
        }
      if(is_pau)
        ptr+=sprintf(ptr,"#x-x");
      else
        {
          ptr+=sprintf(ptr,"#%d-%d",item_feat_int(syl,"num_stressed_syls_before")+1,item_feat_int(syl,"num_stressed_syls_after")+1);
        }
      ptr+=sprintf(ptr,"$x-x");
      if(is_pau)
        ptr+=sprintf(ptr,"!x-x");
      else
        ptr+=sprintf(ptr,"!%d-%d",item_feat_int(syl,"dist_to_prev_stress"),item_feat_int(syl,"dist_to_next_stress"));
      ptr+=sprintf(ptr,";x-x");
      if(is_pau)
        ptr+=sprintf(ptr,"|x");
      else
        {
          tmp=path_to_item(syl,"R:SylVowel.daughter");
          ptr+=sprintf(ptr,"|%s",tmp?item_name(tmp):"novowel");
        }
      ptr+=sprintf(ptr,"/C:%s",ffeature_string(s,is_pau?"n.R:SylStructure.parent.R:Syllable.stress":"R:SylStructure.parent.R:Syllable.n.stress"));
      ptr+=sprintf(ptr,"+x");
      tmp=path_to_item(s,is_pau?"n.R:SylStructure.parent.R:Syllable":"R:SylStructure.parent.R:Syllable.n");
      ptr+=sprintf(ptr,"+%d",tmp?item_feat_int(tmp,"num_phones"):0);
      ptr+=sprintf(ptr,"/D:%s",ffeature_string(s,is_pau?"p.R:SylStructure.parent.parent.R:Word.gpos":"R:SylStructure.parent.parent.R:Word.p.gpos"));
      tmp=path_to_item(s,is_pau?"p.R:SylStructure.parent.parent.R:Word":"R:SylStructure.parent.parent.R:Word.p");
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_syls"):0);
      ptr+=sprintf(ptr,"/E:%s",is_pau?"x":ffeature_string(sw,"gpos"));
      if(is_pau)
        ptr+=sprintf(ptr,"+x");
      else
        ptr+=sprintf(ptr,"+%d",num_syls_in_word);
      if(is_pau)
        ptr+=sprintf(ptr,"@x+x");
      else
        {
          word_pos_in_phrase=item_feat_int(sw,"pos_in_phrase");
          num_words_in_phrase=item_feat_int(phr,"num_words");
          ptr+=sprintf(ptr,"@%d+%d",word_pos_in_phrase+1,num_words_in_phrase-word_pos_in_phrase);
        }
      if(is_pau)
        ptr+=sprintf(ptr,"&x+x");
      else
        ptr+=sprintf(ptr,"&%d+%d",item_feat_int(sw,"num_content_words_before")+1,item_feat_int(sw,"num_content_words_after"));
      if(is_pau)
        ptr+=sprintf(ptr,"#x+x");
      else
        ptr+=sprintf(ptr,"#%d+%d",item_feat_int(sw,"dist_to_prev_content_word"),item_feat_int(sw,"dist_to_next_content_word"));
      ptr+=sprintf(ptr,"/F:%s",ffeature_string(s,is_pau?"n.R:SylStructure.parent.parent.R:Word.gpos":"R:SylStructure.parent.parent.R:Word.n.gpos"));
      tmp=path_to_item(s,is_pau?"n.R:SylStructure.parent.parent.R:Word":"R:SylStructure.parent.parent.R:Word.n");
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_syls"):0);
      tmp=is_pau?path_to_item(s,"p.R:SylStructure.parent.parent.R:Phrase.parent"):item_prev(phr);
      ptr+=sprintf(ptr,"/G:%d",tmp?item_feat_int(tmp,"num_syls"):0);
      ptr+=sprintf(ptr,"_%d",tmp?item_feat_int(tmp,"num_words"):0);
      if(is_pau)
        ptr+=sprintf(ptr,"/H:x=x@x=x");
      else
        {
          phrase_pos_in_utt=item_feat_int(phr,"pos_in_utt");
          ptr+=sprintf(ptr,"/H:%d=%d@%d=%d",item_feat_int(phr,"num_syls"),item_feat_int(phr,"num_words"),phrase_pos_in_utt+1,total_phrases-phrase_pos_in_utt);
        }
      ptr+=sprintf(ptr,"|x");
      tmp=is_pau?path_to_item(s,"n.R:SylStructure.parent.parent.R:Phrase.parent"):item_next(phr);
      ptr+=sprintf(ptr,"/I:%d",tmp?item_feat_int(tmp,"num_syls"):0);
      ptr+=sprintf(ptr,"=%d",tmp?item_feat_int(tmp,"num_words"):0);
      ptr+=sprintf(ptr,"/J:%d+%d-%d",total_syls,total_words,total_phrases);
      ptr=label;
      item_set_string(s,"hts_label",label);
    }
  return u;
}

void RHVoice_synth_text(const char *text,cst_voice *voice,const cst_features *params)
{
  cst_tokenstream *ts=ts_open_string(text,
                                     get_param_string(voice->features,"text_whitespace",NULL),
                                     get_param_string(voice->features,"text_singlecharsymbols",NULL),
                                     get_param_string(voice->features,"text_prepunctuation",NULL),
                                     get_param_string(voice->features,"text_postpunctuation",NULL));
  cst_utterance *utt=new_utterance();
  cst_relation *rel=utt_relation_create(utt,"Token");
  const char *token;
  cst_item *t;
  cst_breakfunc breakfunc=feat_present(voice->features,"utt_break")?val_breakfunc(feat_val(voice->features,"utt_break")):default_utt_break;
  while(TRUE)
    {
      if(ts_eof(ts))
        {
          if(relation_head(rel))
            {
              utt_init(utt,voice);
              if(params)
                feat_copy_into(params,utt->features);
              utt_synth_tokens(utt);
            }
          break;
        }
      token=ts_get(ts);
      if(relation_head(rel)&&breakfunc(ts,token,rel))
        {
          utt_init(utt,voice);
          if(params)
            feat_copy_into(params,utt->features);
          utt_synth_tokens(utt);
          if(get_param_int(utt->features,"last_audio_callback_result",1)==0)
            break;
          delete_utterance(utt);
          utt=new_utterance();
          rel=utt_relation_create(utt,"Token");
        }
      t=relation_append(rel,NULL);
      item_set_string(t,"name",token);
      item_set_string(t,"whitespace",ts->whitespace);
      item_set_string(t,"prepunctuation",ts->prepunctuation);
      item_set_string(t,"punc",ts->postpunctuation);
    }
  if(utt)
    delete_utterance(utt);
  ts_close(ts);
}
