/* Copyright (C) 2009, 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com> */
/* Copyright (C) 2011  Dmitri Paduchikh <dpaduchikh@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <getopt.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#ifdef WIN32
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#else
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#endif
#include "RHVoice.h"

using std::string;
using std::istringstream;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::setw;
using std::left;
using std::ifstream;
using std::ofstream;
using std::exit;

static uint32_t sample_rate=0;

static ofstream outfile;
static ifstream infile;

static void write_wave_header()
{
  uint32_t byte_rate=2*sample_rate;
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
  *reinterpret_cast<uint32_t*>(header+24)=sample_rate;
  /* Write actual byte rate */
  *reinterpret_cast<uint32_t*>(header+28)=byte_rate;
  if(!outfile.is_open())
    {
#ifdef WIN32
      _setmode(_fileno(stdout),_O_BINARY);
#endif
    }
  ((outfile.is_open())?outfile:cout).write(reinterpret_cast<char*>(&header[0]),sizeof(header));
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
  ((outfile.is_open())?outfile:cout).write(reinterpret_cast<const char*>(samples),sizeof(short)*num_samples);
  if(!outfile.is_open())
    cout.flush();
  return 1;
}

static bool daemon_running = false;
#ifndef WIN32
static int start_daemon_flag = false;
static int kill_daemon_flag = false;
static string daemon_dir;
static char *pidfile_name;
static char *socket_name;

static void cleanup_files()
{
  // The order of removal is important.
  if (socket_name && *socket_name)
    {
      unlink(socket_name);
      *socket_name = 0;
    }
  if (pidfile_name && *pidfile_name)
    {
      unlink(pidfile_name);
      *pidfile_name = 0;
    }
}

static void cleanup_handler(int sig)
{
  cleanup_files();
  _exit(2);
}

static void kill_daemon()
{
  int pid;
  ifstream pidfile(pidfile_name);
  if ((pidfile >> pid) && kill(pid, SIGTERM) == -1)
    {
      if (errno == ESRCH)
        {
          // Process does not exist, so removing its files.
          cleanup_files();
        }
      else
        {
          int saved_errno = errno;
          cerr << "kill: " << strerror(saved_errno) << '\n';
        }
    }
}
#endif

static bool set_prosody_option(float& param, const char *str);

namespace opt
{
  const char *inpath=NULL;
  const char *outpath=NULL;
  const char *datadir=NULL;
  const char *cfgpath=NULL;
  float rate=-1;
  float pitch=-1;
  float volume=-1;
  const char *variant_name=NULL;
  int variant=0;
  const char *voice_name=NULL;
  int voice=0;
  RHVoice_message_type message_type=RHVoice_message_text;
  RHVoice_punctuation_mode punct_mode=RHVoice_punctuation_none;
  const char *punct_list=NULL;
  int opt_list_voices=0;
  int opt_list_variants=0;

  struct option program_options[]=
    {
      {"help",no_argument,0,'h'},
      {"version",no_argument,0,'V'},
      {"rate",required_argument,0,'r'},
      {"pitch",required_argument,0,'p'},
      {"volume",required_argument,0,'v'},
      {"input",required_argument,0,'i'},
      {"output",required_argument,0,'o'},
      {"data",required_argument,0,'d'},
      {"config",required_argument,0,'c'},
      {"list-variants",no_argument,0,'l'},
      {"list-voices",no_argument,0,'L'},
      {"variant",required_argument,0,'w'},
      {"voice",required_argument,0,'W'},
      {"type",required_argument,0,'t'},
      {"punct",optional_argument,NULL,'P'},
#ifndef WIN32
      { "daemon", no_argument, &start_daemon_flag, true },
      { "kill", no_argument, &kill_daemon_flag, true },
      { "daemon-dir", required_argument, NULL, 'D' },
#endif
      {0,0,0,0}
    };

  void show_version()
  {
    cout << PACKAGE << " version " << RHVoice_get_version() << endl;
  }

