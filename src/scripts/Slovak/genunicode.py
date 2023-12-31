#!/usr/bin/python3
import os
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

# Path to the folder where this script is located in
folder = os.path.dirname(__file__)

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
	"q": "qé",
	"r": "er",
	"s": "es",
	"t": "té",
	"u": "ú",
	"v": "vé",
	"w": "dvojíté vé",
	"x": "ix",
	"y": "ypsilón",
	"z": "zet",
}

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

def errExit(msg):
	print("Error: "+msg)
	exit(1)

def appendLists(lowcaseChar, upcaseChar, nativeStr, descStr):
	lseqStr=getTranscription(descStr)
	if not len(lseqStr):
		errExit("Can not generate transcription for description '%s'" %(descStr))
	if upcaseChar and ord(upcaseChar) >=255:
		if nativeStr: translitParts.append("[%"+lowcaseChar+"|%"+upcaseChar+"] -> {"+nativeStr+"} || _ ")
	else:
		if nativeStr: translitParts.append("[%"+lowcaseChar+"] -> {"+nativeStr+"} || _ ")
	if upcaseChar and ord(upcaseChar) >=255:
		if nativeStr: translitTokParts.append("[%"+lowcaseChar+"|%"+upcaseChar+"] -> "+nativeStr[:1]+" || _ ")
	else:
		if nativeStr: translitTokParts.append("[%"+lowcaseChar+"] -> "+nativeStr[:1]+" || _ ")
	if upcaseChar and ord(upcaseChar) >=255:
		downcaseParts.append("%"+upcaseChar+" -> %"+lowcaseChar+" || _ ")
	if upcaseChar and ord(upcaseChar) >=255:
		spellParts.append("[[%"+lowcaseChar+"|%"+upcaseChar+"]:["+descStr+"]]")
	else:
		spellParts.append("[[%"+lowcaseChar+"]:["+descStr+"]]")
	lseqParts.append("%"+lowcaseChar+":["+lseqStr+"]")
	if nativeStr in graphs:
		chType=graphs[nativeStr]
	else:
		chType="c"
	graphs[lowcaseChar]=chType
	if upcaseChar and ord(upcaseChar) >=255:
		graphs[upcaseChar]=chType

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

for preformatted in latinExceptions.keys():
	lowcaseChar = preformatted
	try:
		upcaseChar = preformatted.upper()
	except:
		upcaseChar = None
	nativeStr = latinExceptions[preformatted][0]
	descStr = latinExceptions[preformatted][1]
	appendLists(lowcaseChar, upcaseChar, nativeStr, descStr)

resultFoma="""#Do not edit, file automatically generated.
#
#For g2p:
define UnicodeToNativeTranslit \n"""\
+",,\n".join(translitParts)+";\n"\
+"""

#For tok (single char to single letter):
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

fi=open("unicodechars.foma", "w")
fi.write(resultFoma)
fi.close()

fi=open("../../../data/languages/"+lang+"/graph.txt", "w")
for ch, chType in graphs.items():
	fi.write(ch+" "+chType+"\n")
fi.close()

print("Unicode characters definitions generated.\n")
print("Recompile downcase, g2p, lseq, spell and tok.foma.")
