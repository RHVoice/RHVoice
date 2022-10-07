#!/usr/bin/env python3

import os
import sys
import csv
import unicodedata

descriptionFragments={
	"WITH ACUTE": "ostre",
	"WITH BREVE": "z łuczkiem",
	"WITH CARON": "z haczkiem",
	"WITH CEDILLA AND ACUTE": "ostre z cedyllą",
	"WITH CEDILLA": "z cedyllą",
	"WITH CIRCUMFLEX": "z daszkiem",
	"WITH GRAVE": "ciężkie",
	"WITH MACRON": "z makronem",
	"WITH RING ABOVE": "z kółkiem",
	"with stroke": "skreślone",
	"with tilde": "z tyldą",
	"latin small letter ": "",
	"latin capital letter ": "",
} #descriptionFragments

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
fi=open("chardesc.tsv", encoding='UTF-8')
definedChars={}
charsList=csv.reader(fi, delimiter="\t")
next(charsList) #Skip header row.
for ch in charsList:
	lowcaseChar, upcaseChar, *_=ch
	lowcaseChar=lowcaseChar.strip()
	if len(lowcaseChar):
		definedChars[lowcaseChar]=True
	upcaseChar=upcaseChar.strip()
	if len(upcaseChar):
		definedChars[upcaseChar]=True
fi.close()

with open('chardescnew.tsv', 'w', encoding='UTF-8') as f:
	if f.tell() == 0:
		f.write('Lower case\tUpper case\tTranslit\tspell\tNotes\n')
	for i in range(scp, ecp+1):
		ch = chr(i)
		if ch in definedChars:
			continue
		if ch.isupper():
			continue
		chname = unicodedata.name(ch, '')
		if not chname:
			continue
		uch = ch.upper()
		if len(uch) > 1:
			uch = ch
		chname=chname.lower()
		for k in descriptionFragments.keys():
			chname=chname.replace(k.lower(), descriptionFragments[k].lower())
		f.write(f'{ch}\t{uch}\t\t\t{chname}\n')
