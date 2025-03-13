#!/usr/bin/python3
import mmap
import os
import re
import subprocess
import unicodedata

lang="Slovak"
lang2="sk"

diacritic = "diakritika"
accentNames = {
	"\u0300": "prízvuk",
	"\u0301": "dĺžeň",
	"\u0302": "vokáň",
	"\u0303": "vlnovka",
	"\u0304": "makrón",
	"\u0306": "oblúčik",
	"\u0307": "bodka nad",
	"\u0308": "prehláska",
	"\u0309": "háčik nad",
	"\u030a": "krúžok",
	"\u030b": "dvojitý dĺžeň",
	"\u030c": "mäkčeň",
	"\u030f": "dvojitý prízvuk",
	"\u0311": "obrátený oblúčik",
	"\u0313": "čiarka nad",
	"\u0316": "prízvuk pod",
	"\u0317": "dĺžeň pod",
	"\u0326": "čiarka pod",
	"\u0327": "cedilla",
	"\u0328": "ogonek",
	"\u032c": "mäkčeň pod",
}

accentSuffixes = {
	"\u0300": "s prízvukom",
	"\u0301": "s dĺžňom",
	"\u0302": "s vokáňom",
	"\u0303": "s vlnovkou",
	"\u0304": "s makrónom",
	"\u0306": "s oblúčikom",
	"\u0307": "s bodkou nad",
	"\u0308": "s prehláskou",
	"\u0309": "s háčikom nad",
	"\u030a": "s krúžkom",
	"\u030b": "s dvojitým dĺžňom",
	"\u030c": "s mäkčeňom",
	"\u030f": "s dvojitým prízvukom",
	"\u0311": "s obráteným oblúčikom",
	"\u0313": "s čiarkou nad",
	"\u0316": "s prízvukom pod",
	"\u0317": "s dĺžňom pod",
	"\u0326": "s čiarkov pod",
	"\u0327": "s cedillou",
	"\u0328": "s ogoňkom",
	"\u032c": "s mäkčeňom pod",
}

fullWidth = "plná šírka"