  void show_help()
  {
    show_version();
    cout << "a speech synthesizer for Russian language\n";
    cout << "usage: RHVoice [options]\n";
    cout << "reads text from a file or from stdin (expects UTF-8 encoding)\n";
    cout << "writes speech output to a file or to stdout\n";
    int w=40;
    cout << left << setw(w) << "-h, --help" << "print this help message and exit\n";
    cout << setw(w) << "-V, --version" << "print the program version and exit\n";
    cout << setw(w) << "-r, --rate=<number from 0 to 100>" << "rate\n";
    cout << setw(w) << "-p, --pitch=<number from 0 to 100>" << "rate\n";
    cout << setw(w) << "-v, --volume=<number from 0 to 100>" << "volume\n";
    cout << setw(w) << "-P, --punct=<optional string>" << "speak all or some punctuation\n";
    cout << setw(w) << "-i, --input=<path>" << "input file\n";
    cout << setw(w) << "-o, --output=<path>" << "output file\n";
    cout << setw(w) << "-d, --data=<path>" << "path to the data directory\n";
    cout << setw(w) << "-c, --config=<path>" << "path to the configuration directory\n";
    cout << setw(w) << "-l, --list-variants" << "list all available variants\n";
    cout << setw(w) << "-L, --list-voices" << "list all available voices\n";
    cout << setw(w) << "-w, --variant=<name>" << "select a variant\n";
    cout << setw(w) << "-W, --voice=<name>" << "select a voice\n";
    cout << setw(w) << "-t, --type" << "type of input (text/ssml/characters)\n";
#ifndef WIN32
    cout << setw(w) << "--daemon" << "start as a daemon\n";
    cout << setw(w) << "--kill" << "kill the running daemon\n";
    cout << setw(w) << "--daemon-dir=<path-to-directory>" << "Where to create socket and pid-file\n";
#endif
  }

  // Returns error message if any.
  const char* set_option(int c, int idx, const char* arg)
  {
    using ::set_prosody_option;
    const char *daemon_err = "not allowed in daemon mode";
    const char *float_err = "not a float value";

    // We create a copy of arg when it can be saved in daemon mode.
    switch (c)
      {
      case 'V':
        if (daemon_running) return daemon_err;
        show_version();
        exit(0);

      case 'h':
        if (daemon_running) return daemon_err;
        show_help();
        exit(0);

      case 'r':
        if (!set_prosody_option(rate, arg))
          return float_err;
        break;

      case 'p':
        if (!set_prosody_option(pitch, arg))
          return float_err;
        break;

      case 'v':
        if (!set_prosody_option(volume, arg))
          return float_err;
        break;

      case 't':
        {
          string type(arg);
          if(type=="text")
            message_type=RHVoice_message_text;
          else if(type=="ssml")
            message_type=RHVoice_message_ssml;
          else if(type=="characters")
            message_type=RHVoice_message_characters;
          else
            return "unknown input type";
        }
        break;

      case 'd':
        if (daemon_running) return daemon_err;
        datadir=arg;
        break;

      case 'c':
        if (daemon_running) return daemon_err;
        cfgpath=arg;
        break;

      case 'w':
        variant_name=strdup(arg);
        break;

      case 'W':
        voice_name=strdup(arg);
        break;

      case 'i':
        if (daemon_running) return daemon_err;
        inpath=arg;
        break;

      case 'o':
        if (daemon_running) return daemon_err;
        outpath=arg;
        break;

      case 'P':
        if(arg!=NULL)
          {
            punct_mode=RHVoice_punctuation_some;
            punct_list=strdup(arg);
          }
        else
          punct_mode=RHVoice_punctuation_all;
        break;

      case 'l':
        if (daemon_running) return daemon_err;
        opt_list_variants=1;
        break;

      case 'L':
        if (daemon_running) return daemon_err;
        opt_list_voices=1;
        break;

#ifndef WIN32
      case 'D':
        if (daemon_running) return daemon_err;
        daemon_dir = arg;
        break;
#endif

      case 0:
        if (daemon_running) return daemon_err;
        break;

      default:
        return "unknown option";
      }
    return NULL;
  }
}

