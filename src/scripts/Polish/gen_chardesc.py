#!/usr/bin/env python3

import os
import sys
import unicodedata


progname = os.path.basename(sys.argv[0])

if len(sys.argv) != 3:
	print(f'Usage: {progname} START_CODEPOINT END_CODEPOINT')
	sys.exit(2)

scp = sys.argv[1]
ecp = sys.argv[2]

if scp.isdigit() and ecp.isdigit():
	scp = int(scp)
	ecp = int(ecp)
else:
	scp = int(scp, base=16)
	ecp = int(ecp, base=16)

if (scp < 1 or ecp < 1) or (ecp < scp):
	sys.exit('Incorrect character range!')

with open('chardesc.tsv', 'a', encoding='UTF-8') as f:
	if f.tell() == 0:
		f.write('Lower case\tUpper case\tTranslit\tspell\tNotes\n')
	for i in range(scp, ecp+1):
		ch = chr(i)
		if ch.isupper():
			continue
		chname = unicodedata.name(ch, '')
		if not chname:
			continue
		uch = ch.upper()
		if len(uch) > 1:
			uch = ch
		f.write(f'{ch}\t{uch}\t\t\t{chname}\n')
