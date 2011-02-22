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

#include "io_utils.h"
#include <stdlib.h>
#include <unistr.h>

ucs4_t ufgetc(FILE *f,uint8_t *d,size_t *l)
{
  int n;
  size_t i;
  int c;
  ucs4_t uc;
  uint8_t b[4];
  for(i=0;i<4;i++)
    {
      c=getc(f);
      if(c==EOF) return UEOF;
      b[i]=c;
      n=u8_mbtoucr(&uc,b,i+1);
      if(n==-1) return UEOF;
      if(n>0)
        {
          if(l!=NULL) *l=n;
          if(d!=NULL) u8_cpy(d,b,n);
          return uc;
        }
    }
  return UEOF;
}

FILE *my_fopen(const char *path,const char *mode)
{
#ifdef WIN32
  size_t size=0;
  const uint8_t *path8=(const uint8_t*)path;
  const uint8_t *mode8=(const uint8_t*)mode;
  uint16_t *path16=u8_to_u16(path8,u8_strlen(path8)+1,NULL,&size);
  if(path16==NULL) return NULL;
  size=0;
  uint16_t *mode16=u8_to_u16(mode8,u8_strlen(mode8)+1,NULL,&size);
  if(mode16==NULL)
    {
      free(path16);
      return NULL;
    }
  FILE *result=_wfopen(path16,mode16);
  free(mode16);
  free(path16);
  return result;
#else
  return fopen(path,mode);
#endif
}
