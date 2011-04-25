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

#include "lib.h"
#include "mutex.h"

static float min_rate=0.25;
static float default_rate=1.0;
static float max_rate=4.0;
static float current_rate=1.0;
static float min_pitch=0.5;
static float default_pitch=1.0;
static float max_pitch=2.0;
static float current_pitch=1.0;
static float default_volume=1.0;
static float max_volume=2.0;
static float current_volume=1.0;

MUTEX settings_mutex;

static RHVoice_variant current_variant=RHVoice_variant_pseudo_english;

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

void RHVoice_set_variant(RHVoice_variant variant)
{
  if((variant>0)&&(variant<3))
    {
      LOCK_MUTEX(&settings_mutex);
      current_variant=variant;
      UNLOCK_MUTEX(&settings_mutex);
    }
}

RHVoice_variant RHVoice_get_variant()
{
  RHVoice_variant v=RHVoice_variant_pseudo_english;
  LOCK_MUTEX(&settings_mutex);
  v=current_variant;
  UNLOCK_MUTEX(&settings_mutex);
  return v;
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

user_dict global_user_dict=NULL;

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
          if(strcmp(key1,"min_rate")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=6)) min_rate=fvalue;
            }
          if(strcmp(key1,"default_rate")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=6)) default_rate=fvalue;
            }
          else if(strcmp(key1,"max_rate")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>=0.2)&&(fvalue<=6)) max_rate=fvalue;
            }
          else if(strcmp(key1,"default_volume")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>0)&&(fvalue<=2)) default_volume=fvalue;
            }
          else if(strcmp(key1,"max_volume")==0)
            {
              fvalue=strtod_c(value1,NULL);
              if((fvalue>0)&&(fvalue<=2)) max_volume=fvalue;
            }
        }
    }
  else if(strcmp((const char*)section,"dicts")==0)
    user_dict_update(global_user_dict,key1);
  return res;
}

void load_settings(const char *path)
{
  INIT_MUTEX(&settings_mutex);
  if(path==NULL) return;
  global_user_dict=user_dict_create();
  parse_config(path,setting_callback,NULL);
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
    {
      max_volume=2.0;
      default_volume=1.0;
    }
  else
    current_volume=default_volume;
  user_dict_build(global_user_dict);
}

void free_settings()
{
  DESTROY_MUTEX(&settings_mutex);
  user_dict_free(global_user_dict);
  global_user_dict=NULL;
  min_rate=0.25;
  default_rate=1.0;
  max_rate=4.0;
  current_rate=1.0;
  min_pitch=0.5;
  default_pitch=1.0;
  max_pitch=2.0;
  current_pitch=1.0;
  default_volume=1.0;
  max_volume=2.0;
  current_volume=1.0;
  current_variant=RHVoice_variant_pseudo_english;
}
