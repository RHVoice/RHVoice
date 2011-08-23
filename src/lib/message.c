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

#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <expat.h>
#include "lib.h"
#include "vector.h"
#include "ustring.h"
#include "settings.h"

#define ssml_error(s) {s->error_flag=1;XML_StopParser(s->parser,XML_FALSE);return;}

#define max_token_len 80
#define max_sentence_len 800
#define max_line_len 80
#define max_tokens_in_sentence 100

typedef struct {
  float value;
  int is_absolute;
} prosody_param;

typedef struct {
  prosody_param rate,pitch,volume;
} prosody_params;

static prosody_params default_prosody_params={{1.0,0},{1.0,0},{1.0,0}};

struct token_struct;

typedef struct {
  int pos;
  uint8_t *name;
  int next_token_index;
  float silence;
} mark;

static void mark_free(mark *m)
{
  if(m==NULL) return;
  free(m->name);
}

static int report_mark(mark *m,RHVoice_message msg,RHVoice_callback f)
{
  RHVoice_event e;
  e.message=msg;
  e.type=RHVoice_event_mark;
  e.audio_position=0;
  e.text_position=m->pos+1;
  e.text_length=0;
  e.id.name=(const char*)m->name;
  return f(NULL,0,&e,1,msg);
}

vector_t(mark,marklist)

typedef struct {
  RHVoice_punctuation_mode mode;
  uint32_t *list;
} punct_option;

void free_punct_option(punct_option *p)
{
  if((p!=NULL)&&(p->list!=NULL))
    free(p->list);
}

vector_t(punct_option,punct_opt_list)

typedef enum {
  token_sentence_start=1 << 0,
  token_sentence_end=1 << 1,
  token_eol=1 << 2,
  token_eop=1 << 3
} token_flags;

typedef struct {
  unsigned int flags;
  int pos;
  int len;
  ustring32_t text;
  prosody_params prosody;
  int break_strength;
  float break_time;
  float silence;
  int sentence_number;
  int mark_index;
  int say_as;
  uint8_t *say_as_format;
  int variant;
  int voice;
  size_t punct_opt_index;
} token;

static void token_free(token *t)
{
  if(t==NULL) return;
  ustring32_free(t->text);
  if(t->say_as_format!=NULL) free(t->say_as_format);
  return;
}

vector_t(token,toklist)

typedef struct {
  toklist l;
  ucs4_t c;
  unsigned int cs;
  size_t ln;
} tstream;

static void tstream_init(tstream *ts,toklist tl)
{
  ts->l=tl;
  ts->c='\0';
  ts->cs=cs_sp;
  ts->ln=0;
}

static int tstream_putc (tstream *ts,ucs4_t c,size_t src_pos,size_t src_len,int say_as)
{
  token tok;
  tok.flags=0;
  tok.pos=src_pos;
  tok.len=src_len;
  tok.text=NULL;
  tok.prosody=default_prosody_params;
  tok.break_strength='u';
  tok.break_time=0;
  tok.silence=0;
  tok.sentence_number=0;
  tok.mark_index=-1;
  tok.say_as=say_as;
  tok.say_as_format=NULL;
  tok.variant=0;
  tok.voice=0;
  tok.punct_opt_index=0;
  token *prev_tok=toklist_back(ts->l);
  unsigned int cs=classify_character(c);
  if(prev_tok&&(cs&(cs_nl|cs_pr)))
    {
      prev_tok->flags|=token_eol;
      if((cs&cs_pr)||((ts->cs&cs_nl)&&!((ts->c=='\r')&&(c=='\n')))||(ts->ln>=max_line_len))
        prev_tok->flags|=token_eop;
      ts->ln=0;
    }
  if((!(cs&cs_ws))||(say_as=='s')||(say_as=='c'))
    {
      if((ts->cs&cs_ws)||(prev_tok&&((prev_tok->say_as=='c')||(say_as=='s')||(say_as=='c')||(ustring32_length(prev_tok->text)==max_token_len))))
        {
          tok.text=ustring32_alloc(1);
          if(tok.text==NULL) return 0;
          if(!toklist_push(ts->l,&tok))
            {
              ustring32_free(tok.text);
              return 0;
            }
        }
      prev_tok=toklist_back(ts->l);
      if(!ustring32_push(prev_tok->text,c)) return 0;
      if(src_len>0) prev_tok->len=src_pos-prev_tok->pos+src_len;
      ts->ln++;
    }
  ts->c=c;
  ts->cs=cs;
  return 1;
}

typedef enum {
  ssml_audio=0,
  ssml_break,
  ssml_desc,
  ssml_emphasis,
  ssml_lexicon,
  ssml_mark,
  ssml_meta,
  ssml_metadata,
  ssml_p,
  ssml_phoneme,
  ssml_prosody,
  ssml_s,
  ssml_say_as,
  ssml_speak,
  ssml_sub,
  ssml_style,
  ssml_voice,
  ssml_unknown,
  ssml_max
} ssml_tag_id;

const char *ssml_tag_names[ssml_max-1]={
  "audio",
  "break",
  "desc",
  "emphasis",
  "lexicon",
  "mark",
  "meta",
  "metadata",
  "p",
  "phoneme",
  "prosody",
  "s",
  "say-as",
  "speak",
  "sub",
  "tts:style",
  "voice"};

