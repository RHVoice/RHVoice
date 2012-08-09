/* Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <ao/ao.h>
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

using namespace RHVoice;

namespace
{
  class audio_player: public client
  {
  public:
    explicit audio_player(const std::string& path);
    ~audio_player();
    bool play_speech(const short* samples,std::size_t count);
    bool set_sample_rate(int sample_rate);

    int get_sample_rate() const
    {
      return sample_format.rate;
    }

  private:
    std::string file_path;
    ao_sample_format sample_format;
    ao_device* device;

    bool open_device();
  };

  audio_player::audio_player(const std::string& path):
    file_path(path),
    device(0)
  {
    ao_initialize();
    sample_format.bits=16;
    sample_format.rate=16000;
    sample_format.channels=1;
    sample_format.byte_format=AO_FMT_NATIVE;
    sample_format.matrix=0;
  }

  audio_player::~audio_player()
  {
    if(device)
      ao_close(device);
    ao_shutdown();
  }

  bool audio_player::set_sample_rate(int sample_rate)
  {
    if(device)
      {
        ao_close(device);
        device=0;
      }
    sample_format.rate=sample_rate;
    return open_device();
  }

  bool audio_player::open_device()
  {
    if(file_path.empty())
      {
        int id=ao_default_driver_id();
        if(id<0)
          return false;
        device=ao_open_live(id,&sample_format,0);
      }
    else
      {
        int id=ao_driver_id("wav");
        if(id<0)
          return false;
        device=ao_open_file(id,file_path.c_str(),1,&sample_format,0);
      }
    return device;
  }

  bool audio_player::play_speech(const short* samples,std::size_t count)
  {
    if(!device&&!open_device())
      return false;
    else
      return (ao_play(device,reinterpret_cast<char*>(const_cast<short*>(samples)),count*sizeof(short))!=0);
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
      cmd.parse(argc,argv);
      std::ifstream f_in;
      if(inpath_arg.getValue()!="-")
        {
          f_in.open(inpath_arg.getValue().c_str());
          if(!f_in.is_open())
            throw std::runtime_error("Cannot open the input file");
        }
      std::auto_ptr<client> p(new audio_player(outpath_arg.getValue()));
      smart_ptr<engine> eng(new engine);
      std::istreambuf_iterator<char> text_start(f_in.is_open()?f_in:std::cin);
      std::istreambuf_iterator<char> text_end;
      std::auto_ptr<document> doc;
      if(ssml_switch.getValue())
        doc=document::create_from_ssml(eng,text_start,text_end);
      else
        doc=document::create_from_plain_text(eng,text_start,text_end);
      doc->set_owner(*p);
      doc->synthesize();
      return 0;
    }
  catch(const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      return -1;
    }
}
