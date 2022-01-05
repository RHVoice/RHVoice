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
* [Speech Dispatcher](https://freebsoft.org/speechd)

Please note that many distributions provide a separate development package for
a library, which is necessary to compile the programs using the library.
You need to install both runtime and development packages.

## Get sources

1. One line
    ```bash
    git clone --recursive https://github.com/RHVoice/RHVoice.git
    ```
2. Step by step
    ```bash
    git clone https://github.com/RHVoice/RHVoice.git
    cd RHVoice
    git submodule update --init
    ```

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

To install RHVoice execute the following command:

```bash
# scons install
```

Now you can check if the synthesizer is working:

```bash
echo test|RHVoice-test
```
