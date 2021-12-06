# RHVoice

RHVoice is a free and open-source speech synthesizer.

## Features

### Speech synthesis method

RHVoice uses [statistical parametric synthesis](https://en.wikipedia.org/wiki/Speech_synthesis#HMM-based_synthesis).
It relies on existing open-source speech technologies (mainly
[HTS](http://hts.sp.nitech.ac.jp) and related software).

Voices are built from recordings of natural speech. They have small footprints,
because only statistical models are stored on users' computers. And though the
voices lack the naturalness of the synthesizers which generate speech by
combining segments of the recordings themselves, they are still very
intelligible and resemble the speakers who recorded the source material.

### Supported languages

Initially, RHVoice could speak only Russian. Now it also supports:

* American English
* Brazilian Portuguese
* Esperanto
* Georgian
* Ukrainian
* Kyrgyz
* Tatar
* Macedonian
* Albanian

In theory, it is possible to implement support for
other languages, if all the necessary resources can be found or
created.

### Synthesis example

If you want to listen to an example of speech synthesis, You can use the TTS
service on [this page.](https://data2data.ru/tts/)

### Supported platforms

RHVoice supports the following platforms:

* Windows (prebuilt binaries is available [here](doc/en/Binaries.md))
* GNU/Linux (building instructions can be found
  [here](doc/en/Compiling-on-Linux.md). You can also look at
  [packaging status](doc/en/Packaging-status.md).)
* Android (can be installed thru
  [F-Droid](https://f-droid.org/packages/com.github.olga_yakovleva.rhvoice.android/)
  or [Google Play](https://play.google.com/store/apps/details?id=com.github.olga_yakovleva.rhvoice.android)
  you can also download an APK consult [this file](doc/en/Binaries.md#user-content-android))

It is compatible with standard text-to-speech interfaces on these platforms:
SAPI5 on Windows, [Speech Dispatcher](https://devel.freebsoft.org/speechd) on
GNU/Linux and Android's text-to-speech APIs. It can also be used by the
[NVDA screen reader](https://www.nvaccess.org) directly (the driver is provided
by RHVoice itself).

## License

The main library is distributed under [LGPL v2.1](https://www.gnu.org/licenses/lgpl-2.1.html)
or later. But it relies on [MAGE](https://github.com/numediart/mage)
for better responsiveness. MAGE is distributed under
[GPL v3](https://www.gnu.org/licenses/gpl-3.0.html) or later, so the combination
is under GPL v3 or later. If you want to use the library in your program under
[GPL v2](https://www.gnu.org/licenses/gpl-2.0.html) or LGPL, compile
the library without MAGE.

The following restrictions apply to some of the voices:

### RHVoice Lab's voices

All voices from [RHVoice Lab's site](https://rhvoice.su/voices/)
are distributed under the
<!-- markdownlint-disable-next-line MD013 -->
[Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International Public License](https://creativecommons.org/licenses/by-nc-nd/4.0/)

You can send a request for integration of voice into any product to the
laboratory's [e-mail address](mailto:rhvoice@tiflo.org). If the approval is
given by the speaker and our team you will get the appropriate permission.

### Talgat

Permission is hereby granted to use, copy, distribute and modify the
product on condition that the product or its derivatives are not used
commercially: you are not allowed to sell the product or its
derivatives.

Regarding commercial use of the voice or any other use not listed here
contact the developer acting as a representative of the rights holder.

E-mail: rsbs@yandex.ru

### Natia

Copyright (c) 2013 - 2018 Beqa gozalishvili, Olga Yakovleva, Vladimer
urdulashvili

The Georgian voice can be used free of charge only by individuals for personal
use.

Individuals can download  voice from official sources of the project and use it
for example with screen reading software or with software that can read text
aloud.

Individuals can also redistribute the voice among other individuals by putting
it on the network, portable devices or in any other way.

It is prohibited to copy, modify, distribute, sell or use this voice
by Organizations and manufacturers for inclusion in their products or for any
other use. To use the Georgian voice for such purposes, organizations and
manufacturers must agree on the details with the copyright holders:

* Beqa Gozalishvili <beqaprogger@gmail.com>
* Vladimer Urdulashvili <vladimerurdulashvili@gmail.com>

### Letícia-F123

Copyright (C) 2018 - 2020  Olga Yakovleva, Fernando H. F. Botelho

The voice is distributed under the
[Creative Commons Attribution-ShareAlike 4.0 International Public License](https://creativecommons.org/licenses/by-sa/4.0/).

See the documents in the licenses/voices subdirectory for more details.
