/* Copyright (C) 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <unistr.h>
#include "lib.h"
#include "mutex.h"
#include "io_utils.h"

#ifdef WIN32
static const char *cfg_file_name="RHVoice.ini";
#else
static const char *cfg_file_name="RHVoice.conf";
#endif

static float min_rate=0.25;
static float default_rate=1.0;
static float max_rate=4.0;
static float current_rate=1.0;
float min_sonic_rate=2.0;
static float min_pitch=0.5;
static float default_pitch=1.0;
static float max_pitch=2.0;
static float current_pitch=1.0;
static float default_volume=1.0;
const static float max_volume=2.0;
static float current_volume=1.0;

MUTEX settings_mutex;

static int current_variant=variant_pseudo_english;
static int current_voice=1;

static RHVoice_punctuation_mode punctuation_mode=RHVoice_punctuation_none;
static uint32_t *punctuation_list=NULL;
static const uint32_t default_punctuation_list[]={'+','=','<','>','~','@','#','$','%','^','&','*','/','\\','|','\0'};

ucs4_t stress_marker='\0';

int apply_high_pass_filter=0;

int use_libsonic_for_pitch=0;
int libsonic_hq=1;

float check_pitch_range(float value)
{
  return ((value<min_pitch)?min_pitch:((value>max_pitch)?max_pitch:value));
}

float check_rate_range(float value)
{
  return ((value<min_rate)?min_rate:((value>max_rate)?max_rate:value));
}

float check_volume_range(float value)
{
  return ((value<0)?0:((value>max_volume)?max_volume:value));
}

void RHVoice_set_rate(float rate)
{
  float val=check_rate_range(rate);
  LOCK_MUTEX(&settings_mutex);
  current_rate=val;
  UNLOCK_MUTEX(&settings_mutex);
}

float RHVoice_get_rate()
{
  float val=default_rate;
  LOCK_MUTEX(&settings_mutex);
  val=current_rate;
  UNLOCK_MUTEX(&settings_mutex);
  return val;
}

void RHVoice_set_pitch(float pitch)
{
  float val=check_pitch_range(pitch);
  LOCK_MUTEX(&settings_mutex);
  current_pitch=val;
  UNLOCK_MUTEX(&settings_mutex);
}

float RHVoice_get_pitch()
{
  float val=default_pitch;
  LOCK_MUTEX(&settings_mutex);
  val=current_pitch;
  UNLOCK_MUTEX(&settings_mutex);
  return val;
}

void RHVoice_set_volume(float volume)
{
  float val=check_volume_range(volume);
  LOCK_MUTEX(&settings_mutex);
  current_volume=val;
  UNLOCK_MUTEX(&settings_mutex);
}

float RHVoice_get_volume()
{
  float val=default_volume;
  LOCK_MUTEX(&settings_mutex);
  val=current_volume;
  UNLOCK_MUTEX(&settings_mutex);
  return val;
}

void RHVoice_set_variant(int variant)
{
  if((variant>0)&&(variant<=RHVoice_get_variant_count()))
    {
      LOCK_MUTEX(&settings_mutex);
      current_variant=variant;
      UNLOCK_MUTEX(&settings_mutex);
    }
}

int RHVoice_get_variant()
{
  int v=1;
  LOCK_MUTEX(&settings_mutex);
  v=current_variant;
  UNLOCK_MUTEX(&settings_mutex);
  return v;
}

int RHVoice_get_variant_count()
{
  int n=get_user_dicts_count();
  if(n>0)
    return (builtin_variant_count+n-1);
  else
    return builtin_variant_count;
}

const char *RHVoice_get_variant_name(int variant)
{
  if(variant<1) return NULL;
  switch(variant)
    {
    case variant_pseudo_english:
      return "Pseudo-English";
      break;
    case variant_russian:
      return "Russian";
      break;
    default:
      return (const char*)user_dict_get_name(variant-builtin_variant_count);
      break;
    }
}

int RHVoice_find_variant(const char *name)
{
  if(strcmp(name,"Pseudo-English")==0)
    return variant_pseudo_english;
  else if(strcmp(name,"Russian")==0)
    return variant_russian;
  else
    {
      int n=get_user_dicts_count();
      int i;
      for(i=1;i<n;i++)
        {
          if(strcmp(name,(const char*)user_dict_get_name(i))==0)
            return (builtin_variant_count+i);
        }
      return 0;
    }
}

float RHVoice_get_min_rate()
{
  return min_rate;
}

float RHVoice_get_default_rate()
{
  return default_rate;
}

float RHVoice_get_max_rate()
{
  return max_rate;
}

float RHVoice_get_min_pitch()
{
  return min_pitch;
}

float RHVoice_get_default_pitch()
{
  return default_pitch;
}

float RHVoice_get_max_pitch()
{
  return max_pitch;
}

float RHVoice_get_default_volume()
{
  return default_volume;
}

float RHVoice_get_max_volume()
{
  return max_volume;
}

static int setting_callback(const uint8_t *section,const uint8_t *key,const uint8_t *value,void *user_data)
{
  int res=1;
  const char *key1=(const char*)key;
  const char *value1=(const char*)value;
  float fvalue;
  if((section==NULL))
    {
      if(value!=NULL)
        {
          if(strcmp(key1,"min_pitch")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=2)) min_pitch=fvalue;
            }
          else if(strcmp(key1,"default_pitch")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=2)) default_pitch=fvalue;
            }
          else if(strcmp(key1,"max_pitch")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=2)) max_pitch=fvalue;
            }
          else if(strcmp(key1,"min_rate")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=6)) min_rate=fvalue;
            }
          else if(strcmp(key1,"default_rate")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=6)) default_rate=fvalue;
            }
          else if(strcmp(key1,"max_rate")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=6)) max_rate=fvalue;
            }
          else if(strcmp(key1,"min_sonic_rate")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if(fvalue>0) min_sonic_rate=fvalue;
            }
          else if(strcmp(key1,"default_volume")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>0)&&(fvalue<=2)) default_volume=fvalue;
            }
          else if(strcmp(key1,"punctuation_mode")==0)
            {
              if(strcmp(value1,"none")==0)
                punctuation_mode=RHVoice_punctuation_none;
              else if(strcmp(value1,"some")==0)
                punctuation_mode=RHVoice_punctuation_some;
              else if(strcmp(value1,"all")==0)
                punctuation_mode=RHVoice_punctuation_all;
            }
          else if(strcmp(key1,"punctuation_list")==0)
            {
              size_t n=0;
              if(punctuation_list!=NULL) free(punctuation_list);
              punctuation_list=u8_to_u32(value,u8_strlen(value)+1,NULL,&n);
            }
          else if(strcmp(key1,"stress_marker")==0)
            u8_next(&stress_marker,value);
          else if(strcmp(key1,"apply_high_pass_filter")==0)
            {
              if(strcmp(value1,"yes")==0)
                apply_high_pass_filter=1;
              else if(strcmp(value1,"no")==0)
                apply_high_pass_filter=0;
            }
          else if(strcmp(key1,"use_libsonic_for_pitch")==0)
            {
              if(strcmp(value1,"yes")==0)
                use_libsonic_for_pitch=1;
              else if(strcmp(value1,"no")==0)
                use_libsonic_for_pitch=0;
            }
          else if(strcmp(key1,"libsonic_hq")==0)
            {
              if(strcmp(value1,"yes")==0)
                libsonic_hq=1;
              else if(strcmp(value1,"no")==0)
                libsonic_hq=0;
            }
        }
    }
  return res;
}

void load_settings(const char *path)
{
  char *subpath=NULL;
  INIT_MUTEX(&settings_mutex);
  if(path==NULL) return;
  subpath=path_append(path,cfg_file_name);
  if(subpath!=NULL)
    {
      parse_config(subpath,setting_callback,NULL);
      free(subpath);
      if((min_rate>max_rate)||(default_rate<min_rate)||(default_rate>max_rate))
        {
          min_rate=0.25;
          max_rate=4.0;
          default_rate=1.0;
        }
      else
        current_rate=default_rate;
      if((min_pitch>max_pitch)||(default_pitch<min_pitch)||(default_pitch>max_pitch))
        {
          min_pitch=0.5;
          max_pitch=2.0;
          default_pitch=1.0;
        }
      else
        current_pitch=default_pitch;
      if(default_volume>max_volume)
        default_volume=1.0;
      else
        current_volume=default_volume;
    }
  load_user_dicts(path);
}

void free_settings()
{
  DESTROY_MUTEX(&settings_mutex);
  free_user_dicts();
  min_rate=0.25;
  default_rate=1.0;
  max_rate=4.0;
  current_rate=1.0;
  min_sonic_rate=2.0;
  min_pitch=0.5;
  default_pitch=1.0;
  max_pitch=2.0;
  current_pitch=1.0;
  default_volume=1.0;
  current_volume=1.0;
  current_variant=variant_pseudo_english;
  current_voice=0;
  punctuation_mode=RHVoice_punctuation_none;
  if(punctuation_list!=NULL)
    {
      free(punctuation_list);
      punctuation_list=NULL;
    }
  stress_marker='\0';
  apply_high_pass_filter=0;
  use_libsonic_for_pitch=0;
  libsonic_hq=1;
}

void RHVoice_set_voice(int voice)
{
  if((voice>0)&&(voice<=RHVoice_get_voice_count()))
    {
      LOCK_MUTEX(&settings_mutex);
      current_voice=voice;
      UNLOCK_MUTEX(&settings_mutex);
    }
}

int RHVoice_get_voice()
{
  int v=1;
  LOCK_MUTEX(&settings_mutex);
  v=current_voice;
  UNLOCK_MUTEX(&settings_mutex);
  return v;
}

void RHVoice_set_punctuation_mode(RHVoice_punctuation_mode mode)
{
  if(!((mode==RHVoice_punctuation_none)||(mode==RHVoice_punctuation_some)||(mode==RHVoice_punctuation_all))) return;
  LOCK_MUTEX(&settings_mutex);
  punctuation_mode=mode;
  UNLOCK_MUTEX(&settings_mutex);
}

RHVoice_punctuation_mode RHVoice_get_punctuation_mode()
{
  RHVoice_punctuation_mode mode=RHVoice_punctuation_none;
  LOCK_MUTEX(&settings_mutex);
  mode=punctuation_mode;
  UNLOCK_MUTEX(&settings_mutex);
  return mode;
}

void RHVoice_set_punctuation_list(const char *list)
{
  if(list==NULL)
    {
      LOCK_MUTEX(&settings_mutex);
      if(punctuation_list!=NULL)
        {
          free(punctuation_list);
          punctuation_list=NULL;
        }
      UNLOCK_MUTEX(&settings_mutex);
    }
  else
    {
      size_t n=0;
      uint32_t *l=u8_to_u32((const uint8_t*)list,u8_strlen((const uint8_t*)list)+1,NULL,&n);
      if(l==NULL) return;
      LOCK_MUTEX(&settings_mutex);
      if(punctuation_list!=NULL) free(punctuation_list);
      punctuation_list=l;
      UNLOCK_MUTEX(&settings_mutex);
    }
}

uint32_t *copy_punctuation_list()
{
  uint32_t *l=NULL;
  LOCK_MUTEX(&settings_mutex);
  l=u32_strdup((punctuation_list==NULL)?default_punctuation_list:punctuation_list);
  UNLOCK_MUTEX(&settings_mutex);
  return l;
}
