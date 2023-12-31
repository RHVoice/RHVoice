# Configuration file

This file allows the user to customize the behavior of RHVoice. It is read-only
once at startup. So your changes will take effect only after you restart
the application which is using RHVoice.

On Windows, the configuration file is named `RHVoice.ini`. on other platforms it
is named `RHVoice.conf`.

## Path

### GNU/Linux

The path to the directory where the settings are stored is determined at compile
time. It can be redefined by setting the value of the `sysconfdir` variable when
you execute `scons`. By default, the value is `$prefix/etc`, which, if the
default value of the `prefix` variable is used, means `/usr/local/etc`.

The full path to the configuration file is `$sysconfdir/RHVoice/RHVoice.conf`.

### Windows

#### SAPI5

The path is `%APPDATA%\RHVoice\RHVoice.ini`.

#### NVDA

Open the folder where NVDA stores the user settings.
The `RHVoice.ini` file must be put into the `RHVoice-config` subfolder.

### Android

Depending on your device, the configuration file may be stored either in
internal memory or on an SD card. This will likely be somewhere in a subdirectory of `/storage`. 
The relative file path is
`Android/data/com.github.olga_yakovleva.rhvoice.android/files/RHVoice.conf`.
Users will often not have edit permissions to a file in this location. Instead, use the import functionality located in the app's settings, which will copy a file from a location you select to this location. This will overwrite any copy of the file previously placed in the aforementioned path.

## Format

RHVoice uses the classic ini format. But as this format is not defined by a
formal standard and implementations differ, here are some implementation details
users should keep in mind.

* The file must be saved in UTF-8.
* Semicolons are used to indicate comments. Only the whole line can be commented
  out.
* Periods are used as decimal separators.
* Characters in values may be represented with their numeric codes. The format
  is similar to numeric character references in XML, but the initial ampersand
  sign is omitted. For example, the English letter A can be written as `#65;`.
* If an option takes only two boolean values, on or off,  you can use the
  following keywords to enable the corresponding behavior: `true`, `yes`, `on`
  or `1`, and to disable it, use `false`, `no`, `off` or `0`.

## General and specific settings

Some of the options allow you to specify that they must be applied only to a
specific language or voice. And some of them are only defined for specific
languages.

### Language-specific settings

The following format is used:

```ini
languages.<language>.<key>=<value>
```

The language can be indicated by:

* Its English name
* its two-letter code defined by ISO 639-1
* its three-letter code defined by ISO 639-3.

Examples:

```ini
languages.russian.default_rate=2
languages.eng.default_rate=0.8
```

### Voice-specific settings

The following format is used:

```ini
voices.<name>.<key>=<value>
```

Example:

```ini
voices.elena.enabled=no
```

## Available options

### Rate, pitch and volume

The settings in this group determine how RHVoice interprets the values of rate,
pitch and volume it receives from client programs.

The value `1` corresponds to the standard behavior of the voices.

For example, NVDA expresses the value of speech rate as a percentage. RHVoice
interprets 50% as the default speech rate (`default_rate`), and 100% is
interpreted as the maximum speech rate (`max_rate`).

The following table lists all the options in this group.

| Setting          | Description    | Default value | Minimum value | Maximum value |
| ---------------- | -------------- | ------------- | ------------- | ------------- |
| `default_rate`   | default rate   | 1             | `min_rate`    | `max_rate`    |
| `min_rate`       | minimum rate   | 0.5           | 0.2           | 1             |
| `max_rate`       | maximum rate   | 2             | 1             | 5             |
| `default_pitch`  | default pitch  | 1             | `min_pitch`   | `max_pitch`   |
| `min_pitch`      | minimum pitch  | 0.5           | 0.5           | 1             |
| `max_pitch`      | maximum pitch  | 2             | 1             | 2             |
| `default_volume` | default volume | 1             | `min_volume`  | `max_volume`  |
| `min_volume`     | minimum volume | 0.25          | 0.25          | 1             |
| `max_volume`     | maximum volume | 2             | 1             | 4             |

These settings can be applied both to all the voices and to individual languages
or voices.

Examples:

```ini
default_volume=0.8
languages.english.default_rate=1.5
voices.alan.default_pitch=0.9
```

#### Using the Sonic library

