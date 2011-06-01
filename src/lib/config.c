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

static int expand_escapes(uint8_t *s,size_t n,ustring8_t u)
{
  uint8_t *s1=s;
  uint8_t *s2=NULL;
  size_t k=n;
  int l=0;
  ucs4_t c;
  char *p;
  ustring8_clear(u);
  while(k>0)
    {
      l=u8_mbtoucr(&c,s1,k);
      if(l<=0) return 0;
      if(c=='#')
        {
          s2=u8_chr(s1,k,';');
          if((s2==NULL)||(s2-s1==1))
            return 0;
          *s2='\0';
          c=strtoul((const char*)((s1[1]=='x')?(s1+2):(s1+1)),&p,((s1[1]=='x')?16:10));
          *s2=';';
          if(*p!=';') return 0;
          if(u32_check(&c,1)!=NULL) return 0;
          k-=(s2-s1+1);
          s1=s2+1;
        }
      else
        {
          k-=l;
          s1+=l;
        }
      if(!ustring8_push(u,c))
        return 0;
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
  uint8_t *s=NULL;
  uint8_t *e=NULL;
  uint8_t *d=NULL;
  uint8_t *strsection=NULL;
  uint8_t *strkey=NULL;
  uint8_t *strvalue=NULL;
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
      s+=u8_strspn(s,(const uint8_t*)" \t");
      if(s[0]=='\0') continue;
      else if(s[0]=='=') break;
      else if(s[0]==';') continue;
      else if(s[0]=='[')
        {
          e=u8_strchr(s,']');
          if((e==NULL)||(e==(s+1))) break;
          if(!ustring8_assign(section,s+1,e-s-1)) break;
          strsection=ustring8_str(section);
        }
      else
        {
          d=u8_strchr(s,'=');
          if(d==NULL) break;
          for(e=d-1;(e>s)&&((*e==' ')||(*e=='\t'));e--);
          e++;
          if(!expand_escapes(s,e-s,key)) break;
          strkey=ustring8_str(key);
          s=d+1+u8_strspn(d+1,(const uint8_t*)" \t");
          if(s[0]=='\0')
            strvalue=NULL;
          else
            {
              ustring8_clear(value);
              for(e=s+u8_strlen(s)-1;(e>s)&&((*e==' ')||(*e=='\t'));e--);
              e++;
              if(!expand_escapes(s,e-s,value)) break;
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
