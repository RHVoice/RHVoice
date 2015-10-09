Manual instructions how to build Python wheel for distributing
through Pyton Package Index (PyPI). This does not relate to NVDA
plugin in any way and should move into SCons tooling some day.

## Creating Windows wheel by hand 

 0. [ ] Find out your RHVoice.py version ({{version}})
 1. [ ] Build RHVoice.dll
 2. [ ] Create build dir
 3. [ ] Copy RHVoice.dll to build dir
 4. [ ] Copy RHVoice.py to build dir
 5. [ ] Create RHVoice-{{version}}.dist-info
 6. [ ] Create empty RHVoice-{{version}}.dist-info/METADATA
 7. [ ] Create empty RHVoice-{{version}}.dist-info/RECORD
 8. [ ] Create RHVoice-{{version}}.dist-info/WHEEL with content:

            Wheel-Version: 0.1

 9. [ ] Copy data/ to build dir as RHVoice.langdata
10. [ ] Pack contents of build dir into zip archive named:

            RHVoice-{{version}}-py2-none-win32.whl

Repeat the above on Windows 64.

## Creating new release on PyPI

Create PKG-INFO file from the following template:

    Metadata-Version: 1.1
    Name: RHVoice
    Version: 0.5.1.5
    Author: Olga Yakovleva
    Maintainer: anatoly techtonik
    Maintainer-email: techtonik at gmail com
    Home-page: https://github.com/techtonik/RHVoice/tree/master/src/nvda-synthDriver
    Summary: Free and open source speech synthesizer

Make sure to change **version**.

Upload it through the PyPI submit form at
https://pypi.python.org/pypi?%3Aaction=submit_form
Then add .whl package to this new version.
