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

#ifndef SpeakIMPL_H
#define SpeakIMPL_H

#include <windows.h>
#include <sapi.h>
#include <sapiddk.h>
#include <utility>
#include <vector>
#include <algorithm>
#include <flite.h>
#include <boost/smart_ptr.hpp>
#include <boost/optional.hpp>

class SpeakImpl
{
 public:
  SpeakImpl(const SPVTEXTFRAG *pTextFragList,ISpTTSEngineSite *pOutputSite,cst_voice *Voice);
  HRESULT operator()();
  static int callback(const short *samples,int nsamples,cst_item *seg,int pos_in_seg);
 private:
  typedef std::pair<const SPVTEXTFRAG*,ULONG> pos;

  struct token
  {
    std::wstring whitespace;
    std::wstring text;
    pos start;
    pos text_start;
    pos end;
    std::wstring prepunctuation;
    std::wstring name;
    std::wstring postpunctuation;
  };

  class generic_event
  {
  protected:
    SPEVENT sapi_event;
  public:
    virtual ~generic_event() {};
    const SPEVENT *get_sapi_event()
    {
      return &sapi_event;
    }
  };

  typedef boost::shared_ptr<generic_event> event_ptr;

  class bookmark_event: public generic_event
  {
  public:
    bookmark_event(const std::wstring& name,ULONGLONG astream_offset);
  private:
    std::wstring bookmark;
  };

  class sentence_event: public generic_event
  {
  public:
    sentence_event(cst_utterance *utt,ULONGLONG astream_offset);
  };

  class word_event: public generic_event
  {
  public:
    word_event(const cst_item *t,ULONGLONG astream_offset);
  };

  ISpTTSEngineSite *output;
  cst_voice *voice;
  std::wstring whitespace_chars;
  std::wstring prepunc_chars;
  std::wstring postpunc_chars;
  pos sentence_start;
  pos sentence_end;
  boost::optional<token> cur_token;
  boost::shared_ptr<cst_utterance> cur_utt;
  DWORD actions;
  bool stopped;
  std::vector<event_ptr> events;
  std::vector<short> audio;
  ULONGLONG audio_bytes_written;
  static float rate_table[];
  static float pitch_table[];
  long default_rate;
  ULONG default_volume;

  boost::optional<token> get_token();
  void add_token(const token& t);
  bool is_end_of_sentence(const token& next_token);

  bool static is_cap_letter(wchar_t c)
  {
    return (c>=1040&&c<=1071)||(c==1025)||(c>=L'A'&&c<=L'Z');
  }

  bool next_sentence();
  void synth_sentence();
  void queue_silence(ULONG ms);
  void queue_audio(const short *samples,int nsamples);
  void process_bookmarks_and_silences(const cst_item *t1,const cst_item *t2);
  void write_queued_data();
  DWORD get_actions();
  bool is_stopped();
  bool is_skipping();
  bool is_interrupted();
  int real_callback(const short *samples,int nsamples,cst_item *seg,int pos_in_seg);
  ULONGLONG calculate_event_audio_offset();
  static float sapi_rate_to_native_rate(long r);
  static float sapi_pitch_to_native_pitch(long p);
  static float sapi_volume_to_native_volume(float v);
};

#endif
