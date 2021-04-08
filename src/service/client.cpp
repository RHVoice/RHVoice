/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <exception>
#include <iostream>
#include <vector>
#include <iterator>
#include <giomm.h>
#include "common.hpp"
#include "tclap/CmdLine.h"

namespace
{
  class params
  {
  public:
    double pitch,rate,volume;
    Glib::ustring speakers;

    params()
    {
    }

    params(int argc,const char* argv[]);

  private:
    class speech_param_constraint: public TCLAP::Constraint<double>
    {
    public:
      speech_param_constraint():
        desc("number between -1 and 1")
      {
      }

      std::string shortID() const
      {
        return desc;
      }

      std::string description() const
      {
        return desc;
      }

      bool check(const double& value) const
      {
        return ((value>=-1)&&(value<=1));
      }

    private:
      std::string desc;
    };

    speech_param_constraint speech_param_range;
  };

  typedef std::vector<gint16> speech_fragment;

  Glib::RefPtr<Glib::MainLoop> main_loop;
  Glib::RefPtr<Gio::DBus::Connection> connection;
  Glib::RefPtr<Gio::DBus::Proxy> proxy;
  bool wave_header_written=false;
  params user_prefs;

  struct quit_main_loop
  {
    void operator()() const
    {
      main_loop->quit();
    }
  };

  params::params(int argc,const char* argv[])
  {
    TCLAP::CmdLine cmd("RHVoice client");
    TCLAP::ValueArg<double> pitch_arg("p","pitch","Speech pitch",false,0,&speech_param_range,cmd);
    TCLAP::ValueArg<double> rate_arg("r","rate","Speech rate",false,0,&speech_param_range,cmd);
    TCLAP::ValueArg<double> volume_arg("v","volume","Speech volume",false,0,&speech_param_range,cmd);
    TCLAP::ValueArg<std::string> speakers_arg("s","speakers","Speakers",true,"","spec",cmd);
    cmd.parse(argc,argv);
    pitch=pitch_arg.getValue();
    rate=rate_arg.getValue();
    volume=volume_arg.getValue();
    speakers=speakers_arg.getValue();
  }

  void call_method(const Glib::ustring& name)
  {
    proxy->call_sync(name);
  }

  template<typename T>
  void call_method(const Glib::ustring& name,const T& arg)
  {
    Glib::Variant<T> var_arg=Glib::Variant<T>::create(arg);
    Glib::VariantContainerBase var_args=Glib::VariantContainerBase::create_tuple(var_arg);
    proxy->call_sync(name,var_args);
  }

  template<typename T1,typename T2>
  void call_method(const Glib::ustring& name,const T1& arg1,const T2& arg2)
  {
    std::vector<Glib::VariantBase> args;
    args.push_back(Glib::Variant<T1>::create(arg1));
    args.push_back(Glib::Variant<T2>::create(arg2));
    Glib::VariantContainerBase var_args=Glib::VariantContainerBase::create_tuple(args);
    proxy->call_sync(name,var_args);
  }

  void send_text()
  {
    std::istreambuf_iterator<char> input_start(std::cin);
    std::istreambuf_iterator<char> input_end;
    Glib::ustring text(input_start,input_end);
    call_method("SpeakText",text);
  }

  void set_properties()
  {
    call_method("SetPitch",user_prefs.pitch);
    call_method("SetRate",user_prefs.rate);
    call_method("SetVolume",user_prefs.volume);
    call_method("SetSpeakers",user_prefs.speakers);
  }

  // FIXME: This should take into account the platform endianness

  void write_wave_header()
  {
    guint32 sample_rate=24000;
    guint32 byte_rate=2*sample_rate;
    unsigned char header[]={
      /* RIFF header */
      'R','I','F','F',            /* ChunkID */
      /* We cannot determine this number in advance */
      /* This is what espeak puts in this field when writing to stdout */
      0x24,0xf0,0xff,0x7f,        /* ChunkSize */
      'W','A','V','E',            /* Format */
      /* fmt */
      'f','m','t',' ',            /* Subchunk1ID */
      16,0,0,0,                 /* Subchunk1Size */
      1,0,                        /* AudioFormat (1=PCM) */
      1,0,                        /* NumChannels */
      0,0,0,0,                    /* SampleRate */
      0,0,0,0,                    /* ByteRate */
      2,0,                        /* BlockAlign */
      16,0,                       /* BitsPerSample */
      /* data */
      'd','a','t','a',          /* Subchunk2ID */
      /* Again using espeak as an example */
      0x00,0xf0,0xff,0x7f};     /* Subchunk2Size */
    /* Write actual sample rate */
    *reinterpret_cast<guint32*>(header+24)=sample_rate;
    /* Write actual byte rate */
    *reinterpret_cast<guint32*>(header+28)=byte_rate;
    std::cout.write(reinterpret_cast<char*>(&header[0]),sizeof(header));
    wave_header_written=true;
  }

  void write_audio(const Glib::VariantContainerBase& params)
  {
    if(!wave_header_written)
      write_wave_header();
    Glib::Variant<speech_fragment> vsamples;
    params.get_child(vsamples);
    speech_fragment samples=vsamples.get();
    std::cout.write(reinterpret_cast<const char*>(&samples[0]),sizeof(gint16)*samples.size());
  }

  void on_signal(const Glib::ustring& sender,const Glib::ustring& signal_name,const Glib::VariantContainerBase& params)
  {
    if(signal_name=="SpeechAvailable")
      write_audio(params);
    else if(signal_name=="Finished")
      Glib::signal_idle().connect_once(quit_main_loop());
  }

  void on_dbus_proxy_available(Glib::RefPtr<Gio::AsyncResult>& result)
  {
    try
      {
        proxy=Gio::DBus::Proxy::create_finish(result);
        proxy->signal_signal().connect(sigc::ptr_fun(&on_signal));
        set_properties();
        send_text();
      }
    catch(const Glib::Error& e)
      {
        std::cerr << "Dbus error: '" << e.what() << "'" << std::endl;
        Glib::signal_idle().connect_once(quit_main_loop());
      }
    catch(const std::exception& e)
      {
        std::cerr << "Error: '" << e.what() << "'" << std::endl;
        Glib::signal_idle().connect_once(quit_main_loop());
      }
  }
}

int main(int argc,const char* argv[])
{
  std::locale::global(std::locale::classic());
  Gio::init();
  try
    {
      user_prefs=params(argc,argv);
    }
  catch(const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      return 1;
    }
  main_loop=Glib::MainLoop::create();
  try
    {
      connection=Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    }
  catch(const Glib::Error& e)
    {
      std::cerr << "Dbus error: '" << e.what() << "'" << std::endl;
      return 1;
    }
  Gio::DBus::Proxy::create(connection,
                           RHVoice::service::well_known_name,
                           RHVoice::service::object_path,
                           RHVoice::service::interface_name,
                           sigc::ptr_fun(&on_dbus_proxy_available));
  main_loop->run();
  return 0;
}