latinExceptions = {
	'đ': ('d', 'dé so šikmým dĺžňom'),
	'ħ': ('h', 'há so šikmým dĺžňom'),
	'ı': ('i', 'í bez bodky'),
	'ĸ': ('', 'kra'),
	'ł': ('l', 'el so šikmým dĺžňom'),
	'ŋ': ('', 'eng'),
	'œ': ('', 'ligatúra oe'),
	'ŧ': ('t', 'té so šikmým dĺžňom'),
	#'ſ': ('s', 'ostré es'),
	'ƀ': ('b', 'bé so šikmým dĺžňom'),
	'ɓ': ('b', 'bé s hákom'),
	'ƃ': ('b', 'bé s čiarou na vrchu'),
	'ƅ': ('', 'tón šesť'),
	'ɔ': ('o', 'otvorené ó'),
	'ƈ': ('c', 'cé s hákom'),
	'ɖ': ('d', 'africké dé'),
	'ɗ': ('d', 'dé s hákom'),
	'ƌ': ('d', 'dé s čiarou na vrchu'),
	'ƍ': ('', 'otočená delta'),
	'ǝ': ('e', 'otočené é'),
	'ə': ('', 'šwa'),
	'ɛ': ('e', 'otvorené é'),
	#'ƒ': ('f', 'florén'),
	'ɠ': ('g', 'gé s hákom'),
	'ɣ': ('', 'gamma'),
	'ƕ': ('', 'há vé'),
	'ɩ': ('i', 'iota'),
	'ɨ': ('i', 'í so šikmým dĺžňom'),
	'ƙ': ('k', 'ká s hákom'),
	'ƚ': ('l', 'el s čiarou'),
	'ƛ': ('', 'lambda so šikmým dĺžňom'),
	'ɯ': ('m', 'otočené em'),
	'ɲ': ('n', 'en s hákom vľavo'),
	'ƞ': ('n', 'en s dlhou nožičkou vpravo'),
	'ɵ': ('o', 'ó s vlnovkou v strede'),
	'ƣ': ('', 'oi'),
	'ƥ': ('p', 'pé s hákom'),
	'ʀ': ('', 'yr'),
	'ƨ': ('', 'tón dva'),
	'ʃ': ('', 'é es há'),
	'ƪ': ('', 'obrátené é es há dookola'),
	'ƫ': ('t', 'té s predným hákom'),
	'ƭ': ('t', 'té s hákom'),
	'ʈ': ('t', 'té s ohnutým hákom'),
	'ʊ': ('', 'upsilon'),
	'ʋ': ('v', 'vé s hákom'),
	'ƴ': ('y', 'ypsylón s hákom'),
	'ƶ': ('z', 'zet so šikmým dĺžňom'),
	'ʒ': ('', 'é zet há'),
	'ƹ': ('', 'obrátené é zet há'),
	'ƺ': ('', 'é zet há s chvostíkom'),
	'ƻ': ('2', 'dva so šikmým dĺžňom'),
	'ƽ': ('', 'tón päť'),
	'ƾ': ('', 'obrátená hlasivková clona so šikmým dĺžňom'),
	#'ƿ': ('', 'LATIN LETTER WYNN'),
	'ǀ': ('', 'cvaknutie zubami'),
	'ǁ': ('', 'bočné cvaknutie'),
	'ǂ': ('', 'ďasnové cvaknutie'),
	'ǃ': ('', 'ohnuté cvaknutie'),
	'ǥ': ('g', 'gé so šikmým dĺžňom'),
	'ȝ': ('', 'yogh'),
	'ȡ': ('d', 'kučeravé dé'),
	'ȣ': ('', 'ou'),
	'ȥ': ('z', 'zet s hákom'),
	'ȴ': ('l', 'kučeravé el'),
	'ȵ': ('n', 'kučeravé en'),
	'ȶ': ('t', 'kučeravé té'),
	'ȷ': ('j', 'jé bez bodky'),
	'ȸ': ('d', 'dé bé'),
	'ȹ': ('', 'kvé pé'),
	'ⱥ': ('a', 'á so šikmým dĺžňom'),
	'ȼ': ('c', 'cé so šikmým dĺžňom'),
	'ⱦ': ('t', 'té s uhlopriečnym šikmým dĺžňom'),
	'ȿ': ('s', 'es s ozdobným chvostíkom'),
	'ɀ': ('z', 'zet s ozdobným chvostíkom'),
	'ɂ': ('', 'hlasivková clona'),
	'ʉ': ('u', 'ú s čiarou'),
	'ʌ': ('v', 'otočené vé'),
	'ɇ': ('e', 'é so šikmým dĺžňom'),
	'ɉ': ('j', 'jé so šikmým dĺžňom'),
	'ɋ': ('q', 'kvé s chvostíkom v tvare háku'),
	'ɍ': ('r', 'er so šikmým dĺžňom'),
	'ɏ': ('y', 'ypsylón so šikmým dĺžňom'),
	'ɐ': ('a', 'otočené á'),
	'ɑ': ('', 'alfa'),
	'ɒ': ('', 'otočené alfa'),
	'ɕ': ('c', 'kučeravé cé'),
	'ɘ': ('e', 'obrátené é'),
	'ɚ': ('', 'šva s hákom'),
	'ɜ': ('e', 'obrátené otvorené é'),
	'ɝ': ('e', 'obrátené otvorené é s hákom'),
	'ɞ': ('e', 'zatvorené obrátené otvorené é'),
	'ɟ': ('j', 'preškrtnuté jé bez bodky'),
	'ɡ': ('g', 'skript gé'),
	'ɢ': ('g', 'malé veľké gé'),
	'ɤ': ('', 'rams horn'),
	'ɥ': ('h', 'otočené há'),
	'ɦ': ('h', 'há s hákom'),
	'ɧ': ('', 'heng s hákom'),
	'ɪ': ('i', 'malé veľké í'),
	'ɫ': ('l', 'el s vlnovkou v prostriedku'),
	'ɬ': ('l', 'el s pásikom'),
	'ɭ': ('l', 'el s ohnutým hákom'),
	'ɮ': ('', 'lezh'),
	'ɰ': ('m', 'otočené em s dlhou nožičkou'),
	'ɱ': ('m', 'em s hákom'),
	'ɳ': ('n', 'n s ohnutým hákom'),
	'ɴ': ('n', 'malé veľké en'),
	'ɶ': ('e', 'malé veľké oé'),
	'ɷ': ('', 'zatvorené omega'),
	'ɸ': ('', 'fí'),
	'ɹ': ('r', 'otočené er'),
	'ɺ': ('r', 'otočené er s dlhou nožičkou'),
	'ɻ': ('r', 'otočené er s hákom'),
	'ɼ': ('r', 'er s dlhou nožičkou'),
	'ɽ': ('r', 'er s chôstíkom'),
	'ɾ': ('r', 'er s háčikom'),
	'ɿ': ('r', 'obrátené er s háčikom'),
	'ʁ': ('r', 'malé obrátené veľké er'),
	'ʂ': ('s', 'ess s hákom'),
	'ʄ': ('j', 'preškrtnuté jé bez bodky s hákom'),
	'ʅ': ('', 'skrčené obrátené ezh'),
	'ʆ': ('', 'kučeravé ezh'),
	'ʇ': ('t', 'otočené té'),
	'ʍ': ('w', 'otočené dvojité vé'),
	'ʎ': ('y', 'otočené ypsylón'),
	'ʏ': ('y', 'malé veľké ypsylón'),
	'ʐ': ('z', 'zet s ohnutým hákom'),
	'ʑ': ('z', 'kučeravé zet'),
	'ʓ': ('', 'kučeravé ezh'),
	'ʔ': ('', 'hlasivková clona'),
	'ʕ': ('', 'znelý hltanový frikatív'),
	'ʖ': ('', 'opačná hlasivková clona'),
	'ʗ': ('c', 'roztiahnuté cé'),
	'ʘ': ('', 'cvaknutie perami'),
	'ʙ': ('b', 'malé veľké bé'),
	'ʚ': ('e', 'zatvorené otvorené é'),
	'ʛ': ('g', 'malé veľké gé s hákom'),
	'ʜ': ('h', 'malé veľké há'),
	'ʝ': ('j', 'jé so skríženým chvostíkom'),
	'ʞ': ('k', 'otočené ká'),
	'ʟ': ('l', 'malé veľké el'),
	'ʠ': ('q', 'kvé s hákom'),
	'ʡ': ('', 'preškrtnutá hlasivková clona'),
	'ʢ': ('', 'obrátená preškrtnutá hlasivková clona'),
	'ʣ': ('', 'dvojhláska dé zet'),
	'ʤ': ('', 'dvojhláska desh'),
	'ʥ': ('', 'kučeravá dvojhláska dé zet'),
	'ʦ': ('', 'dvojhláska té es'),
	'ʧ': ('', 'dvojhláska tesh'),
	'ʨ': ('', 'kučeravá dvojhláska té cé'),
	'ʩ': ('', 'dvojhláska feng'),
	'ʪ': ('', 'dvojhláska el es'),
	'ʫ': ('', 'dvojhláska el zet'),
	'ʬ': ('', 'perný náraz'),
	'ʭ': ('', 'zubný náraz'),
	'ʮ': ('h', 'otočené há s háčikom'),
	'ʯ': ('h', 'otočené há s háčikom a chvostíkom'),
}

