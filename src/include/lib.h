/* Copyright (C) 2010  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#ifndef LIB_H
#define LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <flite.h>

  typedef int (*RHVoice_callback)(const short *samples,int nsamples,cst_item *seg,int pos_in_seg);

  cst_voice *RHVoice_create_voice(const char *voxdir,RHVoice_callback callback);
  void RHVoice_delete_voice(cst_voice *vox);
  void RHVoice_synth_text(const char *text,cst_voice *vox);
  int RHVoice_load_user_dict(cst_voice *vox,const char *path);

#ifdef __cplusplus
}
#endif

#endif
