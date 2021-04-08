/* Client program to connect to RHVoice running in the daemon mode */
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
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


static struct option program_options[]=
  {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"rate", required_argument, 0, 'r'},
    {"pitch", required_argument, 0, 'p'},
    {"volume", required_argument, 0, 'v'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    // {"list-variants", no_argument, 0, 'l'},
    // {"list-voices", no_argument, 0, 'L'},
    {"variant", required_argument, 0, 'w'},
    {"voice", required_argument, 0, 'W'},
    {"type", required_argument, 0, 't'},
    {"punct", optional_argument, 0, 'P'},
    {"daemon-dir", required_argument, NULL, 'D'},
    {0, 0, 0, 0}
  };

static void show_version(const char *progname)
{
  printf("%s version %s\n", progname, VERSION);
}

static void show_help(const char *progname)
{
  show_version(progname);
  printf("a speech synthesizer for Russian language\n");
  printf("usage: %s [options]\n", progname);
  printf("reads text from a file or from stdin (expects UTF-8 encoding)\n");
  printf("writes speech output to a file or to stdout\n");
  const char *fmt = "%-40s%s\n"; /* option, description */
  printf(fmt, "-h, --help", "print this help message and exit");
  printf(fmt, "-V, --version", "print the program version and exit");
  printf(fmt, "-r, --rate=<number from 0 to 100>", "rate");
  printf(fmt, "-p, --pitch=<number from 0 to 100>", "rate");
  printf(fmt, "-v, --volume=<number from 0 to 100>", "volume");
  printf(fmt, "-P, --punct=<optional string>", "speak all or some punctuation");
  printf(fmt, "-i, --input=<path>", "input file");
  printf(fmt, "-o, --output=<path>", "output file");
  printf(fmt, "-w, --variant=<name>", "select a variant");
  printf(fmt, "-W, --voice=<name>", "select a voice");
  printf(fmt, "-t, --type", "type of input (text/ssml/characters)");
  printf(fmt, "--daemon-dir=<path-to-directory>", "Where to create socket and pid-file");
}

static void check_float(const char *param, const char *value)
{
  if (value[strspn(value, ".0123456789")] != 0)
    {
      fprintf(stderr, "Incorrect value of --%s: `%s'\n", param, value);
      exit(1);
    }
}

static char buf[BUFSIZ];
static int used;		/* of buffer */

static void record_param(const char *param, const char *value)
{
  if (strlen(param) + strlen(value) + 3 > sizeof(buf) - used)
    {
      fprintf(stderr, "Header buffer is overfull. Too many parameters.\n");
      exit(1);
    }
  if (strchr(value, '\n'))
    {
      fprintf(stderr, "Incorrect value of --%s: `%s'\n", param, value);
      exit(1);
    }
  used += sprintf(buf+used, "%s=%s\n", param, value);
}

static void add_path(char **path, const char *str)
{
  int len1, len2;
  len1 = *path ? strlen(*path) : 0;
  len2 = strlen(str);
  *path = realloc(*path, len1 + len2 + 2);
  if (len1 == 0)
    {
      strcpy(*path, str);
      return;
    }
  if ((*path)[len1-1] == '/' && str[0] == '/')
    str += 1;
  else if ((*path)[len1-1] != '/' && str[0] != '/')
    (*path)[len1++] = '/';
  strcpy(*path + len1, str);
}

int main(int argc, char **argv)
{
  char *daemon_dir = NULL;
  char *socket_name = NULL;
  const char *inpath=NULL;
  int input_fd = STDIN_FILENO;
  const char *outpath=NULL;
  int output_fd = STDOUT_FILENO;

  {
    int c, i;
    const char *short_opts = "i:o:d:c:hVr:p:v:w:W:t:P::lL";
    while ((c=getopt_long(argc, argv, short_opts, program_options, &i)) != -1)
      {
        switch (c)
          {
          case 'V':
            show_version(argv[0]);
            return 0;
          case 'h':
            show_help(argv[0]);
            return 0;
          case 'r':
            check_float("rate", optarg);
            record_param("rate", optarg);
            break;
          case 'p':
            check_float("pitch", optarg);
            record_param("pitch", optarg);
            break;
          case 'v':
            check_float("volume", optarg);
            record_param("volume", optarg);
            break;
          case 't':
            if (strcmp(optarg, "text") == 0
                || strcmp(optarg, "ssml") == 0
                || strcmp(optarg, "characters") == 0)
              record_param("type", optarg);
            else
              {
                fprintf(stderr, "Incorrect value of --type: `%s'\n", optarg);
                exit(1);
              }
            break;
          case 'w':
            record_param("variant", optarg);
            break;
          case 'W':
            record_param("voice", optarg);
            break;
          case 'i':
            inpath = optarg;
            break;
          case 'o':
            outpath = optarg;
            break;
          case 'P':
            record_param("punct", optarg ? optarg : "");
            break;
          case 'D':
            daemon_dir = optarg;
            break;
          case 0:
            break;
          default:
            show_help(argv[0]);
            return 1;
          }
      }
  }

  if (daemon_dir)
    add_path(&socket_name, daemon_dir);
  else
    {
      add_path(&socket_name, getenv("HOME"));
      add_path(&socket_name, ".rhvoice");
    }
  add_path(&socket_name, "socket");

  // Creates a socket and connects to it.
  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock == -1)
    {
      perror("socket");
      exit(1);
    }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path) - 1);

  if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
      perror("connect");
      exit(1);
    }

  // Opens input/output files.
  if (inpath && (input_fd = open(inpath, O_RDONLY)) == -1)
    {
      perror(inpath);
      exit(1);
    }
  if (outpath && (output_fd = open(outpath, O_WRONLY|O_TRUNC, 0666)) == -1)
    {
      perror(outpath);
      exit(1);
    }
  if (isatty(output_fd))
    {
      fprintf(stderr, "Cannot write to a terminal.\n");
      exit(1);
    }

  int n, nw;			/* number of read/written bytes */
  if (used >= sizeof(buf)/2)
    {
      // The header is large so write it first.
      nw = 0;
      while (nw < used && (n = write(sock, buf+nw, used-nw)) != -1)
        nw += n;
      used = 0;
    }
  buf[used++] = '\n';

  // First copies whole input to socket.
  while ((n = read(input_fd, buf+used, sizeof(buf)-used)) > 0)
    {
      used += n;
      nw = 0;
      while (nw < used && (n = write(sock, buf+nw, used-nw)) != -1)
        nw += n;
      used = 0;
    }
  shutdown(sock, SHUT_WR);

  // Copies response to output.
  while ((used = read(sock, buf, sizeof(buf))) > 0)
    {
      nw = 0;
      while (nw < used && (n = write(STDOUT_FILENO, buf+nw, used-nw)) != -1)
        nw += n;
  }

  return 0;
}
