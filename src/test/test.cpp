/* Copyright (C) 2012, 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "tclap/CmdLine.h"
#include "core/smart_ptr.hpp"
#include "core/engine.hpp"
#include "core/document.hpp"
#include "core/client.hpp"
#include "audio.hpp"

using namespace RHVoice;

namespace
{
  class audio_player: public client
  {
  public:
    explicit audio_player(const std::string& path);
    bool play_speech(const short* samples,std::size_t count);
    void finish();
    bool set_sample_rate(int sample_rate);

  private:
    audio::playback_stream stream;
  };

  audio_player::audio_player(const std::string& path)
  {
    if(!path.empty())
      {
        stream.set_backend(audio::backend_file);
        stream.set_device(path);
      }
  }

  bool audio_player::set_sample_rate(int sample_rate)
  {
    try
      {
        if(stream.is_open()&&(stream.get_sample_rate()!=sample_rate))
          stream.close();
        stream.set_sample_rate(sample_rate);
        return true;
      }
    catch(...)
      {
        return false;
      }
  }

  bool audio_player::play_speech(const short* samples,std::size_t count)
  {
    try
      {
        if(!stream.is_open())
          stream.open();
        stream.write(samples,count);
        return true;
      }
    catch(...)
      {
        stream.close();
        return false;
      }
  }

  void audio_player::finish()
  {
    if(stream.is_open())
      stream.drain();
  }
}

int main(int argc,const char* argv[])
{
  try
    {
      TCLAP::CmdLine cmd("Simple test of the synthesizer");
      TCLAP::ValueArg<std::string> inpath_arg("i","input","input file",false,"-","path",cmd);
      TCLAP::ValueArg<std::string> outpath_arg("o","output","output file",false,"","path",cmd);
      TCLAP::SwitchArg ssml_switch("s","ssml","Process as ssml",cmd,false);
      TCLAP::ValueArg<std::string> voice_arg("p","profile","voice profile",false,"","spec",cmd);
      TCLAP::ValueArg<unsigned int> rate_arg("r","rate","speech rate",false,100,"percent",cmd);
      TCLAP::ValueArg<unsigned int> pitch_arg("t","pitch","speech pitch",false,100,"percent",cmd);
      TCLAP::ValueArg<unsigned int> volume_arg("v","volume","speech volume",false,100,"percent",cmd);
      cmd.parse(argc,argv);
      std::ifstream f_in;
      if(inpath_arg.getValue()!="-")
        {
          f_in.open(inpath_arg.getValue().c_str());
          if(!f_in.is_open())
            throw std::runtime_error("Cannot open the input file");
        }
      audio_player player(outpath_arg.getValue());
      smart_ptr<engine> eng(new engine);
      voice_profile profile;
      if(!voice_arg.getValue().empty())
        profile=eng->create_voice_profile(voice_arg.getValue());
      std::istreambuf_iterator<char> text_start(f_in.is_open()?f_in:std::cin);
      std::istreambuf_iterator<char> text_end;
      std::unique_ptr<document> doc;
      if(ssml_switch.getValue())
        doc=document::create_from_ssml(eng,text_start,text_end,profile);
      else
        doc=document::create_from_plain_text(eng,text_start,text_end,content_text,profile);
      doc->speech_settings.relative.rate=rate_arg.getValue()/100.0;
      doc->speech_settings.relative.pitch=pitch_arg.getValue()/100.0;
      doc->speech_settings.relative.volume=volume_arg.getValue()/100.0;
      doc->set_owner(player);
      doc->synthesize();
      player.finish();
      return 0;
    }
  catch(const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      return -1;
    }
}
