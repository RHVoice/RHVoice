/* Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef RHVOICE_HTS_VOCODER_WRAPPER_HPP
#define RHVOICE_HTS_VOCODER_WRAPPER_HPP

#include <memory>
#include <vector>
#include <queue>
#include "pitch.hpp"

struct _HTS_Vocoder;
struct _HTS_Engine;

namespace RHVoice
{
  class hts_vocoder_wrapper
  {
  public:
    hts_vocoder_wrapper();
    hts_vocoder_wrapper(hts_vocoder_wrapper&)=delete;
    hts_vocoder_wrapper& operator=(hts_vocoder_wrapper&)=delete;
    ~hts_vocoder_wrapper();
    void init(_HTS_Engine*, pitch::editor*, double);
    void clear();
    void synth(std::size_t i, std::size_t n);
    void finish();

  private:

    void do_synth();

    struct frame_t
    {
      std::size_t index{0};
      bool voiced{false};
      double lf0{0};
      std::vector<double> spec, bap;
    };

    std::unique_ptr<_HTS_Vocoder> vocoder;
    _HTS_Engine* engine;
    pitch::editor* pitch_editor;
    std::size_t count{0};
    std::queue<frame_t> fq;
    double pitch_shift{0};
  };
}
#endif
