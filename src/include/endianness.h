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

#ifndef RHVOICE_ENDIANNESS_H
#define RHVOICE_ENDIANNESS_H

#ifdef __cplusplus
extern "C" {
#endif

  #ifdef _MSC_VER
__inline int is_machine_big_endian()
  #else
  inline int is_machine_big_endian()
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
  __inline int is_machine_little_endian()
  #else
  inline int is_machine_little_endian()
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

#ifdef __cplusplus
}
#endif
#endif
