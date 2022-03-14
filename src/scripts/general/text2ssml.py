#!/usr/bin/env python

# Copyright (C) 2022 Kirill Belousov <belousov.k.m@yandex.ru>

# This script converts plain text files into SSML format, which is required by voice building scripts.

import argparse
from xml.etree.ElementTree import Element, SubElement, tostring

parser = argparse.ArgumentParser()
parser.add_argument("txt", help="Plain text file that should be converted")
parser.add_argument("ssml", help="SSML file that should be produced")
parser.add_argument("-n", "--number-sentences", help="Add sentence numbers to <s> tags", action="store_true")
parser.add_argument("-l", "--language", required=True, help="SSML language")
args = parser.parse_args()

with open(args.txt, "r", encoding = "utf8") as f:
    root = Element("speak")
    root.attrib["xml:lang"] = args.language
    number = 1
    for line in f:
        sentence = line.strip()
        if sentence:
            s = SubElement(root, "s")
            s.text = sentence
            if args.number_sentences:
                s.attrib["number"] = str(number)
            number += 1


result = tostring(root, encoding="UTF-8")

with open(args.ssml, "wb") as f:
    f.write(result)

print("Done")