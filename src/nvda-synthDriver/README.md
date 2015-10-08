RHVoice Python module, CLI interface and NVDA plugin.

Python module is distributed as Python wheel and
tested on:

  [x] win32 + Python 2.7
  [ ] win64 + Python 2.7
  [ ] Linux + Python 2.7
  [ ] OS X + Python 2.7

## Using Python module without NVDA

First, you need to install it.

    pip install RHVoice

Read help.

    python -m RHVoice -h

Synthesize phrase and write sound into output.wav

    python RHVoice.py "Quick brown fox jumps out of the box."

Select voice for synthesizing phrases.

    python RHVoice.py --voice Irina "Читайте анонс двадцать пятого октября."

List available voices.

    python RHVoice.py list

