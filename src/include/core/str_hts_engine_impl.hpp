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

#ifndef RHVOICE_STR_HTS_ENGINE_IMPL_HPP
#define RHVOICE_STR_HTS_ENGINE_IMPL_HPP

#include <memory>
#include <queue>
#include <vector>
#include <array>
#include "hts_engine_impl.hpp"
#include "quality_setting.hpp"
#include "hts_vocoder_wrapper.hpp"

struct _HTS_Engine;
struct _HTS_Model;

namespace RHVoice
{
  class voice_info;

  class str_hts_engine_impl: public hts_engine_impl
  {
  public:
    explicit str_hts_engine_impl(const voice_info& info);
    ~str_hts_engine_impl();

    virtual bool supports_quality(quality_t q) const;

  private:
    using par_vec_t=std::vector<double>;
    using par_mat_t=std::vector<par_vec_t>;
    using hts_par_mat_view_t=std::vector<double*>;

struct par_mat_mem_t
{
  par_mat_t data;
  hts_par_mat_view_t hts_view;
};

    using par_mem_t=std::array<par_mat_mem_t, 3>;

    pointer do_create() const;
    void do_initialize();
    void do_reset();
    void do_synthesize();
    void do_stop();
    void queue_labels();
    bool fill_lab_view();
    void set_speed();
    void set_frame_ranges();
    void set_label_timing();
    void save_params();
    void restore_params();
    void check_units();
    void copy_units();
    void maybe_patch_model(_HTS_Model* mod, const std::string& patch_path);

    std::unique_ptr<_HTS_Engine> engine;
    hts_vocoder_wrapper vocoder;
    std::size_t base_frame_shift;
    std::queue<const hts_label*> lab_queue;
    std::vector<char*> lab_view;
    std::vector<double> dur_mods;
    std::size_t view_size{3};
    std::size_t fixed_size{1};
    par_mem_t par_mem;
    bool first_iter{true};
    std::size_t first_frame{0};
    std::size_t first_frame_in_utt{0};
    std::size_t num_frames{0};
    std::size_t num_mem_frames{0};
    std::size_t num_voiced_frames{0};
    std::size_t num_voiced_mem_frames{0};
    bool units_flag{false};
  };
}
#endif
