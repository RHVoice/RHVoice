# RHVoice

RHVoice is a free and open source speech synthesizer.

## Features

### Speech synthesis method

RHVoice uses [statistical parametric synthesis](https://en.wikipedia.org/wiki/Speech_synthesis#HMM-based_synthesis).
It relies on existing
open source speech technologies (mainly
[HTS](http://hts.sp.nitech.ac.jp) and related software).

Voices are built from recordings of natural speech. They have small
footprints, because only statistical models are stored on users'
computers. And though the voices lack the naturalness of the
synthesizers which generate speech by combining segments of the
recordings themselves, they are still very intelligible and resemble
the speakers who recorded the source material.

### Supported languages

Initially, RHVoice could speak only Russian. Now it also supports:

* American English
* Brazilian Portuguese
* Esperanto
* Georgian
* Ukrainian
* Kyrgyz
* Tatar

In theory, it is possible to implement support for
other languages, if all the necessary resources can be found or
created.

### Supported platforms

RHVoice supports following platforms:

* Windows (prebuilt binaries is available [here](doc/prebuilt/Binaries.md))
* GNU/Linux (building instructions can be found [here](doc/Compiling/Linux.md) You can also look at [packaging status](doc/prebuilt/Packaging-status.md).)
* Android (can be installed thru [Google Play](https://play.google.com/store/apps/details?id=com.github.olga_yakovleva.rhvoice.android) or you can download an APK consult [this file](doc/prebuilt/Binaries.md##Android) ) )

It is compatible with
standard text-to-speech interfaces on these platforms: SAPI5 on
Windows, [Speech Dispatcher](http://devel.freebsoft.org/speechd) on
GNU/Linux and Android's text-to-speech APIs. It can also be used by
the [NVDA screen reader](http://www.nvaccess.org) directly (the driver
is provided by RHVoice itself).

## License

The main library is distributed under [LGPL v2.1](https://www.gnu.org/licenses/lgpl-2.1.html) or later.
some components and voices use other licenses.for more information please read [license.md](license.md) file.
