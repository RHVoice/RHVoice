Building under Linux
Requirements:
* A C/C++ compiler
* SCons build system (http://www.scons.org/)
* Python (http://www.python.org/)
* glibmm (http://gtkmm.org) for building a D-Bus service
To compile the package type 'scons'. Then type 'scons install'. If you
want to change the installation prefix, then the first command should
be 'scons prefix=<path>'. Type 'scons -h' for help.

                              How to use
Run 'RHVoice-client --help' to see the supported command line options.
If you want to configure Voiceman tu use RHVoice for Russian output,
you should add a new output section in voiceman.conf. Here's an
example:

[output]
name = RHVoice
type = command
lang = rus
synth command = "RHVoice-client -r %r -p %p -v %v -m Elena"
alsa player command = "aplay"
pitch num digits after dot = 2
pitch min = -1
pitch aver = 0
pitch max = 1
rate num digits after dot = 2
rate min = -1
rate aver = 0
rate max = 1
volume num digits after dot = 2
volume min = -1
volume aver = 0
volume max = 1
