/* Copyright (C) 2009, 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

#include <getopt.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <unistr.h>
#include "RHVoice.h"

using std::string;
using std::istringstream;
using std::cin;
using std::cout;
using std::endl;
using std::setw;
using std::left;

static int sample_rate=0;

static void write_wave_header()
{
  int byte_rate=2*sample_rate;
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
  header[24]=sample_rate&0xff;
  header[25]=(sample_rate>>8)&0xff;
  header[26]=(sample_rate>>16)&0xff;
  header[27]=(sample_rate>>24)&0xff;
  /* Write actual byte rate */
  header[28]=byte_rate&0xff;
  header[29]=(byte_rate>>8)&0xff;
  header[30]=(byte_rate>>16)&0xff;
  header[31]=(byte_rate>>24)&0xff;
  cout.write(reinterpret_cast<char*>(&header[0]),sizeof(header));
}

static int wave_header_written=0;

int static callback(const short *samples,int num_samples,const RHVoice_event *events,int num_events,RHVoice_message msg)
{
  if(!samples)
    return 1;
  if(!wave_header_written)
    {
      write_wave_header();
      wave_header_written=1;
    }
  cout.write(reinterpret_cast<const char*>(samples),sizeof(short)*num_samples);
  cout.flush();
  return 1;
}

static struct option program_options[]=
  {
    {"help",no_argument,0,'h'},
    {"version",no_argument,0,'V'},
    {"rate",required_argument,0,'r'},
    {"pitch",required_argument,0,'p'},
    {"volume",required_argument,0,'v'},
    {"voice-directory",required_argument,0,'d'},
    {"config",required_argument,0,'c'},
    {"variant",required_argument,0,'n'},
    {"ssml",no_argument,0,'s'},
    {0,0,0,0}
  };

static void show_version()
{
  cout << PACKAGE << " version " << RHVoice_get_version() << endl;
}

static void show_help()
{
  show_version();
  cout << "a speech synthesizer for Russian language\n";
  cout << "usage: RHVoice [options]\n";
  cout << "reads text from stdin (expects UTF-8 encoding)\n";
  cout << "writes speech output to stdout\n";
  int w=40;
  cout << left << setw(w) << "-h, --help" << "print this help message and exit\n";
  cout << setw(w) << "-V, --version" << "print the program version and exit\n";
  cout << setw(w) << "-r, --rate=<number from 0 to 100>" << "rate\n";
  cout << setw(w) << "-p, --pitch=<number from 0 to 100>" << "rate\n";
  cout << setw(w) << "-v, --volume=<number from 0 to 100>" << "volume\n";
  cout << setw(w) << "-d, --voice-directory=<path>" << "path to voice files\n";
  cout << setw(w) << "-c, --config=<path>" << "path to the configuration file\n";
  cout << setw(w) << "-n, --variant=<number from 1 to 2>" << "variant\n";
  cout << setw(w) << "-s, --ssml" << "ssml input\n";
}

static float parse_prosody_option(const char *str)
{
  istringstream s(str);
  s.exceptions(istringstream::failbit|istringstream::badbit);
  float result=-1;
  try
    {
      s >> result;
      if(result<0) result=0;
      else if(result>100) result=100;
    }
  catch(...)
    {
      result=-1;
    }
  return result;
}

static float convert_prosody_value(float val,float nmin,float nmax,float ndef)
{
  float f=val/50.0-1;
  return (ndef+f*((f>=0)?(nmax-ndef):(ndef-nmin)));
}

int main(int argc,char **argv)
{
  const char *voxdir=VOXDIR;
  RHVoice_message msg=NULL;
  const char *cfgfile=NULL;
  float rate=-1;
  float pitch=-1;
  float volume=-1;
  RHVoice_variant variant=RHVoice_variant_pseudo_english;
  int is_ssml=0;
  string text;
  char ch;
  int c;
  int i;
  try
    {
      while((c=getopt_long(argc,argv,"d:c:hVr:p:v:n:s",program_options,&i))!=-1)
        {
          switch(c)
            {
            case 'V':
              show_version();
              return 0;
            case 'h':
              show_help();
              return 0;
            case 'r':
              rate=parse_prosody_option(optarg);
              break;
            case 'p':
              pitch=parse_prosody_option(optarg);
              break;
            case 'v':
              volume=parse_prosody_option(optarg);
              break;
            case 's':
              is_ssml=1;
              break;
            case 'd':
              voxdir=optarg;
              break;
            case 'c':
              cfgfile=optarg;
              break;
            case 'n':
              {
                unsigned int tmp=1;
                istringstream(optarg) >> tmp;
                if(tmp==2) variant=RHVoice_variant_russian;
              }
              break;
            case 0:
              break;
            default:
              return 1;
            }
        }
      while(cin.get(ch))
        {
          if((static_cast<unsigned char>(ch)>=32)||(ch=='\t')||(ch=='\n')||(ch=='\r'))
            text.push_back(ch);
          else
            text.push_back(' ');
        }
      if(text.empty()) return 1;
      sample_rate=RHVoice_initialize(voxdir,callback,cfgfile);
      if(sample_rate==0) return 1;
      if(rate!=-1)
        RHVoice_set_rate(convert_prosody_value(rate,RHVoice_get_min_rate(),RHVoice_get_max_rate(),RHVoice_get_default_rate()));
      if(pitch!=-1)
        RHVoice_set_pitch(convert_prosody_value(pitch,RHVoice_get_min_pitch(),RHVoice_get_max_pitch(),RHVoice_get_default_pitch()));
      if(volume!=-1)
        RHVoice_set_volume(convert_prosody_value(volume,RHVoice_get_min_volume(),RHVoice_get_max_volume(),RHVoice_get_default_volume()));
      RHVoice_set_variant(variant);
      if(is_ssml)
        {
          msg=RHVoice_new_message_utf8(reinterpret_cast<const uint8_t*>(text.data()),text.size(),1);
        }
      else
        {
          bool is_single_char=false;
          int n=u8_mblen(reinterpret_cast<const uint8_t*>(text.data()),text.size());
          if(n<0)
            {
              RHVoice_terminate();
              return 1;
            }
          is_single_char=(n==text.size());
          string ssml_text("<speak>");
          if(is_single_char)
            ssml_text+="<say-as interpret-as=\"tts:char\">";
          ssml_text.reserve(text.size()+50);
          char ch;
          for(string::const_iterator it=text.begin();it!=text.end();++it)
            {
              ch=*it;
              switch(ch)
                {
                case '<':
                  ssml_text+="&lt;";
                  break;
                case '&':
                  ssml_text+="&amp;";
                  break;
                case '\r':
                  ssml_text+="&#13;";
                  break;
                default:
                  ssml_text+=ch;
                  break;
                }
            }
          if(is_single_char)
            ssml_text+="</say-as>";
          ssml_text+="<break strength=\"none\"/>";
          ssml_text+="</speak>";
          msg=RHVoice_new_message_utf8(reinterpret_cast<const uint8_t*>(ssml_text.data()),ssml_text.size(),1);
        }
      if(msg==NULL)
        {
          RHVoice_terminate();
          return 1;
        }
      RHVoice_speak(msg);
      RHVoice_delete_message(msg);
      RHVoice_terminate();
      return 0;
    }
  catch(...)
    {
      if(msg!=NULL) RHVoice_delete_message(msg);
      if(sample_rate!=0) RHVoice_terminate();
      return 1;
    }
}