static const int ssml_element_table[ssml_max][ssml_max+1]={
  {1,1,1,1,0,1,0,0,1,1,1,1,1,0,1,1,1,0,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,0,1,0,1,0,0,0,1,1,0,1,0,1,1,1,0,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,0,1,0,1,0,0,0,1,1,1,1,0,1,1,1,0,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,0,1,0,1,0,0,1,1,1,1,1,0,1,1,1,0,1},
  {1,1,0,1,0,1,0,0,0,1,1,0,1,0,1,1,1,0,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,0,1,0,1,0,0,1,1,1,1,1,0,1,1,1,0,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,0,1,0,1,0,0,1,1,1,1,1,0,1,1,1,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static int ssml_tag_name_cmp(const void *key,const void *element)
{
  const char *s1=(const char*)key;
  const char *s2=*((const char**)element);
  return strcmp(s1,s2);
}

static ssml_tag_id ssml_get_tag_id(const char *name)
{
  const char **found=(const char**)bsearch(name,ssml_tag_names,ssml_max-1,sizeof(const char*),ssml_tag_name_cmp);
  return (found==NULL)?ssml_unknown:(found-&ssml_tag_names[0]);
}

typedef struct {
  uint8_t *name;
  uint8_t *value;
} ssml_attribute;

static void ssml_attribute_free(ssml_attribute *a)
{
  if(a==NULL) return;
  if(a->name!=NULL) free(a->name);
  if(a->value!=NULL) free(a->value);
}

vector_t(ssml_attribute,ssml_attr_list)

static ssml_attr_list ssml_copy_attributes(const char **atts)
{
  ssml_attr_list alist=NULL;
  ssml_attribute a;
  size_t i,n;
  alist=ssml_attr_list_alloc(0,ssml_attribute_free);
  if(alist==NULL) return NULL;
  for(i=0;atts[i]!=NULL;i+=2) {}
  n=i/2;
  if(n==0) return alist;
  if(!ssml_attr_list_reserve(alist,n))
    {
      ssml_attr_list_free(alist);
      return NULL;
    }
  for(i=0;atts[i]!=NULL;i+=2)
    {
      a.name=u8_strdup((const uint8_t*)atts[i]);
      a.value=u8_strdup((const uint8_t*)atts[i+1]);
      ssml_attr_list_push(alist,&a);
      if((a.name==NULL)||(a.value==NULL))
        {
          ssml_attr_list_free(alist);
          return NULL;
        }
    }
  return alist;
}

typedef struct {
  ssml_tag_id id;
  ssml_attr_list attributes;
} ssml_tag;

static void ssml_tag_free(ssml_tag *t)
{
  if(t==NULL) return;
  if(t->attributes!=NULL) ssml_attr_list_free(t->attributes);
}

static const uint8_t *ssml_get_attribute_value(const ssml_tag *t,const char *name)
{
  size_t i,n;
  ssml_attribute *a;
  if(t==NULL) return NULL;
  if(t->attributes==NULL) return NULL;
  for(a=ssml_attr_list_data(t->attributes),i=0,n=ssml_attr_list_size(t->attributes);i<n;i++)
    {
      if(u8_strcmp(a[i].name,(const uint8_t*)name)==0)
        return a[i].value;
    }
  return NULL;
}

vector_t(ssml_tag,ssml_tag_stack)

vector_t(prosody_params,prosody_stack)

vector_t(int,int_stack)

typedef union {
  const uint8_t *u8;
  const uint16_t *u16;
  const uint32_t *u32;
} ustr;

typedef struct
{
  int encoding;
  ustr text;
  int len;
  int is_ssml;
} source_info;

typedef struct {
  source_info src;
  const uint8_t *text;
  size_t len;
  XML_Parser parser;
  RHVoice_message msg;
  ssml_tag_stack tags;
  prosody_stack prosody;
  int_stack variants;
  int_stack voices;
  int_stack punctuation;
  int in_cdata_section;
  int start_sentence;
  unsigned int skip;
  tstream ts;
  size_t text_start;
  size_t text_start_in_chars;
  ssml_tag_id last_closed_element;
  int say_as;
  const uint8_t *say_as_format;
  int error_flag;
} ssml_state;

struct RHVoice_message_s
{
  toklist tokens;
  marklist marks;
  size_t pos;
  float silence;
  void *user_data;
  float rate,pitch,volume;
  uint32_t *default_punct_list;
  punct_opt_list punct_opts;
};

static RHVoice_message RHVoice_message_alloc(void)
{
  punct_option p;
  RHVoice_message msg=(RHVoice_message)malloc(sizeof(struct RHVoice_message_s));
  if(msg==NULL) goto err0;
  msg->pos=0;
  msg->user_data=NULL;
  msg->rate=-1;
  msg->pitch=-1;
  msg->volume=-1;
  msg->silence=0;
  msg->tokens=toklist_alloc(16,token_free);
  if(msg->tokens==NULL) goto err1;
  msg->marks=marklist_alloc(0,mark_free);
  if(msg->marks==NULL) goto err2;
  p.list=NULL;
  p.mode=RHVoice_get_punctuation_mode();
  msg->default_punct_list=copy_punctuation_list();
  if(msg->default_punct_list==NULL) goto err3;
  msg->punct_opts=punct_opt_list_alloc(1,free_punct_option);
  if(msg->punct_opts==NULL) goto err4;
  punct_opt_list_push(msg->punct_opts,&p);
  return msg;
  err4: free(msg->default_punct_list);
  err3: marklist_free(msg->marks);
  err2: toklist_free(msg->tokens);
  err1: free(msg);
  err0: return NULL;
}

static void RHVoice_message_free(RHVoice_message msg)
{
  if(msg==NULL) return;
  toklist_free(msg->tokens);
  marklist_free(msg->marks);
  free(msg->default_punct_list);
  punct_opt_list_free(msg->punct_opts);
  free(msg);
}

static int prosody_parse_as_number(const char *value,float *number,int *sign,char **suffix)
{
  float res;
  const char *str=value;
  int s=0;
  char *end;
  if(str[0]=='+')
    {
      s=1;
      str++;
    }
  else if(str[0]=='-')
    {
      s=-1;
      str++;
    }
  if(!(((str[0]>='0')&&(str[0]<='9'))||(str[0]=='.'))) return 0;
  res=strtod_c(str,&end);
  if(end==str) return 0;
  if(res<0) return 0;
  *number=res;
  *sign=s;
  *suffix=end;
  return 1;
}

static prosody_stack prosody_stack_update(prosody_stack stack,const ssml_tag *tag)
{
  float dvolume=RHVoice_get_default_volume();
  prosody_params p=*prosody_stack_back(stack);
  float fval=1.0;
  const char *strval=NULL;
  int sign=0;
  char *suffix=NULL;
  strval=(const char*)ssml_get_attribute_value(tag,"rate");
  if(strval)
    {
      if(strcmp(strval,"default")==0)
        p.rate=default_prosody_params.rate;
      else if(prosody_parse_as_number(strval,&fval,&sign,&suffix))
        {
          if((sign==0)&&(strlen(suffix)==0))
            {
              p.rate.value*=fval;
            }
          else if(strcmp(suffix,"%")==0)
            {
              if(sign!=0) fval=100+sign*fval;
              if(fval<0) fval=0;
              fval/=100.0;
              p.rate.value*=fval;
            }
        }
    }
  strval=(const char*)ssml_get_attribute_value(tag,"pitch");
  if(strval)
    {
      if(strcmp(strval,"default")==0)
        p.pitch=default_prosody_params.pitch;
      else if(prosody_parse_as_number(strval,&fval,&sign,&suffix))
        {
          if(strcmp(suffix,"%")==0)
            {
              if(sign!=0) fval=100+sign*fval;
              if(fval<0) fval=0;
              fval/=100.0;
              p.pitch.value*=fval;
            }
        }
    }
  strval=(const char*)ssml_get_attribute_value(tag,"volume");
  if(strval)
    {
      if(strcmp(strval,"default")==0)
        p.volume=default_prosody_params.volume;
      else if(prosody_parse_as_number(strval,&fval,&sign,&suffix))
        {
          if((sign==0)&&(strlen(suffix)==0))
            {
              if(fval>100) fval=100;
              fval/=50.0;
              fval-=1;
              p.volume.is_absolute=1;
              p.volume.value=dvolume+fval*((fval>=0)?(RHVoice_get_max_volume()-dvolume):dvolume);
            }
          else if(strcmp(suffix,"%")==0)
            {
              if(sign!=0) fval=100+sign*fval;
              if(fval<0) fval=0;
              fval/=100.0;
              p.volume.value*=fval;
            }
        }
    }
  return prosody_stack_push(stack,&p);
}

static int ssml_add_mark(ssml_state *state)
{
  ssml_tag *top=ssml_tag_stack_back(state->tags);
  const uint8_t *name=ssml_get_attribute_value(top,"name");
  if(name==NULL) return 0;
  size_t pos=XML_GetCurrentByteIndex(state->parser);
  mark m;
  m.pos=state->text_start_in_chars+u8_mbsnlen(state->text+state->text_start,pos-state->text_start);
  m.name=u8_strdup(name);
  if(m.name==NULL) return 0;
  m.next_token_index=toklist_size(state->msg->tokens);
  m.silence=state->msg->silence;
  if(!marklist_push(state->msg->marks,&m))
    {
      free(m.name);
      return 0;
    }
  return 1;
}

#define max_pause 60

static int ssml_add_break(ssml_state *state)
{
  int strength='u';
  float time=0;
  ssml_tag *top=ssml_tag_stack_back(state->tags);
  const char *strstrength=(const char*)ssml_get_attribute_value(top,"strength");
  if(strstrength!=NULL)
    {
      if(strcmp(strstrength,"none")==0) strength='n';
      else if(strcmp(strstrength,"x-weak")==0) strength='x';
      else if(strcmp(strstrength,"weak")==0) strength='w';
      else if(strcmp(strstrength,"medium")==0) strength='m';
      else if(strcmp(strstrength,"strong")==0) strength='s';
      else if(strcmp(strstrength,"x-strong")==0) strength='X';
      else return 0;
    }
  const char *strtime=(const char*)ssml_get_attribute_value(top,"time");
  if(strtime!=NULL)
    {
      char *suffix=NULL;
      time=strtol(strtime,&suffix,10);
      if(suffix==strtime) return 0;
      if(time<0) return 0;
      if(strcmp(suffix,"s")==0) ;
      else if(strcmp(suffix,"ms")==0) time/=1000.0;
      else if((time==0)&&(suffix[0]!='\0')) return 0;
      else return 0;
      if(time>max_pause) time=max_pause;
    }
  else if(strength=='u') strength='b';
  token *tok=toklist_back(state->msg->tokens);
  if((tok!=NULL)&&(tok->break_strength=='u')&&(tok->break_time==0))
    {
      tok->break_strength=strength;
      tok->break_time=time;
    }
  state->msg->silence+=time;
  return 1;
}

static int ssml_translate_say_as_content_type(ssml_tag *t)
{
  const char *strtype=(const char*)ssml_get_attribute_value(t,"interpret-as");
  if(strtype==NULL) return -1;
  if(strcmp(strtype,"characters")==0) return 's';
  else if(strcmp(strtype,"tts:char")==0) return 'c';
  else return 0;
}

static int_stack variant_stack_update(int_stack stack,ssml_tag *tag)
{
  int variant=*int_stack_back(stack);
  if(!int_stack_push(stack,&variant)) return NULL;
  const char *strvariant=(const char*)ssml_get_attribute_value(tag,"variant");
  if(strvariant==NULL) return stack;
  char *suffix;
  variant=strtol(strvariant,&suffix,10);
  if((variant>0)&&(variant<=RHVoice_get_variant_count())&&(suffix[0]=='\0'))
    *int_stack_back(stack)=variant;
  return stack;
}

static int_stack voice_stack_update(int_stack stack,ssml_tag *tag)
{
  int voice=*int_stack_back(stack);
  if(!int_stack_push(stack,&voice)) return NULL;
  const uint8_t *val=ssml_get_attribute_value(tag,"name");
  if(val==NULL) return stack;
  uint8_t *names=u8_strdup(val);
  if(names==NULL) return NULL;
  uint8_t *p=NULL;
  const uint8_t *name=u8_strtok(names,(const uint8_t*)" ",&p);
  if(name==NULL)
    {
      free(names);
      return stack;
    }
  voice=RHVoice_find_voice((const char*)name);
  free(names);
  if(voice>0)
    *int_stack_back(stack)=voice;
  return stack;
}

static ssml_state *punct_stack_update(ssml_state *state,ssml_tag *tag)
{
  int index=*int_stack_back(state->punctuation);
  if(!int_stack_push(state->punctuation,&index)) return NULL;
  const uint8_t *strval=ssml_get_attribute_value(tag,"field");
  if(strval==NULL) return NULL;
  if(u8_strcmp(strval,(const uint8_t*)"punctuation")!=0) return state;
  strval=ssml_get_attribute_value(tag,"mode");
  if(strval==NULL) return NULL;
  punct_option p;
  if(u8_strcmp(strval,(const uint8_t*)"all")==0)
    p.mode=RHVoice_punctuation_all;
  else if(u8_strcmp(strval,(const uint8_t*)"none")==0)
    p.mode=RHVoice_punctuation_none;
  else if(u8_strcmp(strval,(const uint8_t*)"some")==0)
    p.mode=RHVoice_punctuation_some;
  else
    return NULL;
  p.list=NULL;
  if(!punct_opt_list_push(state->msg->punct_opts,&p)) return NULL;
  *int_stack_back(state->punctuation)=punct_opt_list_size(state->msg->punct_opts)-1;
  if(p.mode!=RHVoice_punctuation_some) return state;
  strval=ssml_get_attribute_value(tag,"detail");
  if(strval==NULL) return state;
  size_t n=0;
  p.list=u8_to_u32(strval,u8_strlen(strval)+1,NULL,&n);
  if(p.list==NULL) return NULL;
  punct_opt_list_back(state->msg->punct_opts)->list=p.list;
  return state;
}

static void XMLCALL ssml_element_start(void *user_data,const char *name,const char **atts)
{
  ssml_state *state=(ssml_state*)user_data;
  if(state->skip) {state->skip++;return;}
  if(!tstream_putc(&state->ts,' ',0,0,0)) ssml_error(state);
  ssml_tag tag;
  tag.id=ssml_get_tag_id(name);
  int accept=(ssml_tag_stack_size(state->tags)==0)?(tag.id==ssml_speak):ssml_element_table[ssml_tag_stack_back(state->tags)->id][tag.id];
  if(!accept) ssml_error(state);
  if(tag.id==ssml_metadata) {state->skip=1;return;}
  tag.attributes=ssml_copy_attributes(atts);
  if(tag.attributes==NULL) ssml_error(state);
  if(!ssml_tag_stack_push(state->tags,&tag)) ssml_error(state);
  ssml_tag *top=ssml_tag_stack_back(state->tags);
  switch(top->id)
    {
    case ssml_prosody:
      if(!prosody_stack_update(state->prosody,top)) ssml_error(state);
      break;
    case ssml_s:
      state->start_sentence=1;
      break;
    case ssml_p:
      tstream_putc(&state->ts,8233,0,0,0);
      break;
    case ssml_mark:
      if(!ssml_add_mark(state)) ssml_error(state);
      break;
    case ssml_break:
      if(!ssml_add_break(state)) ssml_error(state);
      break;
    case ssml_say_as:
      state->say_as=ssml_translate_say_as_content_type(top);
      if(state->say_as==-1) ssml_error(state);
      state->say_as_format=ssml_get_attribute_value(top,"format");
      break;
    case ssml_voice:
      if(!variant_stack_update(state->variants,top)) ssml_error(state);
      if(!voice_stack_update(state->voices,top)) ssml_error(state);
      break;
    case ssml_style:
      if(!punct_stack_update(state,top)) ssml_error(state);
      break;
    default:
      break;
    }
}

static void XMLCALL ssml_element_end(void *user_data,const char *name)
{
  ssml_state *state=(ssml_state*)user_data;
  if(state->skip)
    {
      state->skip--;
      if(state->skip==0) state->last_closed_element=ssml_metadata;
      return;
    }
  if(!tstream_putc(&state->ts,' ',0,0,0)) ssml_error(state);
  ssml_tag *top=ssml_tag_stack_back(state->tags);
  switch(top->id)
    {
    case ssml_prosody:
      prosody_stack_pop(state->prosody);
      break;
    case ssml_s:
      if(state->start_sentence) state->start_sentence=0;
      else toklist_back(state->msg->tokens)->flags|=token_sentence_end;
      break;
    case ssml_p:
      if(!tstream_putc(&state->ts,8233,0,0,0)) ssml_error(state);
      break;
    case ssml_say_as:
      state->say_as=0;
      state->say_as_format=NULL;
      break;
    case ssml_voice:
      int_stack_pop(state->variants);
      int_stack_pop(state->voices);
      break;
    case ssml_style:
      int_stack_pop(state->punctuation);
      break;
    default:
      break;
    }
  state->last_closed_element=top->id;
  ssml_tag_stack_pop(state->tags);
}

static void XMLCALL ssml_character_data(void *user_data,const char *text,int len)
{
  ssml_state *state=(ssml_state*)user_data;
  if(state->skip||(ssml_tag_stack_size(state->tags)==0)) return;
  ssml_tag *top=ssml_tag_stack_back(state->tags);
  if(!ssml_element_table[top->id][ssml_max]) ssml_error(state);
  size_t src_len=XML_GetCurrentByteCount(state->parser);
  size_t src_start=XML_GetCurrentByteIndex(state->parser);
  size_t src_len_in_chars=(src_len==0)?0:u8_mbsnlen(state->text+src_start,src_len);
  size_t src_start_in_chars=state->text_start_in_chars+((src_start>state->text_start)?u8_mbsnlen(state->text+state->text_start,src_start-state->text_start):0);
  int no_refs=state->in_cdata_section?1:((src_len==0)?0:(u8_chr(state->text+src_start,src_len,'&')==NULL));
  size_t r=len;
  size_t p=src_start_in_chars;
  const uint8_t *str=(const uint8_t*)text;
  int n;
  ucs4_t c;
  token *tok;
  size_t prev_num_tokens=toklist_size(state->msg->tokens);
  while(r>0)
    {
      n=u8_mbtoucr(&c,str,r);
      if(!tstream_putc(&state->ts,c,p,no_refs?1:src_len_in_chars,state->say_as)) ssml_error(state);
      if(prev_num_tokens!=toklist_size(state->msg->tokens))
        {
          prev_num_tokens=toklist_size(state->msg->tokens);
          tok=toklist_back(state->msg->tokens);
          tok->prosody=*prosody_stack_back(state->prosody);
          tok->variant=*int_stack_back(state->variants);
          tok->voice=*int_stack_back(state->voices);
          tok->silence=state->msg->silence;
          if(state->start_sentence)
            {
              tok->flags|=token_sentence_start;
              state->start_sentence=0;
            }
          if((marklist_size(state->msg->marks)>0)&&
             (state->last_closed_element!=ssml_break)&&
             (marklist_back(state->msg->marks)->next_token_index==(toklist_size(state->msg->tokens)-1)))
                tok->mark_index=marklist_size(state->msg->marks)-1;
          if(state->say_as)
            {
              if(state->say_as_format!=NULL)
                {
                  tok->say_as_format=u8_strdup(state->say_as_format);
                  if(tok->say_as_format==NULL) ssml_error(state);
                }
              if(state->say_as!='s') state->say_as=0;
            }
          tok->punct_opt_index=*int_stack_back(state->punctuation);
        }
      r-=n;
      str+=n;
      if(no_refs) p++;
    }
  state->text_start=src_start;
  state->text_start_in_chars=src_start_in_chars;
}

static void XMLCALL ssml_cdata_section_start(void *user_data)
{
  ssml_state *state=(ssml_state*)user_data;
  if(state->skip) return;
  state->in_cdata_section=1;
}

static void XMLCALL ssml_cdata_section_end(void *user_data)
{
  ssml_state *state=(ssml_state*)user_data;
  if(state->skip) return;
  state->in_cdata_section=0;
}

static ssml_state *ssml_state_init(ssml_state *s,const source_info *i,RHVoice_message m)
{
  int z=0;
  s->src=*i;
  s->text=NULL;
  s->len=0;
  s->msg=m;
  s->error_flag=0;
  s->skip=0;
  s->in_cdata_section=0;
  s->start_sentence=0;
  s->text_start=0;
  s->text_start_in_chars=0;
  s->last_closed_element=ssml_unknown;
  s->say_as=0;
  s->say_as_format=NULL;
  s->parser=XML_ParserCreate("UTF-8");
  if(s->parser==NULL) goto err1;
  s->tags=ssml_tag_stack_alloc(10,ssml_tag_free);
  if(s->tags==NULL) goto err2;
  s->prosody=prosody_stack_alloc(10,NULL);
  if(s->prosody==NULL) goto err3;
  prosody_stack_push(s->prosody,&default_prosody_params);
  s->variants=int_stack_alloc(10,NULL);
  if(s->variants==NULL) goto err4;
  int_stack_push(s->variants,&z);
  s->voices=int_stack_alloc(10,NULL);
  if(s->voices==NULL) goto err5;
  int_stack_push(s->voices,&z);
  s->punctuation=int_stack_alloc(10,NULL);
  if(s->punctuation==NULL) goto err6;
  int_stack_push(s->punctuation,&z);
  switch(s->src.encoding)
    {
    case 16:
      s->text=u16_to_u8(s->src.text.u16,s->src.len,NULL,&s->len);
      break;
    case 32:
      s->text=u32_to_u8(s->src.text.u32,s->src.len,NULL,&s->len);
      break;
    default:
      s->text=s->src.text.u8;
      s->len=s->src.len;
      break;
    }
  if(s->text==NULL) goto err7;
  tstream_init(&s->ts,s->msg->tokens);
  XML_SetUserData(s->parser,s);
  XML_SetElementHandler(s->parser,ssml_element_start,ssml_element_end);
  XML_SetCharacterDataHandler(s->parser,ssml_character_data);
  XML_SetCdataSectionHandler(s->parser,ssml_cdata_section_start,ssml_cdata_section_end);
  return s;
  err7: int_stack_free(s->punctuation);
  err6: int_stack_free(s->voices);
  err5: int_stack_free(s->variants);
  err4: prosody_stack_free(s->prosody);
  err3: ssml_tag_stack_free(s->tags);
  err2: XML_ParserFree(s->parser);
  err1: return NULL;  
}

static void ssml_state_free(ssml_state *s)
{
  if(s==NULL) return;
  XML_ParserFree(s->parser);
  ssml_tag_stack_free(s->tags);
  prosody_stack_free(s->prosody);
  int_stack_free(s->variants);
  int_stack_free(s->voices);
  int_stack_free(s->punctuation);
  if(s->src.encoding!=8) free((uint8_t*)s->text);
}

static RHVoice_message parse_ssml(const source_info *i)
{
  ssml_state s;
  int error_flag=0;
  RHVoice_message msg=RHVoice_message_alloc();
  if(msg==NULL) return NULL;
  if(!ssml_state_init(&s,i,msg))
    {
      RHVoice_message_free(msg);
      return NULL;
    }
  size_t r=s.len;
  const char *str=(const char*)s.text;
  size_t n;
  while(r>0)
    {
      n=(r>4096)?4096:r;
      error_flag=(XML_Parse(s.parser,str,n,n==r)==XML_STATUS_ERROR);
      if(error_flag) break;
      error_flag=s.error_flag;
      if(error_flag) break;
      str+=n;
      r-=n;
    }
  ssml_state_free(&s);
  if(error_flag)
    {
      RHVoice_message_free(msg);
      return NULL;
    }
  return msg;
}

static RHVoice_message parse_text(const source_info *i)
{
  tstream ts;
  RHVoice_message msg=RHVoice_message_alloc();
  if(msg==NULL) return NULL;
  ucs4_t c;
  size_t r=i->len;
  size_t p=0;
  int n;
  ustr s=i->text;
  tstream_init(&ts,msg->tokens);
  while(r>0)
    {
      switch(i->encoding)
        {
        case 8:
          n=u8_mbtoucr(&c,s.u8,r);
          break;
        case 16:
          n=u16_mbtoucr(&c,s.u16,r);
          break;
        default:
          n=u32_mbtoucr(&c,s.u32,r);
          break;
        }
      if(!tstream_putc(&ts,c,p,1,0))
        {
          RHVoice_message_free(msg);
          return NULL;
        }
      r-=n;
      switch(i->encoding)
        {
        case 8:
          s.u8+=n;
          break;
        case 16:
          s.u16+=n;
          break;
        default:
          s.u32+=n;
          break;
        }
      p++;
    }
  return msg;
}

static size_t prepunctuation_length(const ustring32_t t)
{
  if(ustring32_empty(t)) return 0;
  const uint32_t *start=ustring32_str(t);
  const uint32_t *end=start+ustring32_length(t);
  const uint32_t *s=start;
  unsigned int cs;
  while(s<end)
    {
      cs=classify_character(*s);
      if(!(cs&cs_pi))
        {
          if((cs&cs_d)&&(s>start)&&(*(s-1)=='-'))
            s--;
          break;
        }
      s++;
    }
  return (s-start);
}

static size_t postpunctuation_length(const ustring32_t t)
{
  if(ustring32_empty(t)) return 0;
  const uint32_t *start=ustring32_str(t);
  const uint32_t *end=start+ustring32_length(t);
  const uint32_t *s=end;
  do
    {
      s--;
      if(!(classify_character(*s)&cs_pf)) {s++;break;}
    }
  while(s>start);
  return (end-s);
}

static int is_sentence_boundary(const token *t1,const token *t2)
{
  if(t1->break_strength!='u')
    {
      if(t1->break_strength=='X') return 1;
      else return 0;
    }
  if(t1->flags&token_eop) return 1;
  if(t1->say_as=='s') return 1;
  if((ustring32_length(t1->text)>=max_token_len)||(ustring32_length(t2->text)>=max_token_len))
    return 1;
  const uint32_t *str1=ustring32_str(t1->text);
  size_t len1=ustring32_length(t1->text);
  size_t pre1_len=prepunctuation_length(t1->text);
  size_t post1_len=postpunctuation_length(t1->text);
  if((post1_len==0)||(pre1_len+post1_len>=len1)) return 0;
  const uint32_t *post1=str1+len1-post1_len;
  const uint32_t *str2=ustring32_str(t2->text);
  size_t pre2_len=prepunctuation_length(t2->text);
  int starts_with_cap=(classify_character(str2[pre2_len])&cs_lu);
  if(u32_strchr(post1,'.'))
    {
      if(post1_len==1)
        {
          if(pre2_len==0)
            {
              if(starts_with_cap)
                {
                  if((len1==2)&&(classify_character(str1[0])&cs_lu)) return 0;
                  else return 1;
                }
              else return 0;
            }
          else return 1;
        }
      else return 1;
    }
    else if(u32_strchr(post1,'?')||u32_strchr(post1,'!')) return 1;
    else if ((post1[post1_len-1]==':')&&(pre2_len!=0))
      {
        unsigned int cs=classify_character(str2[0]);
        if(((cs&cs_pq)&&(cs&cs_pi))||((t1->flags&token_eol)&&(cs&cs_pd)))
          return 1;
        else return 0;
      }
    else return 0;
}

static void mark_sentence_boundaries(RHVoice_message msg)
{
  if(toklist_size(msg->tokens)==0) return;
  token *first=toklist_front(msg->tokens);
  token *last=toklist_back(msg->tokens);
  last->flags|=token_sentence_end;
  if(first==last)
    {
      first->flags|=token_sentence_start;
      return;
    }
  token *prev=first;
  token *next=prev+1;
  while(prev<last)
    {
      if(prev->flags&token_sentence_start)
        {
          for(;!(prev->flags&token_sentence_end);prev++);
          next=prev+1;
        }
      else if((next->flags&token_sentence_start)||is_sentence_boundary(prev,next))
        prev->flags|=token_sentence_end;
      prev=next;
      next++;
    }
  first->flags|=token_sentence_start;
  for(next=first;next<last;next++)
    {
      if(next->flags&token_sentence_end)
        (next+1)->flags|=token_sentence_start;
    }
  size_t sentence_len=0;
  size_t num_tokens=0;
  for(next=first;next<last;next++)
    {
      if(next->flags&token_sentence_start)
        {
          sentence_len=0;
          num_tokens=0;
        }
      sentence_len+=ustring32_length(next->text);
      num_tokens++;
      if((sentence_len>=max_sentence_len)||(num_tokens>=max_tokens_in_sentence))
        {
          next->flags|=token_sentence_end;
          (next+1)->flags|=token_sentence_start;
        }
    }
      int number=0;
      for(next=first;next<=last;next++)
        {
          if(next->flags&token_sentence_start) number++;
          next->sentence_number=number;
        }
}


static RHVoice_message new_message(const source_info *i)
{
  RHVoice_message msg;
  msg=(i->is_ssml)?parse_ssml(i):parse_text(i);
  if(msg==NULL) return NULL;
  mark_sentence_boundaries(msg);
  return msg;
}

RHVoice_message RHVoice_new_message_utf8(const uint8_t *text,int len,int is_ssml)
{
  if((text==NULL)||(len<=0)) return NULL;
  source_info i;
  if(u8_check(text,len)!=NULL) return NULL;
  i.encoding=8;
  i.text.u8=text;
  i.len=len;
  i.is_ssml=is_ssml;
  return new_message(&i);
}

RHVoice_message RHVoice_new_message_utf16(const uint16_t *text,int len,int is_ssml)
{
  if((text==NULL)||(len<=0)) return NULL;
  source_info i;
  if(u16_check(text,len)!=NULL) return NULL;
  i.encoding=16;
  i.text.u16=text;
  i.len=len;
  i.is_ssml=is_ssml;
  return new_message(&i);
}

RHVoice_message RHVoice_new_message_utf32(const uint32_t *text,int len,int is_ssml)
{
  if((text==NULL)||(len<=0)) return NULL;
  source_info i;
  if(u32_check(text,len)!=NULL) return NULL;
  i.encoding=32;
  i.text.u32=text;
  i.len=len;
  i.is_ssml=is_ssml;
  return new_message(&i);
}

void RHVoice_delete_message(RHVoice_message msg)
{
  if(msg==NULL) return;
  RHVoice_message_free(msg);
}

cst_utterance *next_utt_from_message(RHVoice_message msg)
{
  if(msg==NULL) return NULL;
  if(msg->pos>=toklist_size(msg->tokens)) return NULL;
  ustring32_t str32;
  const token *front=toklist_front(msg->tokens);
  const token *back=toklist_back(msg->tokens);
  const token *first=toklist_at(msg->tokens,msg->pos);
  const token *last;
  for(last=first;last!=back;last++)
    {
      if(last->flags&token_sentence_end) break;
    }
  ustring8_t str8=ustring8_alloc(64);
  if(str8==NULL) return NULL;
  cst_utterance *u=new_utterance();
  cst_relation *tr=utt_relation_create(u,"Token");
  cst_item *i;
  size_t len,pre_len,post_len,name_len;
  const token *tok;
  const mark *m;
  const punct_option *p;
  feat_set_int(u->features,"length",last->pos-first->pos+last->len);
  feat_set_int(u->features,"number",first->sentence_number);
  feat_set(u->features,"message",userdata_val(msg));
  feat_set_int(u->features,"voice_id",(first->voice)?(first->voice):RHVoice_get_voice());
  float msg_rate=(msg->rate==-1)?RHVoice_get_rate():msg->rate;
  float msg_pitch=(msg->pitch==-1)?RHVoice_get_pitch():msg->pitch;
  float msg_volume=(msg->volume==-1)?RHVoice_get_volume():msg->volume;
  float rate=check_rate_range(msg_rate*first->prosody.rate.value);
  feat_set_float(u->features,"rate",rate);
  float pitch=check_pitch_range(msg_pitch*first->prosody.pitch.value);
  feat_set_float(u->features,"pitch",pitch);
  float volume=check_volume_range(first->prosody.volume.value*(first->prosody.volume.is_absolute?1.0:msg_volume));
  feat_set_float(u->features,"volume",volume);
  float initial_silence=(first==front)?0:first->silence;
  float final_silence=((last==back)?msg->silence:toklist_at(msg->tokens,(last-front)+1)->silence)-initial_silence;
  feat_set_float(u->features,"silence_time",final_silence);
  for(tok=first;tok<=last;tok++)
    {
      i=relation_append(tr,NULL);
      str32=tok->text;
      ustring32_substr8(str8,str32,0,0);
      item_set_string(i,"text",(const char*)ustring8_str(str8));
      len=ustring32_length(str32);
      pre_len=prepunctuation_length(str32);
      post_len=postpunctuation_length(str32);
      name_len=(len<=(pre_len+post_len))?0:(len-pre_len-post_len);
      item_set_string(i,"whitespace"," ");
      if(name_len==0)
        item_set_string(i,"name","");
      else
        {
          ustring32_substr8(str8,str32,pre_len,name_len);
          item_set_string(i,"name",(const char*)ustring8_str(str8));
        }
      if(pre_len==0)
        item_set_string(i,"prepunctuation","");
      else
        {
          ustring32_substr8(str8,str32,0,pre_len);
          item_set_string(i,"prepunctuation",(const char*)ustring8_str(str8));
        }
      if(post_len==0)
        item_set_string(i,"punc","");
      else
        {
          ustring32_substr8(str8,str32,pre_len+name_len,post_len);
          item_set_string(i,"punc",(const char*)ustring8_str(str8));
        }
      pitch=check_pitch_range(tok->prosody.pitch.value*msg_pitch);
      item_set_float(i,"pitch",pitch);
      item_set_int(i,"position",tok->pos+1);
      item_set_int(i,"length",tok->len);
      item_set_int(i,"number",tok-front+1);
      item_set_float(i,"silence_time",tok->silence-initial_silence);
      item_set_int(i,"break_strength",tok->break_strength);
      item_set_float(i,"break_time",tok->break_time);
      if(tok->mark_index>=0)
        {
          m=marklist_at(msg->marks,tok->mark_index);
          item_set_string(i,"mark_name",(const char*)m->name);
          item_set_int(i,"mark_position",m->pos+1);
        }
      if(tok->say_as!=0)
        {
          item_set_int(i,"say_as",tok->say_as);
          if(tok->say_as_format!=NULL)
            item_set_string(i,"say_as_format",(const char*)tok->say_as_format);
        }
      item_set_int(i,"variant",(tok->variant?tok->variant:RHVoice_get_variant()));
      p=punct_opt_list_at(msg->punct_opts,tok->punct_opt_index);
      item_set_int(i,"punct_mode",p->mode);
      if(p->mode==RHVoice_punctuation_some)
        item_set(i,"punct_list",userdata_val((void*)((p->list==NULL)?(msg->default_punct_list):(p->list))));
    }
  if(last==back) feat_set_int(u->features,"last",1);
  msg->pos=last-front+1;
  ustring8_free(str8);
  return u;
}

int report_final_mark(RHVoice_message message,RHVoice_callback callback)
{
  int result=1;
  size_t num_marks=marklist_size(message->marks);
  size_t num_tokens=toklist_size(message->tokens);
  size_t i;
  mark *m;
  for(i=0;i<num_marks;i++)
    {
      m=marklist_at(message->marks,i);
      if(m->next_token_index==num_tokens)
        {
          result=report_mark(m,message,callback);
          if(!result) break;
        }
    }
  return result;
}

void RHVoice_set_user_data(RHVoice_message message,void *data)
{
  message->user_data=data;
}

void *RHVoice_get_user_data(RHVoice_message message)
{
  return message->user_data;
}

void RHVoice_set_message_rate(RHVoice_message message,float rate)
{
  message->rate=check_rate_range(rate);
}

void RHVoice_set_message_pitch(RHVoice_message message,float pitch)
{
  message->pitch=check_pitch_range(pitch);
}

void RHVoice_set_message_volume(RHVoice_message message,float volume)
{
  message->volume=check_volume_range(volume);
}

int RHVoice_set_position(RHVoice_message message,const RHVoice_position *position)
{
  if(message==NULL) return 0;
  size_t i,n;
  mark *m;
  int result=0;
  switch(position->type)
    {
    case RHVoice_position_word:
      n=toklist_size(message->tokens);
      if((n>0)&&(position->info.number>0))
        {
          if(position->info.number<=n)
            {
              message->pos=position->info.number-1;
              result=1;
            }
        }
      break;
    case RHVoice_position_sentence:
      n=toklist_size(message->tokens);
      for(i=0;i<n;i++)
        {
          if(toklist_at(message->tokens,i)->sentence_number==position->info.number)
            {
              message->pos=i;
              result=1;
              break;
            }
        }
      break;
    case RHVoice_position_mark:
      n=marklist_size(message->marks);
      for(i=0;i<n;i++)
        {
          m=marklist_at(message->marks,i);
          if(u8_strcmp(m->name,(const uint8_t*)position->info.name)==0)
            {
              message->pos=m->next_token_index;
              result=1;
              break;
            }
        }
      break;
    default:
      break;
    }
  return result;
}

int RHVoice_get_word_count(RHVoice_message message)
{
  return (message==NULL)?0:toklist_size(message->tokens);
}

int RHVoice_get_sentence_count(RHVoice_message message)
{
  if(message==NULL) return 0;
  token *t=toklist_back(message->tokens);
  return (t==NULL)?0:(t->sentence_number);
}