#ifndef WIN32
// Function parses the header transmitted through the connection to daemon
// and sets speech parameters to the corresponding values. Each parameter is
// encoded in a separate line in the form
// parameter=value
// The text proper is separated from the header by the empty line.
static void parse_daemon_header()
{
  using opt::program_options;
  using opt::set_option;
  string line, param;
  bool skip = false;

  while (getline(cin, line), !line.empty())
    {
      if (skip) continue;
      string::size_type p = line.find('=');
      if (p == string::npos)
        {
          syslog(LOG_ERR, "Wrong header: %s", line.substr(0, 40).c_str());
          skip = true;
          continue;
        }

      param.assign(line, 0, p);
      struct option* popt = program_options;
      for (; popt->name && param != popt->name; popt++) {}
      if (popt->name == NULL)
        {
          syslog(LOG_ERR, "Wrong header: %s", param.substr(0, 40).c_str());
          skip = true;
          continue;
        }
      const char* err = set_option(popt->flag == NULL ? popt->val : 0,
                                   popt - program_options,
                                   p+1 == line.length() ? NULL : line.c_str()+p+1);
      if (err)
        syslog(LOG_ERR, "%s: %s", param.c_str(), err);
    }
}

static void handle_daemon_error(const char *msg)
{
  syslog(LOG_ERR, "%s: %m", msg);
    cleanup_files();
    RHVoice_terminate();
  exit(1);
}

static void start_daemon()
{
  if (mkdir(daemon_dir.c_str(), 0777) == -1 && errno != EEXIST)
    {
      int saved_errno = errno;
      cerr << "mkdir(" << daemon_dir << "): " << strerror(saved_errno) << '\n';
      RHVoice_terminate();
      exit(1);
    }
  int pidfile;
  // Tries to create pidfile with timeout.
  for (int i=1;
       (pidfile = open(pidfile_name, O_WRONLY|O_CREAT|O_EXCL, 0666)) == -1
         && errno == EEXIST && kill_daemon_flag && i <= 20; i++)
    {
      timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = 500000000L;
      nanosleep(&ts, NULL);
    }
  if (pidfile == -1)
    {
      int saved_errno = errno;
      cerr << "Could not create " << pidfile_name << " anew: "
           << strerror(saved_errno) << endl;
      RHVoice_terminate();
      exit(1);
    }
  close(pidfile);
  signal(SIGTERM, cleanup_handler);
  signal(SIGCHLD, SIG_IGN);

  openlog("RHVoice", LOG_PID, LOG_USER);
  if (daemon(0, 0) == -1)
    handle_daemon_error("daemon");
  daemon_running = true;
  {
    ofstream pidfile(pidfile_name);
    pidfile << getpid() << endl;
    pidfile.close();
  }

  // Creates a socket and waits incoming connections.
  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock == -1)
    handle_daemon_error("socket");

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path) - 1);

  if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    handle_daemon_error("bind");

  if (listen(sock, 20) == -1)
    handle_daemon_error("listen");

  // Accepts incoming connections in the infinite loop.
  do
    {
      socklen_t size = sizeof(addr);
      int conn = accept(sock, (struct sockaddr *) &addr, &size);
      if (conn == -1)
        handle_daemon_error("accept");

      switch (fork())
        {
        case -1:
          handle_daemon_error("fork");

        case 0:
	  signal(SIGTERM, SIG_DFL);
          dup2(conn, STDIN_FILENO);
          dup2(conn, STDOUT_FILENO);
          parse_daemon_header();
          return;
        }
      close(conn);
    }
  while (true);
}
#endif

static void list_variants()
{
  int n=RHVoice_get_variant_count();
  if(n==0) return;
  cout << "List of variants:\n";
  int i;
  for(i=1;i<=n;i++)
    {
      cout << RHVoice_get_variant_name(i) << endl;
    }
}

static void list_voices()
{
  int n=RHVoice_get_voice_count();
  if(n==0) return;
  cout << "List of voices:\n";
  int i;
  for(i=1;i<=n;i++)
    {
      cout << RHVoice_get_voice_name(i) << endl;
    }
}

static bool set_prosody_option(float& param, const char *str)
{
  istringstream s(str);
  float result=-1;
  
  if (s >> result)
    {
      if(result<0) result=0;
      else if(result>100) result=100;
      param = result;
      return true;
    }
  return false;
}

