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
#include "vector.h"
#include "io_utils.h"

static const char *lib_version=VERSION;

static cst_voice *voice=NULL;
static HTS_Engine ru_engine;
static RHVoice_callback user_callback=NULL;

vector_t(short,svector)
vector_t(RHVoice_event,eventlist)

typedef struct {
  svector samples;
  int min_buff_size;
  sonicStream stream;
  eventlist events;
  cst_item *cur_seg;
  int in_nsamples;
  int out_nsamples;
  int num_events;
  int event_index;
  int last_result;
} synth_state;

#define synth_error(s) {s->last_result=0;return 0;}

static int synth_callback(const short *samples,int nsamples,void *user_data)
{
  synth_state *state=(synth_state*)user_data;
  float volume,fs;
  short s;
  int i,size,d;
  RHVoice_event *events=NULL;
  int num_events=0;
  cst_item *t;
  state->in_nsamples+=nsamples;
  if(state->in_nsamples>item_feat_int(state->cur_seg,"end"))
    state->cur_seg=item_next(state->cur_seg);
  int end_of_audio=((!item_next(state->cur_seg))&&(state->in_nsamples==item_feat_int(state->cur_seg,"end")));
  if(!svector_reserve(state->samples,svector_size(state->samples)+nsamples)) synth_error(state);
  t=path_to_item(state->cur_seg,"R:Transcription.parent.R:Token.parent");
  if(t)
    {
      volume=item_feat_float(t,"volume");
      for(i=0;i<nsamples;i++)
        {
          fs=(float)samples[i]*volume;
          if(fs<-32766) s=-32766;
          else if(fs>32766) s=32766;
          else s=(short)(fs+0.5);
          svector_push(state->samples,&s);
        }
    }
  else svector_append(state->samples,samples,nsamples);
  if(state->stream)
    {
      if(!sonicWriteShortToStream(state->stream,svector_data(state->samples),svector_size(state->samples)))
        synth_error(state);
      svector_clear(state->samples);
      if(end_of_audio) sonicFlushStream(state->stream);
      size=sonicSamplesAvailable(state->stream);
      if((size>0)&&((size>=state->min_buff_size)||end_of_audio))
        {
          s=0;
          if(!svector_resize(state->samples,size,&s)) synth_error(state);
          sonicReadShortFromStream(state->stream,svector_data(state->samples),size);
        }
    }
  size=svector_size(state->samples);
  if((size>0)&&((size>=state->min_buff_size)||end_of_audio))
    {
      for(i=state->event_index;i<state->num_events;i++)
        {
          d=eventlist_at(state->events,i)->audio_position-state->out_nsamples;
          if(d>=size)
            {
              if(end_of_audio) d=size;
              else break;
            }
          num_events++;
          eventlist_at(state->events,i)->audio_position=d;
        }
      if(num_events>0) events=eventlist_at(state->events,state->event_index);
      state->last_result=user_callback(svector_data(state->samples),size,events,num_events);
      state->out_nsamples+=size;
      state->event_index+=num_events;
      svector_clear(state->samples);
    }
  return end_of_audio?0:(state->last_result);
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
      file_list[i]=my_fopen(full_path,hts_data_list[i].open_mode);
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
  free_user_dict();
}

const char *RHVoice_get_version()
{
  return lib_version;
}

static eventlist generate_events(cst_utterance *u)
{
  eventlist events=eventlist_alloc(0,NULL);
  RHVoice_event e;
  cst_item *t,*s;
  e.audio_position=0;
  s=item_daughter(relation_head(utt_relation(u,"Transcription")));
  if(s) e.audio_position=item_feat_int(s,"expected_start");
  e.message=(RHVoice_message)val_userdata(feat_val(u->features,"message"));
  for(t=relation_head(utt_relation(u,"Token"));t;t=item_next(t))
    {
      s=path_to_item(t,"daughter1.R:Transcription.daughter1");
      if(s) e.audio_position=item_feat_int(s,"expected_start");
      if(item_feat_present(t,"mark_name"))
        {
          e.type=RHVoice_event_mark;
          e.text_position=item_feat_int(t,"mark_position");
          e.text_length=0;
          e.id.name=item_feat_string(t,"mark_name");
          eventlist_push(events,&e);
        }
      e.text_position=item_feat_int(t,"position");
      if(!item_prev(t))
        {
          e.type=RHVoice_event_sentence_start;
          e.text_length=feat_int(u->features,"length");
          e.id.number=feat_int(u->features,"number");
          eventlist_push(events,&e);
        }
      e.type=RHVoice_event_word_start;
      e.text_length=item_feat_int(t,"length");
      e.id.number=item_feat_int(t,"number");
      eventlist_push(events,&e);
      if(s)
        {
          s=path_to_item(t,"daughtern.R:Transcription.daughtern");
          e.audio_position=item_feat_int(s,"expected_end");
        }
      e.text_position+=e.text_length;
      e.text_length=0;
      e.type=RHVoice_event_word_end;
      eventlist_push(events,&e);
      if(!item_next(t))
        {
          e.type=RHVoice_event_sentence_end;
          e.id.number=feat_int(u->features,"number");
          eventlist_push(events,&e);
        }
    }
  return events;
}