baseCharSpelling = {
	"a": "á",
	"b": "bé",
	"c": "cé",
	"d": "dé",
	"e": "é",
	"f": "ef",
	"g": "gé",
	"h": "há",
	"i": "í",
	"j": "jé",
	"k": "ká",
	"l": "el",
	"m": "em",
	"n": "en",
	"o": "ó",
	"p": "pé",
	"q": "kvé",
	"r": "er",
	"s": "es",
	"t": "té",
	"u": "ú",
	"v": "vé",
	"w": "dvojité vé",
	"x": "ix",
	"y": "ypsilón",
	"z": "zet",
}

fullWidthOffset = 0xfee0

# Math unicode symbols that mirror basic latin alphabet
# description is appended to the letter spelling such as a bold, a italic
# Offset is a difference between basic unicode character and math unicode symbol
# Python's builtin transformation from lowercase to uppercase and the other way round is not supported thus store two offset for each type.
unicodeMath = (
	#(description, upper case offset, lowercase offset)
	('Tučné Matematické', 0x1d39f, 0x1d3b9), # MATHEMATICAL BOLD
	('Kurzíva Matematické', 0x1d3d3, 0x1d3ed), # MATHEMATICAL ITALIC
	('Tučné Kurzíva Matematické', 0x1d407, 0x1d421), # MATHEMATICAL BOLD ITALIC
	('Skript Matematické', 0x1d43b, 0x1d455), # MATHEMATICAL SCRIPT
	('Tučné Skript Matematické', 0x1d46f, 0x1d489), # MATHEMATICAL BOLD SCRIPT
	('Fraktúra Matematické', 0x1d4a3, 0x1d4bd), # MATHEMATICAL FRAKTUR
	('Dvojnásobne vyrazené Matematické', 0x1d4d7, 0x1d4f1), # MATHEMATICAL DOUBLE-STRUCK
	('Tučné Fraktúra Matematické', 0x1d50b, 0x1d525), # MATHEMATICAL BOLD FRAKTUR
	('Bezpätkové Matematické', 0x1d53f, 0x1d559), # MATHEMATICAL SANS-SERIF
	('Tučné Bezpätkové Matematické', 0x1d573, 0x1d58d), # MATHEMATICAL SANS-SERIF BOLD
	('Kurzíva Bezpätkové Matematické', 0x1d5a7, 0x1d5c1), # MATHEMATICAL SANS-SERIF ITALIC
	('Tučné Kurzíva Bezpätkové Matematické', 0x1d5db, 0x1d5f5), # MATHEMATICAL SANS-SERIF BOLD ITALIC
	('S pevnou šírkou Matematické', 0x1d60f, 0x1d629), # MATHEMATICAL MONOSPACE
)

