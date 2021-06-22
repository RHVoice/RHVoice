# Compiling on Linux

## Required tools

To compile RHVoice the following programs must be installed on your system:

* [GCC](https://gcc.gnu.org)
* [Pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
* [SCons](https://www.scons.org)

## Additional dependencies

Before compiling RHVoice, make sure that at least one of the following
audio libraries is installed on your machine:

* [PulseAudio](https://www.freedesktop.org/wiki/Software/PulseAudio/)
* [Libao](https://www.xiph.org/ao/)
* [PortAudio](http://www.portaudio.com) V19

Please note that many distributions provide a separate development package for
a library, which is necessary to compile the programs using the library.
You need to install both runtime and development packages.

## Compilation

To start compilation, execute the command:

```bash
scons
```

You can change some compilation options. For more information, run the command:

```bash
scons -h
```

## Installation

To install RHVoice execute the following commands:

```bash
# scons install
```

Now you can check if the synthesizer is working:

```bash
echo test|RHVoice-test
```

## Using RHVoice with Speech Dispatcher

To enable Speech Dispatcher to find the output module, which allows it to
communicate with RHVoice, create a symlink
to the module in the directory where Speech Dispatcher has installed its
built-in output modules.
For example:

```bash
# ln -s /usr/local/bin/sd_rhvoice /usr/lib/speech-dispatcher-modules
```
