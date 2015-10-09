Manual instructions how to build Python wheel. This should move
into SCons tooling one day.

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