static cst_utterance *hts_synth(cst_utterance *u)
{
  HTS_LabelString *lstring=NULL;
  double *dur_mean,*dur_vari,local_dur;
  char **labels=NULL;
  int i,j,nsamples,nlabels,total_nsamples;
  cst_item *s,*t;
  float pitch,rate,f0;
  synth_state state;
  state.min_buff_size=0.15*ru_engine.global.sampling_rate;
  state.samples=svector_alloc(state.min_buff_size,NULL);
  if(state.samples==NULL) return NULL;
  rate=feat_float(u->features,"rate");
  float frames_per_sec=(float)ru_engine.global.sampling_rate/((float)ru_engine.global.fperiod);
  float final_pause_limit=0.25*frames_per_sec/((rate<=2)?rate:1);
  float factor,time;
  for(nlabels=0,s=relation_head(utt_relation(u,"Segment"));s;nlabels++,s=item_next(s)) {}
  if(nlabels==0) return NULL;
  create_hts_labels(u);
  s=relation_head(utt_relation(u,"Segment"));
  if((item_feat_float(s,"factor")==0)&&(item_feat_float(s,"time")==0))
    {
      nlabels--;
      if(nlabels==0) return NULL;
      delete_item(s);
    }
  s=relation_tail(utt_relation(u,"Segment"));
  if((item_feat_float(s,"factor")==0)&&(item_feat_float(s,"time")==0))
    {
      nlabels--;
      if(nlabels==0) return NULL;
      delete_item(s);
    }
  labels=(char**)calloc(nlabels,sizeof(char*));
  for(i=0,s=relation_head(utt_relation(u,"Segment"));s;s=item_next(s),i++)
    {
      labels[i]=(char*)item_feat_string(s,"hts_label");
    }
  HTS_Engine_load_label_from_string_list(&ru_engine,labels,nlabels);
  free(labels);
  dur_mean=(double*)calloc(ru_engine.ms.nstate,sizeof(double));
  dur_vari=(double*)calloc(ru_engine.ms.nstate,sizeof(double));
  ru_engine.label.head->start=0;
  for(s=relation_head(utt_relation(u,"Segment")),lstring=ru_engine.label.head;s;s=item_next(s),lstring=lstring->next)
    {
      HTS_ModelSet_get_duration(&ru_engine.ms,lstring->name,dur_mean,dur_vari,ru_engine.global.duration_iw);
      local_dur=0;
      for(i=0;i<ru_engine.ms.nstate;i++)
        {
          local_dur+=dur_mean[i];
        }
      if(rate<=2) local_dur/=rate;
      if(cst_streq(item_name(s),"pau"))
        {
          if((!item_next(s))&&(local_dur>final_pause_limit))
            local_dur=final_pause_limit;
          factor=item_feat_float(s,"factor");
          time=item_feat_float(s,"time");
          local_dur*=factor;
          local_dur+=((rate>2)?rate:1.0)*time*frames_per_sec;
        }
      lstring->end=lstring->start+local_dur;
      if(lstring->next)
        lstring->next->start=lstring->end;
    }
  free(dur_vari);
  free(dur_mean);
  HTS_Label_set_frame_specified_flag(&ru_engine.label,TRUE);
  HTS_Engine_create_sstream(&ru_engine);
  total_nsamples=0;
  for(s=relation_head(utt_relation(u,"Segment")),i=0;s;s=item_next(s),i++)
    {
      t=path_to_item(s,"R:Transcription.parent.R:Token.parent");
      if(t)
        {
          pitch=item_feat_float(t,"pitch");
          for(j=0;j<ru_engine.sss.nstate;j++)
            {
              f0=exp(HTS_SStreamSet_get_mean(&ru_engine.sss,1,i*ru_engine.sss.nstate+j,0))*pitch;
              if(f0<10.0) f0=10.0;
              HTS_SStreamSet_set_mean(&ru_engine.sss,1,i*ru_engine.sss.nstate+j,0,log(f0));
            }
        }
      nsamples=0;
      for(j=0;j<ru_engine.sss.nstate;j++)
        {
          nsamples+=HTS_SStreamSet_get_duration(&ru_engine.sss,i*ru_engine.sss.nstate+j)*ru_engine.global.fperiod;
        }
      item_set_int(s,"start",total_nsamples);
      item_set_int(s,"expected_start",(rate>2)?((int)((float)total_nsamples/rate)):total_nsamples);
      total_nsamples+=nsamples;
      item_set_int(s,"end",total_nsamples);
      item_set_int(s,"expected_end",(rate>2)?((int)((float)total_nsamples/rate)):total_nsamples);
    }
  HTS_Engine_create_pstream(&ru_engine);
  if(rate<=2)
    state.stream=NULL;
  else
    {
      state.stream=sonicCreateStream(ru_engine.global.sampling_rate,1);
      if(state.stream!=NULL)
        sonicSetSpeed(state.stream,rate);
      sonicSetQuality(state.stream,1);
    }
  state.events=generate_events(u);
  state.num_events=eventlist_size(state.events);
  state.event_index=0;
  state.cur_seg=relation_head(utt_relation(u,"Segment"));
  state.in_nsamples=0;
  state.out_nsamples=0;
  state.last_result=1;
  HTS_Engine_set_user_data(&ru_engine,&state);
  HTS_Engine_create_gstream(&ru_engine);
  HTS_Engine_set_user_data(&ru_engine,NULL);
  HTS_Engine_refresh(&ru_engine);
  feat_set_int(u->features,"last_user_callback_result",state.last_result);
  if(state.stream!=NULL) sonicDestroyStream(state.stream);
  if(state.events!=NULL) eventlist_free(state.events);
  svector_free(state.samples);
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


