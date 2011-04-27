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

#ifndef IO_UTILS_H
#define IO_UTILS_H
#include <stdio.h>
#include <unitypes.h>

#define UEOF 0xffffffffu

ucs4_t ufgetc(FILE *f,uint8_t *d,size_t *l);
#ifdef WIN32
ucs4_t ufgetca(FILE *f,uint8_t *d,size_t *l);
#endif
int skip_utf8_bom(FILE *f);
FILE *my_fopen(const char *path,const char *mode);
char *path_append(const char *path1,const char *path2);
typedef int (*file_action)(const char *path,void *user_data);
void for_each_file_in_dir(const char *dir_path,file_action action,void *user_data);
#endif
