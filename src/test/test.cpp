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
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>

#ifdef WITH_CLI11
	#include <CLI/CLI.hpp>
#else
	#include "tclap/CmdLine.h"
#endif

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
    bool set_buffer_size(unsigned int buffer_size);

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

  bool audio_player::set_buffer_size(unsigned int buffer_size)
  {
    try
      {
        if(stream.is_open()&&(stream.get_buffer_size()!=buffer_size))
          stream.close();
        stream.set_buffer_size(buffer_size);
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

#ifdef WITH_CLI11
	typedef CLI::App AppT;
	#define GET_CLI_PARAM_VALUE(NAME) (NAME ## Stor)
#else
	typedef TCLAP::CmdLine AppT;
	#define GET_CLI_PARAM_VALUE(NAME) (NAME).getValue()
#endif




int main(int argc,const char* argv[])
{
  try{
      AppT cmd("Simple test of the synthesizer");

#ifdef WITH_CLI11
      std::string inpath_argStor {"-"};
      auto inpath_arg = cmd.add_option("-i,--input",inpath_argStor,"input file");  // path
      std::string outpath_argStor {""};
      auto outpath_arg = cmd.add_option("-o,--output",outpath_argStor,"output file");  // path
      bool ssml_switchStor = false;
      auto ssml_switch = cmd.add_option("-s,--ssml",ssml_switchStor,"Process as ssml");
      std::string voice_argStor{""};
      auto voice_arg = cmd.add_option("-p,--profile",voice_argStor,"voice profile");  // spec
      unsigned int rate_argStor = 100;
      auto rate_arg = cmd.add_option("-r,--rate",rate_argStor,"speech rate");  // percent
      uint32_t sample_rateStor = 24000;
      auto sample_rate = cmd.add_option("-R,--sample-rate",sample_rateStor,"sample rate");  // Hz
      unsigned int pitch_argStor = 100;
      auto pitch_arg =  cmd.add_option("-t,--pitch",pitch_argStor,"speech pitch");  //percent
      unsigned int volume_argStor = 100;
      auto volume_arg =  cmd.add_option("-v,--volume",volume_argStor,"speech volume");  // percent
      std::string quality_argStor;
      auto quality_arg = cmd.add_option("-q,--quality",quality_argStor,"quality");
      unsigned int view_argStor = 0;
      auto view_arg =  cmd.add_option("-w,--view", view_argStor,"stream view size");
      cmd.allow_windows_style_options();
#else
      TCLAP::ValueArg<std::string> inpath_arg("i","input","input file",false,"-","path",cmd);
      TCLAP::ValueArg<std::string> outpath_arg("o","output","output file",false,"","path",cmd);
      TCLAP::SwitchArg ssml_switch("s","ssml","Process as ssml",cmd,false);
      TCLAP::ValueArg<std::string> voice_arg("p","profile","voice profile",false,"","spec",cmd);
      TCLAP::ValueArg<unsigned int> rate_arg("r","rate","speech rate",false,100,"percent",cmd);
      TCLAP::ValueArg<uint32_t> sample_rate("R","sample-rate","sample rate",false, 24000,"Hz",cmd);
      TCLAP::ValueArg<unsigned int> pitch_arg("t","pitch","speech pitch",false,100,"percent",cmd);
      TCLAP::ValueArg<unsigned int> volume_arg("v","volume","speech volume",false,100,"percent",cmd);
      TCLAP::ValueArg<std::string> quality_arg("q","quality","quality",false,"","quality",cmd);
      TCLAP::ValueArg<unsigned int> view_arg("w","view","stream view size",false,0,"positive",cmd);
#endif

#ifdef WITH_CLI11
     try{
#endif
      cmd.parse(argc,argv);
#ifdef WITH_CLI11
      }catch (const CLI::ParseError &e) {
        return cmd.exit(e);
      }
#endif

      std::ifstream f_in;
      if(GET_CLI_PARAM_VALUE(inpath_arg)!="-")
        {
          f_in.open(GET_CLI_PARAM_VALUE(inpath_arg).c_str());
          if(!f_in.is_open())
            throw std::runtime_error("Cannot open the input file");
        }
      audio_player player(GET_CLI_PARAM_VALUE(outpath_arg));
      player.set_sample_rate(GET_CLI_PARAM_VALUE(sample_rate));
      player.set_buffer_size(20);
      std::shared_ptr<engine> eng(new engine);
      auto q=GET_CLI_PARAM_VALUE(quality_arg);
      if(!q.empty())
        eng->configure("quality", q);
      auto w=GET_CLI_PARAM_VALUE(view_arg);
      if(w>0)
        eng->stream_settings.view_size=w;

      voice_profile profile;
      if(!GET_CLI_PARAM_VALUE(voice_arg).empty())
        profile=eng->create_voice_profile(GET_CLI_PARAM_VALUE(voice_arg));
      std::istreambuf_iterator<char> text_start(f_in.is_open()?f_in:std::cin);
      std::istreambuf_iterator<char> text_end;
      std::unique_ptr<document> doc;
      if(GET_CLI_PARAM_VALUE(ssml_switch))
        doc=document::create_from_ssml(eng,text_start,text_end,profile);
      else
        doc=document::create_from_plain_text(eng,text_start,text_end,content_text,profile);
      doc->speech_settings.relative.rate=GET_CLI_PARAM_VALUE(rate_arg)/100.0;
      doc->speech_settings.relative.pitch=GET_CLI_PARAM_VALUE(pitch_arg)/100.0;
      doc->speech_settings.relative.volume=GET_CLI_PARAM_VALUE(volume_arg)/100.0;
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
