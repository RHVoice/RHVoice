/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_BPF_H
#define RHVOICE_BPF_H
#ifdef __cplusplus
extern "C" {
  #endif

typedef struct
{
  unsigned int number;
  unsigned int length;
  double** coefs;
} BPF;

  void bpf_init(BPF* bpf);
  void bpf_clear(BPF* bpf);
  int bpf_load(BPF* bpf,const char* path);
#ifdef __cplusplus
}
#endif
#endif