# Path to the folder where this script is located in
folder = os.path.dirname(__file__)

graphs = {}
# Load the original graphs
print("Loading graphs from file...")
with open(os.path.join(folder, "graph.txt"), encoding="utf-8") as f:
	lineCounter = 0
	for line in f:
		line = line.strip()
		if not line:
			continue
		try:
			(symbol, type) = line.split(" ")
		except:
			print ("Invalid text at line %d, %s"%(lineCounter, line))
			continue
		if len(symbol) !=1 or len(type) != 1:
			print ("Invalid text at line %d, %s"%(lineCounter, line))
			continue
		graphs[symbol] = type

def getTranscription(str):
	inStr="<speak xml:lang=\""+lang2+"\"><s>"+str+"</s></speak>"
	res=subprocess.run([os.path.join(folder, "../../../local/bin/RHVoice-transcribe-sentences"), "/dev/stdin", "/dev/stdout"], capture_output=True, input=inStr, text=True)
	res=res.stdout.strip()
	if res.startswith("pau "):
		res=res[4:]
	if res.endswith(" pau"):
		res=res[:-4]
	return res

def getCharSpelling(str):
	if len(str) != 1:
			return ''
	try:
		if int(str) >= 0:
			re_match = re.compile(r"%s\s?\:\s?(\w+)"%str)
	except:
		re_match = re.compile(r"\%" + r"%s\s?\:\s?\[?(\w*)\]?\s"%re.escape(str))
	with open(os.path.join(folder, "spell.foma"), 'r+') as f:
		data = mmap.mmap(f.fileno(), 0).read().decode("utf-8")
		mo = re_match.search(data)
		if mo:
			return mo.group(1)
	return ''

def errExit(msg):
	print("Error: "+msg)
	exit(1)

