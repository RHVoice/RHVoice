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
#include <unistdio.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef WIN32

static int cp1251_table[64]={
  1026,
  1027,
  8218,
  1107,
  8222,
  8230,
  8224,
  8225,
  8364,
  8240,
  1033,
  8249,
  1034,
  1036,
  1035,
  1039,
  1106,
  8216,
  8217,
  8220,
  8221,
  8226,
  8211,
  8212,
  -1,
  8482,
  1113,
  8250,
  1114,
  1116,
  1115,
  1119,
  160,
  1038,
  1118,
  1032,
  164,
  1168,
  166,
  167,
  1025,
  169,
  1028,
  171,
  172,
  173,
  174,
  1031,
  176,
  177,
  1030,
  1110,
  1169,
  181,
  182,
  183,
  1105,
  8470,
  1108,
  187,
  1112,
  1029,
  1109,
  1111};

ucs4_t ufgetca(FILE *f,uint8_t *d,size_t *l)
{
  int n;
  int C;
  unsigned char c;
  ucs4_t uc;
  uint8_t b[4];
  C=getc(f);
  if((C==EOF)||(C==0)) return UEOF;
  c=C;
  if(c<128) uc=c;
  else if(c>=192) uc=c+848;
  else if(c==152) return UEOF;
  else uc=cp1251_table[c-128];
  if(l!=NULL)
    {
      n=u8_uctomb(b,uc,4);
      if(d!=NULL) u8_cpy(d,b,n);
      *l=n;
    }
  return uc;
}
#endif
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
          if(uc==0) return UEOF;
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

int skip_utf8_bom(FILE *f)
{
  if(ftell(f)!=0) return 0;
  unsigned char b[3];
  size_t n=fread(b,3,1,f);
  if((n==1)&&(b[0]==0xef)&&(b[1]==0xbb)&&(b[2]==0xbf))
    return 1;
  else
    {
      rewind(f);
      return 0;
    }
}

char *path_append(const char *path1,const char *path2)
{
  if((path1==NULL)||(path2==NULL)||(path1[0]=='\0')||(path2[0]=='\0')) return NULL;
  uint8_t *result=NULL;
  int n=0;
  if(u8_endswith((const uint8_t*)path1,(const uint8_t*)path_sep))
    n=u8_asprintf(&result,"%U%U",(const uint8_t*)path1,(const uint8_t*)path2);
  else
    n=u8_asprintf(&result,"%U%U%U",(const uint8_t*)path1,(const uint8_t*)path_sep,(const uint8_t*)path2);
  return (n>0)?((char*)result):NULL;
}

#ifdef WIN32

void for_each_file_in_dir(const char *dir_path,file_action action,void *user_data)
{
  if((dir_path==NULL)||(dir_path[0]=='\0')) return;
  size_t n=0;
  uint16_t *dir_wpath=u8_to_u16((const uint8_t*)dir_path,u8_strlen((const uint8_t*)dir_path)+1,NULL,&n);
  if(dir_wpath==NULL) return;
  _WDIR *d=_wopendir((const wchar_t*)dir_wpath);
  if(d==NULL)
    {
      free(dir_wpath);
      return;
    }
  struct _wdirent *e=NULL;
  struct _stat s;
  uint8_t *full_path=NULL;
  uint16_t *full_wpath=NULL;
  int r=1;
  const char *fmt=(dir_wpath[n-2]=='\\')?"%lU%lU":"%lU\\%lU";
  while((e=_wreaddir(d)))
    {
      if(u16_asprintf(&full_wpath,fmt,dir_wpath,(const uint16_t*)e->d_name)<=0) break;
      if(_wstat((const wchar_t*)full_wpath,&s)==0)
        {
          if(S_ISREG(s.st_mode))
            {
              full_path=u16_to_u8(full_wpath,u16_strlen(full_wpath)+1,NULL,&n);
              if(full_path!=NULL)
                {
                  r=action((const char*)full_path,user_data);
                  free(full_path);
                  full_path=NULL;
                }
            }
        }
      free(full_wpath);
      full_wpath=NULL;
      if(r==0) break;
    }
  _wclosedir(d);
  free(dir_wpath);
}

#else

void for_each_file_in_dir(const char *dir_path,file_action action,void *user_data)
{
  if((dir_path==NULL)||(dir_path[0]=='\0')) return;
  DIR *d=opendir(dir_path);
  if(d==NULL) return;
  struct dirent *e=NULL;
  struct stat s;
  char *full_path=NULL;
  int r=1;
  while((e=readdir(d)))
    {
      full_path=path_append(dir_path,e->d_name);
      if(full_path==NULL) break;
      if(stat(full_path,&s)==0)
        {
          if(S_ISREG(s.st_mode))
            {
              r=action(full_path,user_data);
            }
        }
      free(full_path);
      full_path=NULL;
      if(r==0) break;
    }
  closedir(d);
}

#endif
