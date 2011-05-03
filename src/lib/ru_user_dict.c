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
  uint32_t *substring;
  ustring32_t prefix;
  ustring32_t suffix;
  uint8_t *pron;
} rule;

void free_rule(rule *r)
{
  if(r==NULL) return;
  free(r->substring);
  if(r->prefix) ustring32_free(r->prefix);
  if(r->suffix) ustring32_free(r->suffix);
  free(r->pron);
}

vector_t(rule,ruleset)

typedef struct
{
  uint8_t *name;
  uint32_t *members;
} group;

void free_group(group *g)
{
  if(g->name!=NULL) free(g->name);
  if(g->members!=NULL) free(g->members);
}

vector_t(group,grouplist)

typedef struct {
  char *name;
  user_dict dict;
  ustring32_t alphabet;
  grouplist groups;
  ruleset rules;
} user_variant;

vector_t(user_variant,variantlist)

static void user_variant_free(user_variant *pv)
{
  user_dict_free(pv->dict);
  ustring32_free(pv->alphabet);
  ruleset_free(pv->rules);
}

static variantlist variants=NULL;

static int check_context_delims(const uint8_t *key)
{
  const uint8_t *l=u8_strchr(key,'<');
  if(l!=u8_strrchr(key,'<')) return 0;
  const uint8_t *r=u8_strchr(key,'>');
  if(r!=u8_strrchr(key,'>')) return 0;
  if(l&&r&&(l>r)) return 0;
  return 1;
}

static ustring32_t parse_context_pattern(user_variant *pv,const uint8_t *pattern)
{
  ustring32_t result=ustring32_alloc(0);
  if(result==NULL) return NULL;
  ucs4_t c;
  const uint8_t *s=pattern;
  const uint8_t *p=NULL;
  size_t n=grouplist_size(pv->groups);
  size_t i;
  const uint8_t *gname=NULL;
  while((s=u8_next(&c,s)))
    {
      if(c=='{')
        {
          p=u8_strchr(s,'}');
          if((p==NULL)||(p==s))
            {
              ustring32_free(result);
              return NULL;
            }
          gname=NULL;
          for(i=0;i<n;i++)
            {
              gname=grouplist_at(pv->groups,i)->name;
              if(u8_cmp2(s,p-s,gname,u8_strlen(gname))==0)
                {
                  c=0x10ffff+1+i;
                  break;
                }
              else
                gname=NULL;
            }
          if(gname==NULL)
            {
              ustring32_free(result);
              return NULL;
            }
          s=p+1;
        }
      if(!ustring32_push(result,c))
        {
          ustring32_free(result);
          return NULL;
        }
    }
  return result;
}

static void add_rule(user_variant *pv,const uint8_t *key,const uint8_t *value)
{
  if(value==NULL) return;
  if(!check_context_delims(key)) return;
  if(!((value[0]=='\0')||check_user_pron_value(pv->dict,value))) return;
  size_t n=0;
  uint8_t *key0=u8_strdup(key);
  if(key0==NULL) goto err0;
  rule r;
  r.substring=NULL;
  r.prefix=NULL;
  r.suffix=NULL;
  r.pron=NULL;
  uint8_t *s1=u8_strchr(key0,'<');
  if(s1!=NULL)
    {
      s1[0]='\0';
      s1++;
      r.prefix=parse_context_pattern(pv,key0);
      if(r.prefix==NULL) goto err1;
    }
  else
    s1=key0;
  uint8_t *s2=u8_strrchr(s1,'>');
  if(s2!=NULL)
    {
      s2[0]='\0';
      s2++;
      r.suffix=parse_context_pattern(pv,s2);
      if(r.suffix==NULL) goto err2;
    }
  if(s1[0]=='\0') goto err3;
  r.substring=u8_to_u32(s1,u8_strlen(s1)+1,NULL,&n);
  if(r.substring==NULL) goto err3;
  r.pron=copy_user_pron_value(pv->dict,value);
  if(r.pron==NULL) goto err4;
  ucs4_t c;
  const uint8_t *s=s1;
  while((s=u8_next(&c,s)))
    {
      if(ustring32_empty(pv->alphabet)||(u32_chr(ustring32_str(pv->alphabet),ustring32_length(pv->alphabet),c)==NULL))
        ustring32_push(pv->alphabet,c);
    }
  if(!ruleset_push(pv->rules,&r)) goto err5;
  free(key0);
  return;
  err5: if(r.pron) free(r.pron);
  err4: if(r.substring) free(r.substring);
  err3: if(r.suffix!=NULL) ustring32_free(r.suffix);
  err2: if(r.prefix!=NULL) ustring32_free(r.prefix);
  err1: free(key0);
  err0: return;
}

