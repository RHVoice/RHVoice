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
#include "io_utils.h"
#include <stdio.h>
#include <unistr.h>

typedef ucs4_t(*ufgetc_func)(FILE*,uint8_t*,size_t*);

static int next_line(FILE *fp,ufgetc_func f,ustring8_t s)
{
  ucs4_t c;
  uint8_t b[4];
  size_t n;
  while((c=f(fp,b,&n))!=UEOF)
    {
      if((c!='\n')&&(c!='\r')) break;
    }
  if(c==UEOF) return 0;
  if(!ustring8_assign(s,b,n)) return 0;
  while((c=f(fp,b,&n))!=UEOF)
    {
      if((c=='\n')||(c=='\r')) break;
      if(!ustring8_append(s,b,n)) return 0;
    }
  return 1;
}

static const uint8_t *find_closing_quote(const uint8_t *s)
{
  const uint8_t *p=NULL;
  size_t n=0;
  const uint8_t *e=s+1;
  if(*e=='\"') return e;
  else if(*e=='\0') return NULL;
  else
    {
      while((e=u8_strchr(e+1,'\"')))
        {
          for(p=e-1,n=0;(p>s)&&(*p=='\\');p--,n++);
          if(n%2==0) break;
        }
      return e;
    }
}

static const uint8_t *special_chars=(const uint8_t*)"\\\"nrtuUx";
static const uint8_t *hex_chars=(const uint8_t*)"0123456789abcdefABCDEF";

static int expand_escapes(const uint8_t *s,size_t n,ustring8_t u)
{
  const uint8_t *s1=s;
  const uint8_t *s2=NULL;
  size_t k;
  ucs4_t c;
  char b[10];
  char *p;
  ustring8_clear(u);
  while((s2=u8_chr(s1,n-(s1-s),'\\')))
    {
      if((s2-s+1)==n) break;
      if(u8_strchr(special_chars,s2[1])==NULL)
        {
          s1=s2+1;
          continue;
        }
      if(s2!=s1)
        {
          if(!ustring8_append(u,s1,s2-s1)) return 0;
        }
      s2++;
      k=0;
      if(s2[0]=='n') c='\n';
      else if(s2[0]=='r') c='\r';
      else if(s2[0]=='t') c='\t';
      else if(s2[0]=='\\') c='\\';
      else if(s2[0]=='\"') c='\"';
      else
        {
          if(s2[0]=='x') k=2;
          else if(s2[0]=='u') k=4;
          else k=8;
          if((s2+1-s+k)>n) return 0;
          if(u8_strchr(hex_chars,s2[1])==NULL) return 0;
          u8_cpy((uint8_t*)b,s2+1,k);
          b[k]='\0';
          c=strtoul(b,&p,16);
          if(*p!='\0') return 0;
          if(u32_check(&c,1)!=NULL) return 0;
        }
      if(!ustring8_push(u,c)) return 0;
      s1=s2+1+k;
      if((s1-s)==n) break;
    }
  if((s1-s)<n)
    {
      if(!ustring8_append(u,s1,n-(s1-s))) return 0;
    }
  return 1;
}

void parse_config(const char *path,config_callback callback,void *user_data)
{
  FILE *fp=my_fopen(path,"rb");
  if(fp==NULL) goto err0;
  ustring8_t line=ustring8_alloc(0);
  if(line==NULL) goto err1;
  ustring8_t section=ustring8_alloc(0);
  if(section==NULL) goto err2;
  ustring8_t key=ustring8_alloc(0);
  if(key==NULL) goto err3;
  ustring8_t value=ustring8_alloc(0);
  if(value==NULL) goto err4;
  const uint8_t *s=NULL;
  const uint8_t *e=NULL;
  size_t n=0;
  const uint8_t *strsection=NULL;
  const uint8_t *strkey=NULL;
  const uint8_t *strvalue=NULL;
  ufgetc_func f;
#ifdef WIN32
  if(skip_utf8_bom(fp))
    f=ufgetc;
  else
    f=ufgetca;
#else
  skip_utf8_bom(fp);
  f=ufgetc;
#endif
  while(next_line(fp,f,line))
    {
      s=ustring8_str(line);
      n=u8_strspn(s,(const uint8_t*)" \t");
      s+=n;
      if(n==ustring8_length(line)) continue;
      else if(s[0]=='=') break;
      else if((s[0]==';')||(s[0]=='#')) continue;
      else if(s[0]=='[')
        {
          e=u8_strchr(s,']');
          if((e==NULL)||(e==(s+1))) break;
          if(!ustring8_assign(section,s+1,e-s-1)) break;
          strsection=ustring8_str(section);
        }
      else
        {
          if(s[0]=='\"')
            {
              e=find_closing_quote(s);
              if((e==NULL)||(e==(s+1))) break;
              s++;
              n=e-s;
              e++;
            }
          else
            {
              n=u8_strcspn(s,(const uint8_t*)" \t=");
              e=s+n;
            }
          if(!expand_escapes(s,n,key)) break;
          strkey=ustring8_str(key);
          s=e+u8_strspn(e,(const uint8_t*)" \t");
          if(s[0]=='\0') strvalue=NULL;
          else if(s[0]!='=') break;
          else
            {
              s++;
              s+=u8_strspn(s,(const uint8_t*)" \t");
              ustring8_clear(value);
              if(!((s[0]=='\0')||((s[0]=='\"')&&(s[1]=='\"'))))
                {
                  if(s[0]=='\"')
                    {
                      e=find_closing_quote(s);
                      if(e==NULL) break;
                      s++;
                      n=e-s;
                    }
                  else
                    {
                      n=u8_strcspn(s,(const uint8_t*)" \t");
                    }
                  if(!expand_escapes(s,n,value)) break;
                }
              strvalue=ustring8_str(value);
            }
          if(!callback(strsection,strkey,strvalue,user_data)) break;
        }
    }
  ustring8_free(value);
  err4: ustring8_free(key);
  err3: ustring8_free(section);
  err2: ustring8_free(line);
  err1: fclose(fp);
  err0: return;
}