RHVoice can use [the Sonic library](https://github.com/waywardgeek/sonic) for
changing speech rate. The native algorithm of speech rate modification in
RHVoice was improved in version 1.4.0, and this library is not included in the
builds provided by the RHVoice project. So the following setting is only
supported in custom builds with Sonic enabled.

The `min_sonic_rate` setting specifies the minimum rate
value starting from which Sonic will be used instead of the built-in algorithm.
By default the built-in algorithm is always used on Android, and on other
platforms Sonic is used to speed up speech, that is the synthesizer behaves as
if the user wrote in the configuration file:

```ini
min_sonic_rate=1
```

### Language switching and voice profiles

RHVoice can analyze the input text and change the synthesis language
automatically. This functionality is currently implemented only for those
languages whose alphabets share no letters. .

#### Voice profiles

A voice profile is just a combination of voices selected by the user. The
languages of the voices must differ. The first voice in the group is considered
the primary voice and determines the so-called primary language.

Screen readers and other client programs will include the voice profiles in the
list of available voices. But on Android voice profiles aren't configured via
this configuration file. .

A voice profile is written as a sequence of names of voices separated by the `+`
character. The `voice_profiles` setting is used to define the list of voice
profiles. Its value must be a comma-separated list of voice profiles.

Example:

```ini
voice_profiles=anna+slt,aleksandr+alan
```

#### Language switching

When one of the voice profiles is active, RHVoice will try to use the voice
which corresponds to the language of the text. The language may be set by the
client program or detected automatically. If the language cannot be detected
automatically, as is the case when reading individual numbers or names of
punctuationmarks, the primary voice will be used, that is the first voice of the
profile.

But if numbers or non-alphabetic characters appear in a sentence, and RHVoice
determines that the language of the sentence differs from the primary language,
there are two possibilities: read them with the primary voice or read the whole
sentence with one voice according to the language. By default the first
alternative is chosen, but the user can change this behavior by disabling the
`prefer_primary_language` setting:

```ini
prefer_primary_language=false
```

### Speech quality

The `quality` setting allows the user to choose one of the three available
speech quality options.  The lower the quality, the better the performance: most
importantly, the less time it will take for the synthesizer to start speaking.
The available modes are described in the following table..

| Value      | Sampling rate (kHz) | Optimized response time |
| ---------- | ------------------- | ----------------------- |
| `max`      | 24                  | no                      |
| `standard` | 24                  | yes                     |
| `min`      | 16                  | yes                     |

The standard quality is used by default, that is the synthesizer behaves as if
the user wrote in the configuration file:

```ini
quality=standard
```

### Punctuation

Despite the title, the settings in this group apply to other non-alphabetic
characters as well, even though, strictly speaking, they can't be considered
punctuation marks.

#### Punctuation mode

The `punctuation_mode` setting determines whether punctuation marks and other
symbols will be spoken. The following modes are supported:

| Value  | Description                      |
| ------ | -------------------------------- |
| `none` | don't read any symbols (default) |
| `some` | selective reading                |
| `all`  | read all symbols                 |

For example, reading of all punctuation is enabled this way:

```ini
punctuation_mode=all
```

#### Selective reading of punctuation

The `punctuation_list` setting is used to specify which symbols to speak when
the punctuation mode is set to `some`. For example:

```ini
punctuation_list=@$/\
```

#### Names of symbols

The built-in dictionary of punctuation marks and other symbols can't be called
comprehensive.  But users can define the names of additional symbols in the user
dictionary.

### Capital letters

The following settings determine whether and how the synthesizer will indicate
capital letters when it reads individual characters.

#### Capital letter indication mode

This mode is specified by the `indicate_capitals` setting. The following values
are supported:

| Value   | Description                   |
| ------- | ----------------------------- |
| `no`    | disabled (default)            |
| `word`  | say "capital" before a letter |
| `pitch` | change speech pitch           |
| `sound` | play a short sound            |

Example:

```ini
indicate_capitals=pitch
```

#### Changing pitch to indicate capital letters

If capital letters are indicated by a change in pitch, than the
`cap_pitch_factor` setting specifies how much to change the pitch in this
situation. This setting can be applied both to the synthesizer as a whole and to
individual voices.

The following example specifies a thirty percent increase:

```ini
cap_pitch_factor=1.3
```

### Stress marks

Stress indication in a text is supported only for Russian and Ukrainian
languages. For example, to tell the synthesizer that the vowel after the `+`
sign must receive stress, Add the following to the configuration file:

```ini
stress_marker=+
```

### Disabling individual languages and voices

You can disable some of the languages or voices. It may be useful if you don't
use them and don't want your screen reader to show them. The following example
disables all the English voices and Aleksandr::

```ini
languages.english.enabled=false
voices.aleksandr.enabled=false
```

### Pseudo-English mode

This mode is available only if the language doesn't use the Latin alphabet.

When they read English words, the voices can try to follow the correct English
pronunciation as much as possible. Though they can't use the correct English
sounds, but the closest sounds in their native language.

To use this mode, you need to install the English language pack. The English
voices don't need to be installed.

If the English language pack is installed, the pseudo-English mode is enabled
by
default. Using Russian as an example, you can disable it this way:

```ini
languages.russian.use_pseudo_english=no
```