static void add_group(user_variant *pv,const uint8_t *key,const uint8_t *value)
{
  if(value==NULL) return;
  if(value[0]=='\0') return;
  group g;
  g.name=u8_strdup(key);
  if(g.name==NULL) return;
  size_t n=0;
  g.members=u8_to_u32(value,u8_strlen(value+1),NULL,&n);
  if(g.members==NULL)
    {
      free(g.name);
      return;
    }
  if(!grouplist_push(pv->groups,&g))
    {
      free(g.name);
      free(g.members);
    }
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
  else if(u8_strcmp(section,(const uint8_t*)"rules")==0)
    add_rule(pv,key,value);
  else if(u8_strcmp(section,(const uint8_t*)"groups")==0)
    add_group(pv,key,value);
  else return user_dict_callback(section,key,value,pv->dict);
  return 1;
}

static int load_user_variant(const char *path,void *data)
{
  user_variant v;
  v.name=NULL;
  v.dict=user_dict_create();
  if(v.dict==NULL) goto err0;
  v.alphabet=ustring32_alloc(40);
  if(v.alphabet==NULL) goto err1;
  v.rules=ruleset_alloc(0,free_rule);
  if(v.rules==NULL) goto err2;
  v.groups=grouplist_alloc(0,free_group);
  if(v.groups==NULL) goto err3;
  parse_config(path,variant_callback,&v);
  if((v.name==NULL)||ustring32_empty(v.alphabet)) goto err3;
  user_dict_build(v.dict);
  if(!variantlist_push(variants,&v)) goto err4;
  return 1;
  err4: grouplist_free(v.groups);
  err3:
  if(v.name!=NULL) free(v.name);
  ruleset_free(v.rules);
  err2: ustring32_free(v.alphabet);
  err1: user_dict_free(v.dict);
  err0: return 0;
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
  ustring32_t a=variantlist_at(variants,v-1)->alphabet;
  return (u32_chr(ustring32_str(a),ustring32_length(a),c)!=NULL);
}

const uint8_t *user_variant_lookup(int v,const uint32_t *word)
{
  if(v>user_variant_get_count()) return NULL;
  return user_dict_lookup32(variantlist_at(variants,v-1)->dict,word);
}

static int matches(user_variant *pv,const uint32_t *w,size_t p,rule *r)
{
  if(!u32_startswith(w+p,r->substring)) return 0;
  size_t e=p+u32_strlen(r->substring);
  group *g=grouplist_data(pv->groups);
  ucs4_t c1,c2;
  const uint32_t *s1,*s2;
  if(r->prefix!=NULL)
    {
      const uint32_t *prefix_pattern=ustring32_str(r->prefix);
      s1=w+p;
      s2=prefix_pattern+ustring32_length(r->prefix);
      while((s2=u32_prev(&c2,s2,prefix_pattern)))
        {
          if((c2=='$')&&(s2==prefix_pattern))
            {
              if(s1==w)
                break;
              else
                return 0;
            }
          if(s1==w) return 0;
          s1--;
          c1=*s1;
          if(c2>0x10ffff)
            {
              if(!u32_strchr(g[c2-0x10ffff-1].members,c1)) return 0;
            }
          else if(c1!=c2) return 0;
        }
    }
  if(r->suffix!=NULL)
    {
      s1=w+e;
      s2=ustring32_str(r->suffix);
      while((s2=u32_next(&c2,s2)))
        {
          if((c2=='$')&&(s2[0]=='\0'))
            {
              if(s1[0]=='\0')
                break;
              else
                return 0;
            }
          if(s1[0]=='\0') return 0;
          c1=*s1;
          s1++;
          if(c2>0x10ffff)
            {
              if(!u32_strchr(g[c2-0x10ffff-1].members,c1)) return 0;
            }
          else if(c1!=c2) return 0;
        }
    }
  return 1;
}

static rule *find_rule(user_variant *pv,const uint32_t *w,size_t p)
{
  rule *r;
  size_t n=ruleset_size(pv->rules);
  size_t i;
  for(i=0;i<n;i++)
    {
      r=ruleset_at(pv->rules,i);
      if(matches(pv,w,p,r)) return r;
    }
  return NULL;
}

ustring8_t user_variant_apply(int v,const uint32_t *word)
{
  if(v>user_variant_get_count()) return NULL;
  user_variant *pv=variantlist_at(variants,v-1);
  ustring8_t result=ustring8_alloc(0);
  if(result==NULL) return NULL;
  size_t n=u32_strlen(word);
  size_t p=0;
  rule *r;
  while(p<n)
    {
      r=find_rule(pv,word,p);
      if(r==NULL)
        {
          p++;
          continue;
        }
      p+=u32_strlen(r->substring);
      if(r->pron[0]!='\0')
        {
          if(!ustring8_append(result,r->pron,u8_strlen(r->pron)))
            {
              ustring8_free(result);
              return NULL;
            }
        }
    }
  if(ustring8_empty(result)||(!ustring8_push(result,'\0')))
    {
      ustring8_free(result);
      return NULL;
    }
  else
    return result;
}