def appendLists(lowcaseChar, upcaseChar, nativeStr, descStr):
	lseqStr=getTranscription(descStr)
	if not len(lseqStr):
		errExit("Can not generate transcription for description '%s'" %(descStr))
	if lowcaseChar and nativeStr and not nativeStr in translitDict.keys():
		translitDict[nativeStr] = []
	try:
		nativeStrTok = nativeStr[:1]
	except:
		nativeStrTok = ''
	if lowcaseChar and nativeStrTok and not nativeStrTok in translitTokDict.keys():
		translitTokDict[nativeStrTok] = []
	if (lowcaseChar or upcaseChar) and descStr and not descStr in spellDict.keys():
		spellDict[descStr] = []
	if lowcaseChar and not lseqStr in lseqDict.keys():
		lseqDict[lseqStr] = []
	if nativeStr and lowcaseChar:
		translitDict[nativeStr].append("%"+lowcaseChar)
	if nativeStrTok and lowcaseChar:
		translitTokDict[nativeStrTok].append("%"+lowcaseChar)
	if descStr and lowcaseChar:
		spellDict[descStr].append("%"+lowcaseChar)
	if lowcaseChar:
		lseqDict[lseqStr].append("%"+lowcaseChar)
	if upcaseChar and upcaseChar != lowcaseChar and ord(upcaseChar) >=255:
		if lowcaseChar:
			downcaseParts.append("%"+upcaseChar+" -> %"+lowcaseChar+" || _ ")
		if nativeStr:
			translitDict[nativeStr].append("%"+upcaseChar)
		if nativeStrTok:
			translitTokDict[nativeStrTok].append("%"+upcaseChar)
		if descStr:
			spellDict[descStr].append("%"+upcaseChar)

	if not nativeStr:
		return
	if nativeStr in graphs:
		chType=graphs[nativeStr]
	else:
		chType="c"
	if lowcaseChar:
		graphs[lowcaseChar]=chType
	if upcaseChar and ord(upcaseChar) >=255:
		graphs[upcaseChar]=chType

def writeFoma(fileName, content):
	with open(fileName, "w") as fi:
		fi.write(content)

translitDict={}
translitTokDict={}
lseqDict={}
spellDict={}
translitParts=[]
translitTokParts=[]
downcaseParts=[]
lseqParts=[]
spellParts=[]
for i in range(ord("a"), ord("z") +1):
	baseChar = chr(i)
	if baseChar not in baseCharSpelling.keys():
		print("Missing spelling for base character '%s'"%baseChar)
		continue
	for combiningAccent in accentSuffixes.keys():
		normalized = unicodedata.normalize("NFC", baseChar +combiningAccent)
		if len(normalized) !=1: # is not composed to a single unicode character
			continue
		if normalized in graphs.keys():
			print("Skipping symbol '%s', %s. It's already included in graphs."%(normalized, unicodedata.name(normalized)))
			continue
		lowcaseChar = normalized
		upcaseChar = normalized.upper()
		if len(upcaseChar) != 1: # some capital letters don't have composed form.
			upcaseChar = ''
		nativeStr = baseChar
		descStr = " ".join((baseCharSpelling[baseChar], accentSuffixes[combiningAccent]))
		appendLists(lowcaseChar, upcaseChar, nativeStr, descStr)
		for combiningAccent2 in accentSuffixes.keys():
			normalized = unicodedata.normalize("NFC", baseChar +combiningAccent +combiningAccent2)
			if len(normalized) !=1: # is not composed to a single unicode character
				continue
			lowcaseChar = normalized
			upcaseChar = normalized.upper()
			nativeStr = baseChar
			descStr = " ".join((baseCharSpelling[baseChar], accentSuffixes[combiningAccent], "a", accentSuffixes[combiningAccent2]))
			print(descStr)
			appendLists(lowcaseChar, upcaseChar, nativeStr, descStr)
	for fontType in unicodeMath:
		lowcaseChar = chr(ord(baseChar) +fontType[2])
		lowcaseCharName = ''
		try:
			lowcaseCharName = unicodedata.name(lowcaseChar)
		except:
			lowcaseChar = ''
		upcaseChar = chr(ord(baseChar) +fontType[1])
		upcaseCharName = ''
		try:
			upcaseCharName = unicodedata.name(upcaseChar)
		except:
			upcaseChar = ''
		if not lowcaseChar and not upcaseChar:
			continue
		print(" ".join((upcaseCharName, lowcaseCharName)))
		nativeStr = baseChar
		descStr = " ".join((baseCharSpelling[baseChar], fontType[0]))
		appendLists(lowcaseChar, upcaseChar, nativeStr, descStr)

