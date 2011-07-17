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

#include <unistr.h>
#include "lib.h"

static int should_be_final_in_phrase(cst_item *w)
{
  if(item_next(item_as(w,"Token"))!=NULL) return 0;
  cst_item *n=item_next(item_as(w,"Word"));
  if(n==NULL) return 1;
  cst_item *nt=item_parent(item_as(n,"Token"));
  cst_item *t=item_prev(nt);
  int bs=item_feat_int(t,"break_strength");
  if(bs!='u')
    {
      if((bs=='s')||(bs=='b')) return 1;
      else return 0;
    }
  const uint8_t *p=(const uint8_t*)item_feat_string(nt,"prepunctuation");
  if(p[0]=='(') return 1;
  if(strlen(item_name(nt))==0) return 0;
  p=(const uint8_t*)item_feat_string(t,"punc");
  ucs4_t c;
  unsigned int cs;
  int result=0;
  while((p=u8_next(&c,p)))
    {
      cs=classify_character(c);
      if(cs&cs_pq) continue;
      if(cs&cs_pf)
        {
          result=1;
          break;
        }
    }
  return result;
}

cst_utterance *russian_phrasify(cst_utterance *u)
{
  cst_relation *r;
  cst_item *w,*n,*p=NULL;
  r = utt_relation_create(u,"Phrase");
  for (p=NULL,w=relation_head(utt_relation(u,"Word")); w; w=item_next(w))
    {
      if (p == NULL)
	{
          p = relation_append(r,NULL);
          item_set_string(p,"name","B");
	}
      item_add_daughter(p,w);
      if(should_be_final_in_phrase(w))
        {
          n=item_next(w);
          if(n)
            item_set_float(p,"break_time",item_feat_float(item_prev(item_parent(item_as(n,"Token"))),"break_time"));
          p = NULL;
        }
    }
  p=relation_tail(r);
  if(p)
    item_set_float(p,"break_time",item_feat_float(relation_tail(utt_relation(u,"Token")),"break_time"));
  return u;
}

cst_utterance *russian_pause_insertion(cst_utterance *u)
{
  const char *p=NULL;
  float time2=feat_float(u->features,"silence_time");
  float time1,factor;
  cst_item *s,*w1,*w2,*t1,*t2,*p1,*p2;
  w2=NULL;
  t2=NULL;
  p2=NULL;
  w1=relation_tail(utt_relation(u,"Transcription"));
  while(w1)
    {
      t1=item_parent(item_as(w1,"Token"));
      p1=item_parent(item_as(w1,"Phrase"));
      time1=item_feat_float(t1,"silence_time");
      if((p1!=p2)||(time1!=time2))
        {
          s=item_append(item_as(item_last_daughter(w1),"Segment"),NULL);
          item_set_string(s,"name","pau");
          item_set_float(s,"time",time2-time1);
          if((p1!=p2)&&(item_feat_float(p1,"break_time")==0))
            factor=1;
          else factor=0;
          item_set_float(s,"factor",factor);
        }
      w2=w1;
      t2=t1;
      p2=p1;
      time2=time1;
      w1=item_prev(w1);
    }
  if(w2)
    s=item_prepend(relation_head(utt_relation(u,"Segment")),NULL);
  else if(feat_float(u->features,"silence_time")!=0)
    s=relation_append(utt_relation(u,"Segment"),NULL);
  else s=NULL;
  if(s!=NULL)
    {
      item_set_string(s,"name","pau");
      item_set_float(s,"time",(time2>0)?time2:0.0);
      item_set_float(s,"factor",(time2>0)?0.0:1.0);
    }
  s=relation_tail(utt_relation(u,"Segment"));
  if((s!=NULL)&&(item_feat_float(s,"time")==0.0))
    {
      item_set_float(s,"factor",0.0);
      t1=relation_tail(utt_relation(u,"Token"));
      if(t1!=NULL)
        {
          p=item_feat_string(t1,"punc");
          if(p[0]=='\0')
            item_set_float(s,"time",0.05);
          else if(strchr(p,'.')||strchr(p,'?')||strchr(p,'!'))
            item_set_float(s,"time",0.2);
          else if(strchr(p,':')||strchr(p,';'))
            item_set_float(s,"time",0.15);
          else
            item_set_float(s,"time",0.1);
        }
    }
  return u;
}
