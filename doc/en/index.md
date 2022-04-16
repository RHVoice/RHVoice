English / [Russian](/doc/ru/index.md) / [Українська](/doc/ua/index.md)

# RHVoice documentation

<details>
<summary>
About the Speech synthesizer
</summary>

## Features

### Speech synthesis method

RHVoice uses statistical parametric synthesis. It relies on existing open source
speech technologies (mainly [HTS](http://hts.sp.nitech.ac.jp) and related
software). 

Voices are built from recordings of natural speech. They have small footprints,
because only statistical models are stored on users' computers. And though the
voices lack the naturalness of the synthesizers which generate speech by
combining segments of the recordings themselves, they are still very
intelligible and resemble the speakers who recorded the source material.

### Supported languages

Initially, RHVoice could speak only Russian. Now it also supports American
English, Brazilian Portuguese, Esperanto, Georgian, Ukrainian,
Kyrgyz, Tatar, Macedonian, Albanian and Polish.
In theory, it is possible to implement support for other languages, if all the
necessary resources can be found or created.

### Synthesis example

If you want to listen to an example of speech synthesis, You can use the TTS
service on [this page.](https://data2data.ru/tts/)

### Supported platforms

RHVoice supports Windows, GNU/Linux and Android. It is compatible with standard
text-to-speech interfaces on these platforms: SAPI5 on Windows,
[Speech Dispatcher](http://devel.freebsoft.org/speechd) on GNU/Linux and
Android's text-to-speech APIs. It can also be used by the
[NVDA screen reader](http://www.nvaccess.org) directly (the driver is provided
by RHVoice itself).
</details>

## Main information

* [Prebuilt binaries](Binaries.md)
* [Configuration file](Configuration-file.md)

## Compiling instructions

* [Compiling on linux](Compiling-on-Linux.md)
* [Packaging status](Packaging-status.md)

## Legal information

* [License](License.md)
* [Privacy Policy](Privacy.md)