for charStr, descStr in accentNames.items():
	latinExceptions[charStr] = ('', descStr)
for charStr, preformatted in latinExceptions.items():
	lowcaseChar = charStr
	if not lowcaseChar:
		continue
	try:
		upcaseChar = lowcaseChar.upper()
	except:
		upcaseChar = ''
	if upcaseChar and upcaseChar == lowcaseChar:
		upcaseChar = ''
	nativeStr = preformatted[0]
	descStr = preformatted[1]
	appendLists(lowcaseChar, upcaseChar, nativeStr, descStr)

for num in range(33, 127):
	char = chr(num)
	if char.isupper():
		#print("Skipping uppercase letter %s"%char)
		continue
	if char in baseCharSpelling.keys():
		#print("Smal letter %s"%char)
		nativeStr = char
		lowcaseChar = chr(ord(char) +fullWidthOffset)
		try:
			upcaseChar = lowcaseChar.upper()
		except:
			upcaseChar = ''
		if upcaseChar and upcaseChar == lowcaseChar:
			upcaseChar = ''
		descStr = " ".join((baseCharSpelling[char], fullWidth))
		appendLists(lowcaseChar, upcaseChar, nativeStr, descStr)
		continue
	spelling = getCharSpelling(char)
	if spelling:
		#print("Have spelling %s"%spelling)
		nativeStr = ''
		lowcaseChar = chr(ord(char) +fullWidthOffset)
		upcaseChar = ''
		descStr = " ".join((spelling, fullWidth))
		appendLists(lowcaseChar, upcaseChar, nativeStr, descStr)
	else:
		print("Unknown %s"%char)

for nativeStr in translitDict.keys():
	if not translitDict[nativeStr]:
		continue
	translitParts.append("[" + "|".join(translitDict[nativeStr]) + "] -> {"+nativeStr+"} || _ ")
for nativeStrTok in translitTokDict.keys():
	if not translitTokDict[nativeStrTok]:
		continue
	translitTokParts.append("[" + "|".join(translitTokDict[nativeStrTok]) + "] -> "+nativeStrTok+" || _ ")
for descStr in spellDict.keys():
	if not spellDict[descStr]:
		continue
	spellParts.append("[[" + "|".join(spellDict[descStr]) + "]:["+descStr+"]]")
for lseqStr in lseqDict.keys():
	if not lseqDict[lseqStr]:
		continue
	lseqParts.append("[" + "|".join(lseqDict[lseqStr]) + "]:["+lseqStr+"]")
resultFoma="""#Do not edit, file automatically generated.
#
#For pg2p:
define NormalizeCharactersBase \n"""\
+",,\n".join(translitParts)+";\n"\
+"""

#For tok (character(s) to single letter):
define UnicodeToNativeTranslitTok \n"""\
+",,\n".join(translitTokParts)+";\n"\
+"""

#For spelling:
define UnicodeSpell \n"""\
+"|\n".join(spellParts)+";\n"\
+"""

#For lseq:
define UnicodeLseq \n"""\
+"|\n".join(lseqParts)+";\n"\
+"""

#For downcase:
define UnicodeDowncase \n"""\
+",,\n".join(downcaseParts)+";\n"

writeFoma("unicodechars.foma", resultFoma)

with open("../../../data/languages/"+lang+"/graph.txt", "w", encoding="utf-8") as fi:
	for ch, chType in graphs.items():
		if ord(ch) >= 0x1d400: # RHVoice can't decode these
			continue
		fi.write(ch+" "+chType+"\n")

print("Unicode characters definitions generated.\n")
print("Recompile downcase, pg2p, lseq, spell and tok.foma.")
