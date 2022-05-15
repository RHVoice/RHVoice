/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the structied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RHVOICE_UTILS_H
#define RHVOICE_UTILS_H

#include <stdio.h>
#ifdef WIN32
#include <stdlib.h>
#include <windows.h>
#endif

  #ifdef _MSC_VER
__inline static int is_machine_big_endian()
  #else
  inline static int is_machine_big_endian()
  #endif
  {
    union
    {
      int i;
      char c[sizeof(int)];
    } u;
    u.i=1;
    return (u.c[0]==0);
  }

  #ifdef _MSC_VER
  __inline static int is_machine_little_endian()
  #else
  inline static int is_machine_little_endian()
  #endif
  {
    union
    {
      int i;
      char c[sizeof(int)];
    } u;
    u.i=1;
    return (u.c[0]==1);
  }

  #ifdef WIN32
  #ifdef _MSC_VER
  __inline static wchar_t* utf8_to_utf16(const char* str)
  #else
  inline static wchar_t* utf8_to_utf16(const char* str)
  #endif
  {
    wchar_t* wstr=NULL;
    int size=MultiByteToWideChar(CP_UTF8,0,str,-1,NULL,0);
    if(size==0)
      return NULL;
    wstr=(wchar_t*)malloc(size*sizeof(wchar_t));
    if(wstr==NULL)
      return NULL;
    if(MultiByteToWideChar(CP_UTF8,0,str,-1,wstr,size)==0)
  {
    free(wstr);
    return NULL;
  }
  else
    return wstr;
  }
  #endif

  #ifdef _MSC_VER
  __inline static FILE* utf8_fopen(const char* path,const char* mode)
  #else
  inline static FILE* utf8_fopen(const char* path,const char* mode)
  #endif
  {
  #ifdef WIN32
    FILE* fp=NULL;
    wchar_t* wpath=NULL;
    wchar_t* wmode=utf8_to_utf16(mode);
    if(wmode==NULL)
      return NULL;
    wpath=utf8_to_utf16(path);
    if(wpath==NULL)
      {
    free(wmode);
    return NULL;
  }
    fp=_wfopen(wpath,wmode);
    free(wmode);
    free(wpath);
    return fp;
  #else
  return fopen(path,mode);
  #endif
}
#endif
