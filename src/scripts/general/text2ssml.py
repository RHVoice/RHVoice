import argparse
from xml.etree.ElementTree import Element, SubElement, tostring
from xml.dom import minidom

parser = argparse.ArgumentParser()
parser.add_argument("txt", help="Plain text file that should be converted")
parser.add_argument("ssml", help="SSML file that should be produced")
parser.add_argument("-n", "--number-sentences", help="Add sentence numbers to <s> tags", action="store_true")
parser.add_argument("-l", "--language", required=True, help="SSML language")
args = parser.parse_args()

with open(args.txt, "r", encoding = "utf8") as f:
    root = Element("speak")
    root.attrib["xml:lang"] = args.language
    for index, line in enumerate(f):
        sentence = line.strip()
        if sentence:
            s = SubElement(root, "s")
            s.text = sentence
            if args.number_sentences:
                s.attrib["number"] = str(index)


result = tostring(root, encoding="UTF-8")

with open(args.ssml, "wb") as f:
    f.write(result)

print("Done")