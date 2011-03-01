/* Copyright (C) 2009, 2010  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

cst_lexicon *cmu_lex_init();

const char *ru_proc_list[]={"а","без","безо","в","во","для","до","за","и","из",
                            "изо","из-за","из-под","к","ко","меж","на","над",
                            "не","ни","о","об","обо","от","ото","перед",
                            "передо","по","под","пред","при","про","с","со","у",
                            "через","чтоб",NULL};
const char *ru_enc_list[]={"б","бы","ж","же","ли","таки",NULL};
const char *ru_h_enc_list[]={"де","ка","либо","нибудь","с","таки","тка","то",NULL};

static cst_utterance* do_nothing(cst_utterance* u)
{
  return u;
}

static cst_val *add_lts_boundary_marks(const cst_val *l)
{
    cst_val *l1;
    const cst_val *v;
    l1 = cons_val(string_val("#"),NULL);
    for (v=l;v;v=val_cdr(v))
      {
        l1=cons_val(val_car(v),l1);
      }
    l1 = cons_val(string_val("#"),l1);
    l1 = val_reverse(l1);
    return l1;
}

cst_val* ru_lts_apply(const cst_val *input, const cst_lts_rewrites *rule)
{
  cst_val *l=add_lts_boundary_marks(input);
  cst_val *output=lts_rewrites(l, rule);
  delete_val(l);
  return output;
}

cst_val* ustring32_lts_apply(const ustring32_t u32,const cst_lts_rewrites *rule)
{
  size_t n=ustring32_length(u32);
  if(n==0) return NULL;
  cst_val *l=cons_val(string_val("#"),NULL);
  uint8_t b[8];
  size_t i=n;
  int k;
  do
    {
      i--;
      k=u8_uctomb(b,ustring32_at(u32,i),sizeof(b));
      b[k]='\0';
      l=cons_val(string_val((char*)b),l);
    }
  while(i);
  l=cons_val(string_val("#"),l);
  cst_val *output=lts_rewrites(l, rule);
  delete_val(l);
  return output;
}

char *ru_implode(const cst_val *l)
{
  const cst_val *v;
  size_t n=1;
  char *s,*p;
  for(v=l;(v!=NULL);v=val_cdr(v))
    {
      if(val_stringp(val_car(v)))
        n+=strlen(val_string(val_car(v)));
    }
  s=cst_alloc(char,n);
  p=s;
  for(v=l;(v!=NULL);v=val_cdr(v))
    {
      if(val_stringp(val_car(v)))
        {
          n=strlen(val_string(val_car(v)));
          memcpy(p,val_string(val_car(v)),n);
          p+=n;
        }
    }
  *p='\0';
  return s;
}

static const char* vpairs[][2]=
  {
    {"b","p"},
    {"bb","pp"},
    {"v","f"},
    {"vv","ff"},
    {"g","k"},
    {"gg","kk"},
    {"d","t"},
    {"dd","tt"},
    {"zh","sh"},
    {"z","s"},
    {"zz","ss"},
    {NULL,NULL}
  };

void ru_ff_register(cst_features *ffunctions);

const char *russian_vpair(const char *ph)
{
  int i;
  for(i=0;vpairs[i][0];++i)
    {
      if(cst_streq(vpairs[i][0],ph))
        return vpairs[i][1];
      if(cst_streq(vpairs[i][1],ph))
        return vpairs[i][0];
    }
  return NULL;
}

void russian_init(cst_voice *v)
{
    feat_set(v->features,"phoneset",phoneset_val(&ru_phoneset));
    feat_set_string(v->features,"silence",ru_phoneset.silence);
    feat_set(v->features,"textanalysis_func",uttfunc_val(&russian_textanalysis));
    feat_set(v->features,"lexical_insertion_func",uttfunc_val(&russian_lexical_insertion));
    feat_set(v->features,"phrasing_func",uttfunc_val(&russian_phrasify));
    feat_set(v->features,"pause_insertion_func",uttfunc_val(&russian_pause_insertion));
    feat_set(v->features,"intonation_func",uttfunc_val(&do_nothing));
    feat_set(v->features,"postlex_func",uttfunc_val(russian_postlex_function));
    feat_set(v->features,"duration_model_func",uttfunc_val(do_nothing));
    feat_set(v->features,"f0_model_func",uttfunc_val(do_nothing));
    feat_set(v->features,"cmu_lex",lexicon_val(cmu_lex_init()));
    ru_ff_register(v->ffunctions);
}
