/* Copyright (C) 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
#include <unistr.h>
#include "tst.h"
#include "lib.h"
#include "ustring.h"
#include "vector.h"
#include "io_utils.h"

static const uint8_t space_delims[]={' ','\t','\0'};

static void free_pron(void *p)
{
  free(p);
}

struct user_dict_s
{
  tst words;
  ucs4_t stress_marker;
  int mark_next_vowel_as_stressed;
};

user_dict user_dict_create()
{
  user_dict dict=(user_dict)malloc(sizeof(struct user_dict_s));
  if(dict==NULL) goto err0;
  dict->words=tst_create(1,free_pron);
  if(dict->words==NULL) goto err1;
  dict->stress_marker='+';
  dict->mark_next_vowel_as_stressed=1;
  return dict;
  err1: free(dict);
  err0: return NULL;
}

void user_dict_free(user_dict dict)
{
  if(dict==NULL) return;
  tst_free(dict->words);
  free(dict);
}

static int check_user_pron_value(const user_dict d,const uint8_t *p)
{
  ucs4_t c;
  ucs4_t pc='\0';
  unsigned int cs;
  unsigned int pcs=0;
  const uint8_t *s=p+u8_strspn(p,space_delims);
  if(s[0]=='\0') return 0;
  while((s=u8_next(&c,s)))
    {
      cs=classify_character(c);
      if(!(((cs&cs_ru)&&(cs&cs_ll))||(c==d->stress_marker)||(c==' ')||(c=='\t'))) return 0;
      if(d->mark_next_vowel_as_stressed)
        {
          if((pc==d->stress_marker)&&!(cs&cs_lv)) return 0;
        }
      else
        {
          if((c==d->stress_marker)&&!(pcs&cs_lv)) return 0;
        }
      pc=c;
      pcs=cs;
    }
  return 1;
}

static uint8_t *copy_user_pron_value(user_dict d,const uint8_t *p)
{
  const uint8_t *s=p+u8_strspn(p,space_delims);
  const uint8_t *s1=s;
  const uint8_t *s2;
  ucs4_t c;
  size_t n=1;
  while((s2=u8_next(&c,s1)))
    {
      if((c!=' ')&&(c!='\t'))
        {
          if(c==d->stress_marker) n++;
          else n+=(s2-s1);
          if((s2[0]=='\0')||(s2[0]==' ')||(s2[0]=='\t')) n++;
        }
      s1=s2;
    }
  uint8_t *r=(uint8_t*)malloc(n);
  if(r==NULL) return NULL;
  size_t i=0;
  size_t l=0;
  s1=s;
  while((s2=u8_next(&c,s1)))
    {
      if((c!=' ')&&(c!='\t'))
        {
          if(c==d->stress_marker)
            {
              if(d->mark_next_vowel_as_stressed)
                r[i++]='+';
              else
                {
                  u8_move(&r[i-l+1],&r[i-l],l);
                  r[i-l]='+';
                  i++;
                }
            }
          else
            {
              l=s2-s1;
              u8_cpy(&r[i],s1,l);
              i+=l;
            }
          if((s2[0]=='\0')||(s2[0]==' ')||(s2[0]=='\t')) r[i++]='\0';
        }
      s1=s2;
    }
  r[n-1]='\0';
  return r;
}

static size_t get_user_pron_len(const uint8_t *pron)
{
  uint8_t c=pron[0];
  uint8_t pc;
  size_t n=0;
  do
    {
      pc=c;
      n++;
      c=pron[n];
    }
  while(!((c=='\0')&&(pc=='\0')));
  return (n+1);
}

static void add_word(user_dict dict,const uint8_t *key,const uint8_t *value)
{
  if(value==NULL) return;
  if(!check_user_pron_value(dict,value)) return;
  uint8_t *pron=copy_user_pron_value(dict,value);
  if(pron==NULL) return;
  if(!tst_add(dict->words,key,pron)) free(pron);
}

static int user_dict_callback(const uint8_t *section,const uint8_t *key,const uint8_t *value,void *user_data)
{
  user_dict dict=(user_dict)user_data;
  if(section==NULL)
    {
      if(value!=NULL)
        {
          if(u8_strcmp(key,(const uint8_t*)"stress_marker")==0)
            {
              u8_next(&dict->stress_marker,value);
            }
          else if(u8_strcmp(key,(const uint8_t*)"mark_next_vowel_as_stressed")==0)
            {
              if(strchr("tTyY1",(char)value[0]))
                dict->mark_next_vowel_as_stressed=1;
              else if(strchr("fFnN0",(char)value[0]))
                dict->mark_next_vowel_as_stressed=0;
            }
        }
    }
  else if(u8_strcmp(section,(const uint8_t*)"dict")==0)
      add_word(dict,key,value);
  return 1;
}

int user_dict_update(const char *path,void *data)
{
  user_dict dict=(user_dict)data;
  if(dict==NULL) return 0;
  parse_config(path,user_dict_callback,dict);
  dict->stress_marker='+';
  dict->mark_next_vowel_as_stressed=1;
  return 1;
}

void user_dict_build(user_dict dict)
{
  if(dict==NULL) return;
  tst_build(dict->words);
}

const uint8_t* user_dict_lookup8(const user_dict dict,const uint8_t* word)
{
  if(dict==NULL) return NULL;
  uint8_t *p=NULL;
  if(tst_search8(dict->words,word,(void**)&p))
    return p;
  else
    return NULL;
}

const uint8_t* user_dict_lookup32(const user_dict dict,const uint32_t* word)
{
  if(dict==NULL) return NULL;
  uint8_t *p=NULL;
  if(tst_search32(dict->words,word,(void**)&p))
    return p;
  else
    return NULL;
}

typedef struct
{
  uint32_t *prefix,*substring,*suffix;
  uint8_t *pron;
} rule;

void free_rule(rule *r)
{
  if(r==NULL) return;
  free(r->substring);
  if(r->prefix) free(r->prefix);
  if(r->suffix) free(r->suffix);
  free(r->pron);
}

vector_t(rule,ruleset)

typedef struct {
  char *name;
  user_dict dict;
  uint32_t *groups[26];
  ruleset rules;
} user_variant;

vector_t(user_variant,variantlist)

static void user_variant_free(user_variant *pv)
{
  size_t i;
  user_dict_free(pv->dict);
  for(i=0;i<26;i++)
    {
      if(pv->groups[i]!=NULL) free(pv->groups[i]);
    }
  ruleset_free(pv->rules);
}

static variantlist variants=NULL;

static int check_context_delims(const uint32_t *key)
{
  const uint32_t *l=u32_strchr(key,'<');
  if(l!=u32_strrchr(key,'<')) return 0;
  const uint32_t *r=u32_strchr(key,'>');
  if(r!=u32_strrchr(key,'>')) return 0;
  if(l&&r&&(l>r)) return 0;
  return 1;
}

static uint32_t *ustr_reverse(uint32_t *s)
{
  size_t n=u32_strlen(s);
  if(n<2) return s;
  n--;
  size_t m;
  uint32_t tmp;
  for(m=0;m<n;m++,n--)
    {
      tmp=s[m];
      s[m]=s[n];
      s[n]=tmp;
    }
  return s;
}

static void add_rule(user_variant *pv,const uint8_t *key,const uint8_t *value)
{
  if(value==NULL) return;
  if(!((value[0]=='\0')||check_user_pron_value(pv->dict,value))) return;
  size_t n=0;
  uint32_t *key0=u8_to_u32(key,u8_strlen(key)+1,NULL,&n);
  if(key0==NULL) goto err0;
  if(!check_context_delims(key0)) goto err1;
  rule r;
  r.substring=NULL;
  r.prefix=NULL;
  r.suffix=NULL;
  r.pron=NULL;
  uint32_t *s1=u32_strchr(key0,'<');
  if(s1!=NULL)
    {
      s1[0]='\0';
      s1++;
      r.prefix=u32_strdup(key0);
      if(r.prefix==NULL) goto err1;
      r.prefix=ustr_reverse(r.prefix);
    }
  else
    s1=key0;
  uint32_t *s2=u32_strrchr(s1,'>');
  if(s2!=NULL)
    {
      s2[0]='\0';
      s2++;
      r.suffix=u32_strdup(s2);
      if(r.suffix==NULL) goto err2;
    }
  if(s1[0]=='\0') goto err3;
  r.substring=u32_strdup(s1);
  if(r.substring==NULL) goto err3;
  r.pron=copy_user_pron_value(pv->dict,value);
  if(r.pron==NULL) goto err4;
  if(!ruleset_push(pv->rules,&r)) goto err5;
  free(key0);
  return;
  err5: if(r.pron) free(r.pron);
  err4: if(r.substring) free(r.substring);
  err3: if(r.suffix!=NULL) free(r.suffix);
  err2: if(r.prefix!=NULL) free(r.prefix);
  err1: free(key0);
  err0: return;
}

static void add_group(user_variant *pv,const uint8_t *key,const uint8_t *value)
{
  if(key[1]!='\0') return;
  if((key[0]<'A')||(key[0]>'Z')) return;
  size_t k=key[0]-'A';
  if(value==NULL) return;
  if(value[0]=='\0') return;
  size_t n=0;
  if(pv->groups[k]!=NULL) free(pv->groups[k]);
  pv->groups[k]=u8_to_u32(value,u8_strlen(value)+1,NULL,&n);
}

static int user_variant_is_in_group(user_variant *pv,ucs4_t c,uint8_t n)
{
  if(c=='\0') return 0;
  const uint32_t *g=pv->groups[n-'A'];
  if(g==NULL) return 0;
  return (u32_strchr(g,c)!=NULL);
}

int user_variant_is_member(int v,ucs4_t c,uint8_t g)
{
  if(v>user_variant_get_count()) return 0;
  if((g<'A')||(g>'Z')) return 0;
  return user_variant_is_in_group(variantlist_at(variants,v-1),c,g);
}

static int variant_callback(const uint8_t *section,const uint8_t *key,const uint8_t *value,void *user_data)
{
  user_variant *pv=(user_variant*)user_data;
  if(section==NULL)
    {
      if(u8_strcmp(key,(const uint8_t*)"name")==0)
        {
          if((value!=NULL)&&(value[0]!='\0'))
            {
              if(pv->name!=NULL) free(pv->name);
              pv->name=(char*)u8_strdup(value);
              if(pv->name==NULL) return 0;
            }
        }
      else return user_dict_callback(section,key,value,pv->dict);
    }
  else
    {
      if(pv->name==NULL) return 0;
      if(u8_strcmp(section,(const uint8_t*)"rules")==0)
        add_rule(pv,key,value);
      else if(u8_strcmp(section,(const uint8_t*)"groups")==0)
        add_group(pv,key,value);
      else return user_dict_callback(section,key,value,pv->dict);
    }
  return 1;
}

static int load_user_variant(const char *path,void *data)
{
  user_variant v;
  v.name=NULL;
  size_t i;
  for(i=0;i<26;i++)
    {
      v.groups[i]=NULL;
    }
  v.dict=user_dict_create();
  if(v.dict==NULL) goto err1;
  v.rules=ruleset_alloc(0,free_rule);
  if(v.rules==NULL) goto err2;
  parse_config(path,variant_callback,&v);
  if(v.name==NULL) goto err3;
  user_dict_build(v.dict);
  if(!variantlist_push(variants,&v)) goto err3;
  return 1;
  err3:
  if(v.name!=NULL) free(v.name);
  ruleset_free(v.rules);
  err2: user_dict_free(v.dict);
  err1: return 0;
}

void load_user_variants(const char *dir)
{
  variants=variantlist_alloc(0,user_variant_free);
  if(variants==NULL) return;
  for_each_file_in_dir(dir,load_user_variant,NULL);
}

void free_user_variants()
{
  variantlist_free(variants);
  variants=NULL;
}

int user_variant_get_count()
{
  return variantlist_size(variants);
}

const char *user_variant_get_name(int v)
{
  if(v>user_variant_get_count()) return NULL;
  return variantlist_at(variants,v-1)->name;
}

int user_variant_is_alpha(int v,ucs4_t c)
{
  if(v>user_variant_get_count()) return 0;
  if(((c>='0')&&(c<='9'))||((c>='A')&&(c<='Z'))||(c=='$')||(c=='<')||(c=='>')) return 0;
  return (user_variant_is_member(v,c,'V')||
          user_variant_is_member(v,c,'C')||
          user_variant_is_member(v,c,'O'));
}

const uint8_t *user_variant_lookup(int v,const uint32_t *word)
{
  if(v>user_variant_get_count()) return NULL;
  return user_dict_lookup32(variantlist_at(variants,v-1)->dict,word);
}

static int context_matches(user_variant *pv,const uint32_t *str,const uint32_t *pattern)
{
  if(pattern==NULL) return 1;
  const uint32_t *p=pattern;
  const uint32_t *s=str;
  ucs4_t c,pc,c0;
  size_t l,n;
  const uint32_t *s0;
  while((p=u32_next(&pc,p)))
    {
      c=*s;
      if(pc=='$')
        {
          if(p[0]=='\0')
            {
              if(c!='\0') return 0;
              else break;
            }
          else if(p[0]=='$')
            {
              p=u32_next(&pc,p);
              if(c!=pc) return 0;
            }
          else return 0;
        }
      else if((pc>='0')&&(pc<='9')&&((p[0]=='\0')||((p[0]=='$')&&(p[1]=='\0'))))
        {
          l=pc-'0';
          n=0;
          s0=s;
          while((s0=u32_next(&c0,s0)))
            {
              if(user_variant_is_in_group(pv,c0,'V')) n++;
            }
          if((p[0]=='$')&&(n!=l)) return 0;
          else if((p[0]=='\0')&&(n<l)) return 0;
          else break;
        }
      else if(c=='\0') return 0;
      else if((pc>='A')&&(pc<='Z'))
            {
              if(!user_variant_is_in_group(pv,c,pc)) return 0;
              s++;
            }
      else
        {
          if(c!=pc) return 0;
          s++;
        }
    }
  return 1;
}

static int rule_matches(user_variant *pv,const uint32_t *left,const uint32_t *right,rule *r)
{
  if(!u32_startswith(right,r->substring)) return 0;
  return (context_matches(pv,left,r->prefix)&&context_matches(pv,right+u32_strlen(r->substring),r->suffix));
}

static rule *find_rule(user_variant *pv,const uint32_t *left,const uint32_t *right)
{
  rule *r;
  size_t n=ruleset_size(pv->rules);
  size_t i;
  for(i=0;i<n;i++)
    {
      r=ruleset_at(pv->rules,i);
      if(rule_matches(pv,left,right,r)) return r;
    }
  return NULL;
}

ustring8_t user_variant_apply(int v,const uint32_t *word)
{
  if(v>user_variant_get_count()) return NULL;
  user_variant *pv=variantlist_at(variants,v-1);
  uint32_t *rword=u32_strdup(word);
  if(rword==NULL) return NULL;
  ustr_reverse(rword);
  ustring8_t result=ustring8_alloc(0);
  if(result==NULL)
    {
      free(rword);
      return NULL;
    }
  size_t n=u32_strlen(word);
  const uint32_t *right=word;
  const uint32_t *left=rword+n;
  size_t l;
  rule *r;
  while((right[0]!='\0'))
    {
      r=find_rule(pv,left,right);
      if(r==NULL)
        {
          right++;
          left--;
          continue;
        }
      l=u32_strlen(r->substring);
      if(r->pron[0]!='\0')
        {
          if(!ustring8_append(result,r->pron,u8_strlen(r->pron)))
            {
              ustring8_free(result);
              free(rword);
              return NULL;
            }
        }
      right+=l;
      left-=l;
    }
  free(rword);
  if(ustring8_empty(result)||(!ustring8_push(result,'\0')))
    {
      ustring8_free(result);
      return NULL;
    }
  else
    return result;
}
