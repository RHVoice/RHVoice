# RHVoice

RHVoice is a free and open-source speech synthesizer.

## Features

### Speech synthesis method

RHVoice uses [statistical parametric synthesis](https://en.wikipedia.org/wiki/Speech_synthesis#HMM-based_synthesis).
It relies on existing open-source speech technologies (mainly
[HTS](https://hts.sp.nitech.ac.jp) and related software).

Voices are built from recordings of natural speech. They have small footprints,
because only statistical models are stored on users' computers. And though the
voices lack the naturalness of the synthesizers which generate speech by
combining segments of the recordings themselves, they are still very
intelligible and resemble the speakers who recorded the source material.

### Supported languages

Initially, RHVoice could speak only Russian. Now it also supports:

* American and Scottish English
* Brazilian Portuguese
* Esperanto
* Georgian
* Ukrainian
* Kyrgyz
* Tatar
* Macedonian
* Albanian
* Polish

In theory, it is possible to implement support for
other languages, if all the necessary resources can be found or
created.

### Synthesis example

If you want to listen to an example of speech synthesis, You can use the TTS
service on [this page](https://data2data.ru/tts/).

### Supported platforms

RHVoice supports the following platforms:

* Windows (prebuilt binaries is available in documentation)
* GNU/Linux (building instructions and packaging status
  can be found in "Compiling instructions" section of documentation.
* Android (can be installed thru
  [F-Droid](https://f-droid.org/packages/com.github.olga_yakovleva.rhvoice.android/)
  or [Google Play](https://play.google.com/store/apps/details?id=com.github.olga_yakovleva.rhvoice.android)

It is compatible with standard text-to-speech interfaces on these platforms:
SAPI5 on Windows, [Speech Dispatcher](https://devel.freebsoft.org/speechd) on
GNU/Linux and Android's text-to-speech APIs. It can also be used by the
[NVDA screen reader](https://www.nvaccess.org) directly (the driver is provided
by RHVoice itself).

## Documentation

All prebuild binaries packages, main
and legal information and more
are available in three languages:

* [English](doc/en/index.md)
* [Русский](doc/ru/index.md)
* [Українська](doc/ua/index.md)

## Community

### Official

* [GitHub Discussions](https://github.com/RHVoice/RHVoice/discussions/)
* [Mailing list](https://groups.io/g/RHVoice-rus) (Russian)

### Unofficial

* IRC channel: `#rhvoice` at [irc.libera.chat](ircs://irc.libera.chat:6697)
* Matrix room: [#rhvoice:libera.chat](https://matrix.to/#/#rhvoice:libera.chat)
