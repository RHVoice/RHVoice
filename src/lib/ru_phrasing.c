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

static const ucs4_t phrase_final_punc_list[]={'.','!','?',',',';',':','-',')','\0'};

static cst_utterance *assign_phrase_breaks(cst_utterance *u)
{
  cst_item *w=relation_tail(utt_relation(u,"Word"));
  if(w)
    item_set_string(w,"phrbrk","B");
  else
    return u;
  cst_item *t0,*t;
  const uint8_t *p;
  int bs;
  for(t0=relation_head(utt_relation(u,"Token"));t0;t0=item_next(t0))
    {
      w=item_last_daughter(t0);
      if(w!=NULL)
        {
          t=(item_next(item_as(w,"Word"))?path_to_item(w,"R:Word.n.R:Token.parent.p"):t0);
          bs=item_feat_int(t,"break_strength");
          ucs4_t c;
          if((bs!='n')&&(bs!='u'))
            {
              item_set_string(w,"phrbrk","B");
              continue;
            }
          if(item_next(t)&&
             (ffeature_string(t,"n.prepunctuation")[0]=='('))
            {
              item_set_string(w,"phrbrk","B");
              continue;
            }
          p=(const uint8_t*)item_feat_string(t,"punc");
          while((p=u8_next(&c,p)))
            {
              if(u32_strchr(phrase_final_punc_list,c))
                {
                  item_set_string(w,"phrbrk","B");
                  break;
                }
            }
        }
    }
  int num_words_to_prev_break=0;
  int num_words_to_next_break;
  int num_cont_words_to_prev_break=0;
  int num_cont_words_to_next_break;
  cst_item *w1;
  for(w=relation_head(utt_relation(u,"Word"));w;w=item_next(w))
    {
      num_words_to_prev_break++;
      if(cst_streq(ffeature_string(w,"gpos"),"content"))
        num_cont_words_to_prev_break++;
      if(cst_streq(item_feat_string(w,"phrbrk"),"NB")) continue;
      else if(cst_streq(item_feat_string(w,"phrbrk"),"B"))
        {
          num_words_to_prev_break=0;
          num_cont_words_to_prev_break=0;
          continue;
        }
      else if(!cst_streq(ffeature_string(w,"gpos"),"proc")&&cst_streq(ffeature_string(w,"n.gpos"),"proc"))
        {
          num_words_to_next_break=0;
          num_cont_words_to_next_break=0;
          for(w1=item_next(w);w1;w1=item_next(w1))
            {
              num_words_to_next_break++;
              if(cst_streq(ffeature_string(w1,"gpos"),"content"))
                num_cont_words_to_next_break++;
              if(cst_streq(item_feat_string(w1,"phrbrk"),"B")) break;
            }
          if(((num_words_to_prev_break+num_words_to_next_break)>=9)&&
             (cst_streq(ffeature_string(w,"n.name"),"и")&&(num_cont_words_to_prev_break>=2)&&(num_cont_words_to_next_break>=2)))
            {
              item_set_string(w,"phrbrk","B");
              num_words_to_prev_break=0;
              num_cont_words_to_prev_break=0;
            }
        }
    }
  num_words_to_prev_break=0;
  for(w=relation_head(utt_relation(u,"Word"));w;w=item_next(w))
    {
      num_words_to_prev_break++;
      if(cst_streq(item_feat_string(w,"phrbrk"),"NB")) continue;
      else if(cst_streq(item_feat_string(w,"phrbrk"),"B"))
        {
          num_words_to_prev_break=0;
          continue;
        }
      else if(num_words_to_prev_break>25)
        {
          num_words_to_next_break=0;
          for(w1=item_next(w);w1;w1=item_next(w1))
            {
              num_words_to_next_break++;
              if(cst_streq(item_feat_string(w1,"phrbrk"),"B")) break;
            }
          if(num_words_to_next_break>5)
            {
              item_set_string(w,"phrbrk","B");
              num_words_to_prev_break=0;
            }
        }
    }
  return u;
}

cst_utterance *russian_phrasify(cst_utterance *u)
{
  cst_relation *r;
  cst_item *w,*n,*p=NULL;
  assign_phrase_breaks(u);
  r = utt_relation_create(u,"Phrase");
  for (p=NULL,w=relation_head(utt_relation(u,"Word")); w; w=item_next(w))
    {
      if (p == NULL)
	{
          p = relation_append(r,NULL);
          item_set_string(p,"name","B");
	}
      item_add_daughter(p,w);
      if(cst_streq(item_feat_string(w,"phrbrk"),"B"))
        {
          n=item_next(w);
          if(n)
            item_set_float(p,"break_time",ffeature_float(n,"R:Token.parent.p.break_time"));
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
  float time,factor;
  cst_item *s,*w1,*w2,*t1,*t2,*p1,*p2;
  w2=NULL;
  t2=NULL;
  p2=NULL;
  w1=relation_tail(utt_relation(u,"Transcription"));
  while(w1)
    {
      t1=item_parent(item_as(w1,"Token"));
      p1=item_parent(item_as(w1,"Phrase"));
      if(p1!=p2)
        {
          s=item_append(item_as(item_last_daughter(w1),"Segment"),NULL);
          item_set_string(s,"name","pau");
          time=item_feat_float(p1,"break_time");
          item_set_float(s,"time",time);
          if(time==0)
            factor=1;
          else factor=0;
          item_set_float(s,"factor",factor);
        }
      w2=w1;
      t2=t1;
      p2=p1;
      w1=item_prev(w1);
    }
  if(w2)
    {
      s=item_prepend(relation_head(utt_relation(u,"Segment")),NULL);
      item_set_string(s,"name","pau");
      item_set_float(s,"time",0.0);
      item_set_float(s,"factor",1.0);
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