static float convert_prosody_value(float val,float nmin,float nmax,float ndef)
{
  float f=val/50.0-1;
  return (ndef+f*((f>=0)?(nmax-ndef):(ndef-nmin)));
}

int main(int argc,char **argv)
{
  using namespace opt;
  RHVoice_message msg=NULL;
  unsigned int init_options=0;
  string text;
  char ch;
  int c;
  int i;
  try
    {
      while((c=getopt_long(argc,argv,"i:o:d:c:hVr:p:v:w:W:t:P::lL",program_options,&i))!=-1)
	{
          if (c == '?') exit(1);
	  const char* err = set_option(c, i, optarg);
	  if (err)
	    {
	      cerr << "Error in " << program_options[i].name
		   << " option: " << err << endl;
	      exit(1);
	    }
	}

#ifndef WIN32
      if ((opt_list_variants || opt_list_voices || inpath || outpath)
          && (kill_daemon_flag || start_daemon_flag))
        {
          cerr << "Cannot accept these arguments "
               << "while --daemon or --kill is given\n";
          exit(1);
        }

      if(start_daemon_flag)
        init_options|=RHVoice_preload_voices;

      if (daemon_dir.empty())
        {
          daemon_dir = getenv("HOME");
          if (daemon_dir[daemon_dir.length()-1] != '/')
            daemon_dir += '/';
          daemon_dir += ".rhvoice/";
        }
      else if (daemon_dir[daemon_dir.length()-1] != '/')
        daemon_dir += '/';
      pidfile_name = new char[daemon_dir.length() + sizeof("rhvoice.pid")];
      daemon_dir.copy(pidfile_name, string::npos);
      strcpy(pidfile_name + daemon_dir.length(), "rhvoice.pid");
      socket_name = new char[daemon_dir.length() + sizeof("socket")];
      daemon_dir.copy(socket_name, string::npos);
      strcpy(socket_name + daemon_dir.length(), "socket");

      if (kill_daemon_flag)
        {
          kill_daemon();
          if (!start_daemon_flag)
            exit(0);
        }
#endif

      sample_rate=RHVoice_initialize(datadir,callback,cfgpath,init_options);
      if(sample_rate==0) return 1;
      if(opt_list_variants||opt_list_voices)
        {
          if(opt_list_voices)
            list_voices();
          if(opt_list_variants)
            list_variants();
          RHVoice_terminate();
          return 0;
        }

#ifndef WIN32
if (start_daemon_flag)
        start_daemon();
#endif

      if(inpath!=NULL)
        infile.open(inpath);
      if(outpath!=NULL)
        outfile.open(outpath,std::ios::out|std::ios::binary);
      while(((infile.is_open())?infile:cin).get(ch))
        {
          if((static_cast<unsigned char>(ch)>=32)||(ch=='\t')||(ch=='\n')||(ch=='\r'))
            text.push_back(ch);
          else
            text.push_back(' ');
        }
      if(text.empty())
        {
          RHVoice_terminate();
          return 1;
        }
      if(rate!=-1)
        RHVoice_set_rate(convert_prosody_value(rate,RHVoice_get_min_rate(),RHVoice_get_max_rate(),RHVoice_get_default_rate()));
      if(pitch!=-1)
        RHVoice_set_pitch(convert_prosody_value(pitch,RHVoice_get_min_pitch(),RHVoice_get_max_pitch(),RHVoice_get_default_pitch()));
      if(volume!=-1)
        RHVoice_set_volume(convert_prosody_value(volume,0,RHVoice_get_max_volume(),RHVoice_get_default_volume()));
      if(variant_name!=NULL)
        {
          variant=RHVoice_find_variant(variant_name);
          if(variant>0)
            RHVoice_set_variant(variant);
        }
      if(voice_name!=NULL)
        {
          voice=RHVoice_find_voice(voice_name);
          if(voice>0)
            RHVoice_set_voice(voice);
        }
      if(punct_mode!=RHVoice_punctuation_none)
        {
          RHVoice_set_punctuation_mode(punct_mode);
          if(punct_list!=NULL)
            RHVoice_set_punctuation_list(punct_list);
        }
      msg=RHVoice_new_message_utf8(reinterpret_cast<const uint8_t*>(text.data()),text.size(),message_type);
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
