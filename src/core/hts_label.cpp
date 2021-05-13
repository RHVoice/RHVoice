/* Copyright (C) 2013, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "core/voice.hpp"
#include "core/hts_label.hpp"

namespace RHVoice
{
  double hts_label::calculate_speech_param(double absolute_change,double relative_change,const numeric_property<double>& default_setting,const numeric_property<double>& min_setting,const numeric_property<double>& max_setting,bool clip) const
  {
    double default_value=default_setting;
    double min_value=min_setting;
    double max_value=max_setting;
    if(!(min_value<=max_value))
      return 1;
    if(default_value>max_value)
      default_value=max_value;
    else if(default_value<min_value)
      default_value=min_value;
    double result=default_value;
    if(absolute_change>0)
      {
        if(absolute_change>=1)
          result=max_value;
        else
          result+=absolute_change*(max_value-default_value);
      }
    else if(absolute_change<0)
      {
        if(absolute_change<=-1)
          result=min_value;
        else
          result+=absolute_change*(default_value-min_value);
      }
    result*=relative_change;
    if(!clip)
      {
        min_value=min_setting.get_min();
        max_value=max_setting.get_max();
}
    if(result<min_value)
      result=min_value;
    else if(result>max_value)
      result=max_value;
    return result;
  }

  const item* hts_label::get_token() const
  {
    if(segment->in("Transcription"))
      return &(segment->as("Transcription").parent().as("TokStructure").parent());
    else if(segment->has_next())
      return &(segment->next().as("Transcription").parent().as("TokStructure").parent());
    else if(segment->has_prev())
      return &(segment->prev().as("Transcription").parent().as("TokStructure").parent());
    else
      return 0;
  }

  double hts_label::get_rate() const
  {
    const utterance& utt=segment->get_relation().get_utterance();
    const voice_params&voice_settings=utt.get_voice().get_info().settings;
    double absolute_rate=utt.get_absolute_rate();
    double relative_rate=utt.get_relative_rate();
    bool clip_rate=(utt.get_flags()&RHVoice_synth_flag_dont_clip_rate)?false:true;
    double rate=calculate_speech_param(absolute_rate,
                                        relative_rate,
                                        voice_settings.default_rate,
                                       voice_settings.min_rate,
                                       voice_settings.max_rate,
clip_rate);
    return rate;
  }

  double hts_label::get_pitch() const
  {
    const utterance& utt=segment->get_relation().get_utterance();
    const voice_params&voice_settings=utt.get_voice().get_info().settings;
    double absolute_pitch=utt.get_absolute_pitch();
    double relative_pitch=utt.get_relative_pitch();
    if(const item* token=get_token())
      if(token->get("verbosity").as<verbosity_t>()&verbosity_pitch)
        relative_pitch*=voice_settings.cap_pitch_factor;
    double pitch=calculate_speech_param(absolute_pitch,
                                        relative_pitch,
                                        voice_settings.default_pitch,
                                        voice_settings.min_pitch,
                                        voice_settings.max_pitch,
true);
    return pitch;
  }

  double hts_label::get_volume() const
  {
    const utterance& utt=segment->get_relation().get_utterance();
    const voice_params&voice_settings=utt.get_voice().get_info().settings;
    double absolute_volume=utt.get_absolute_volume();
    double relative_volume=utt.get_relative_volume();
    double volume=calculate_speech_param(absolute_volume,
                                        relative_volume,
                                        voice_settings.default_volume,
                                        voice_settings.min_volume,
                                         voice_settings.max_volume,
true);
    return volume;
  }

  bool hts_label::is_marked_by_sound_icon() const
  {
    if(segment->in("Transcription"))
      {
        const item& seg_in_word=segment->as("Transcription");
        if(!seg_in_word.has_prev())
          {
            const item& word=seg_in_word.parent().as("TokStructure");
            if(!word.has_prev())
              {
                const item& token=word.parent();
                if(token.get("verbosity").as<verbosity_t>()&verbosity_sound)
                  return true;
              }
          }
      }
    return false;
  }

  const std::string& hts_label::get_name() const
    {
      if(name.empty())
        {
          const hts_labeller& labeller=segment->get_relation().get_utterance().get_language().get_hts_labeller();
          name=labeller.eval_segment_label(*segment);
        }
      return name;
    }
}
