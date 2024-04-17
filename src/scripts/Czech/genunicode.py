#!/usr/bin/python3
import os
import subprocess
import unicodedata

lang="Czech"
lang2="cs"

diacritic = "diakritika"
accentNames = {
	"\u0300": "čárka doleva",
	"\u0301": "čárka",
	"\u0302": "stříška",
	"\u0303": "vlnovka",
	"\u0304": "makron",
	"\u0306": "oblouček",
	"\u0307": "tečka nad",
	"\u0308": "přehláska",
	"\u0309": "hák nad",
	"\u030a": "kroužek",
	"\u030b": "dvojčárka",
	"\u030c": "háček",
	"\u030f": "dvojčárka doleva",
	"\u0311": "obrácený oblouček",
	"\u0313": "čárka nad",
	"\u0316": "čárka doleva pod",
	"\u0317": "čárka doprava pod",
	"\u0326": "čárka pod",
	"\u0327": "cedilla",
	"\u0328": "ocásek",
	"\u032c": "háček pod",
}

accentSuffixes = {
	"\u0300": "s čárkou doleva",
	"\u0301": "s čárkou",
	"\u0302": "se stříškou",
	"\u0303": "s vlnovkou",
	"\u0304": "s makronem",
	"\u0306": "s obloučkem",
	"\u0307": "s tečkou nad",
	"\u0308": "s přehláskou",
	"\u0309": "s hákem nad",
	"\u030a": "s kroužkem",
	"\u030b": "s dvojčárkou",
	"\u030c": "s háčkem",
	"\u030f": "s dvojčárkou doleva",
	"\u0311": "s obráceným obloučkem",
	"\u0313": "s čárkou nad",
	"\u0316": "s čárkou doleva pod",
	"\u0317": "s čárkou doprava pod",
	"\u0326": "s čárkou pod",
	"\u0327": "s cedillou",
	"\u0328": "s ocáskem",
	"\u032c": "s háčkem pod",
}

latinExceptions = {
	'đ': ('d', 'dé přeškrtnuté'),
	'ħ': ('h', 'há přeškrtnuté'),
	'ı': ('i', 'í bez tečky'),
	'ĸ': ('', 'kra'),
	'ł': ('l', 'el přeškrtnuté'),
	'ŋ': ('', 'eng'),
	'œ': ('', 'ligatura oe'),
	'ŧ': ('t', 'té přeškrtnuté'),
	#'ſ': ('s', 'ostré es'),
	'ƀ': ('b', 'bé přeškrtnuté'),
	'ɓ': ('b', 'bé s hákem'),
	'ƃ': ('b', 'bé s čárou shora'),
	'ƅ': ('', 'tón šest'),
	'ɔ': ('o', 'otevřené ó'),
	'ƈ': ('c', 'cé s hákem'),
	'ɖ': ('d', 'africké dé'),
	'ɗ': ('d', 'dé s hákem'),
	'ƌ': ('d', 'dé s čárou shora'),
	'ƍ': ('', 'otočená delta'),
	'ǝ': ('e', 'obrácené é'),
	'ə': ('', 'šva'),
	'ɛ': ('e', 'otevřené é'),
	#'ƒ': ('f', 'florin'),
	'ɠ': ('g', 'gé s hákem'),
	'ɣ': ('', 'gama'),
	'ƕ': ('', 'há vé'),
	'ɩ': ('i', 'jota'),
	'ɨ': ('i', 'í přeškrtnuté'),
	'ƙ': ('k', 'ká s hákem'),
	'ƚ': ('l', 'el s čárou'),
	'ƛ': ('', 'lambda přeškrtnutá'),
	'ɯ': ('m', 'obrácené em'),
	'ɲ': ('n', 'en s hákem zleva'),
	'ƞ': ('n', 'en s dlouhou nožičkou zprava'),
	'ɵ': ('o', 'ó s vlnovkou uprostřed'),
	'ƣ': ('', 'oi'),
	'ƥ': ('p', 'pé s hákem'),
	'ʀ': ('', 'yr'),
	'ƨ': ('', 'tón dva'),
	'ʃ': ('', 'é es há'),
	'ƪ': ('', 'obrácené é es há dokola'),
	'ƫ': ('t', 'té s předním hákem'),
	'ƭ': ('t', 'té s hákem'),
	'ʈ': ('t', 'té s ohnutým hákem'),
	'ʊ': ('', 'upsilon'),
	'ʋ': ('v', 'vé s hákem'),
	'ƴ': ('y', 'ypsylon s hákem'),
	'ƶ': ('z', 'zet přeškrtnuté'),
	'ʒ': ('', 'éž'),
	'ƹ': ('', 'obrácené éž'),
	'ƺ': ('', 'éž s ocáskem'),
	'ƻ': ('2', 'dva s přeškrtnutím'),
	'ƽ': ('', 'tón pět'),
	'ƾ': ('', 'obrácený fonetický ráz s přeškrtnutím'),
	#'ƿ': ('', 'latinské písmeno WYNN'),
	'ǀ': ('', 'cvaknutí zuby'),
	'ǁ': ('', 'boční cvaknutí'),
	'ǂ': ('', 'dásňové cvaknutí'),
	'ǃ': ('', 'ohnuté cvaknutí'),
	'ǥ': ('g', 'gé přeškrtnuté'),
	'ȝ': ('', 'yogh'),
	'ȡ': ('d', 'kudrnaté dé'),
	'ȣ': ('', 'ou'),
	'ȥ': ('z', 'zet s hákem'),
	'ȴ': ('l', 'kudrnaté el'),
	'ȵ': ('n', 'kudrnaté en'),
	'ȶ': ('t', 'kudrnaté té'),
	'ȷ': ('j', 'jé bez tečky'),
	'ȸ': ('d', 'dé bé'),
	'ȹ': ('', 'kvé pé'),
	'ⱥ': ('a', 'á přeškrtnuté'),
	'ȼ': ('c', 'cé přeškrtnuté'),
	'ⱦ': ('t', 'té přeškrtnuté úhlopříčně'),
	'ȿ': ('s', 'es s ozdobným ocáskem'),
	'ɀ': ('z', 'zet s ozdobným ocáskem'),
	'ɂ': ('', 'fonetický ráz'),
	'ʉ': ('u', 'ú s čárou'),
	'ʌ': ('v', 'obrácené vé'),
	'ɇ': ('e', 'é přeškrtnuté'),
	'ɉ': ('j', 'jé přeškrtnuté'),
	'ɋ': ('q', 'kvé s ocáskem ve tvaru háku'),
	'ɍ': ('r', 'er přeškrtnuté'),
	'ɏ': ('y', 'ypsylon přeškrtnuté'),
	'ɐ': ('a', 'obrácené á'),
	'ɑ': ('', 'alfa'),
	'ɒ': ('', 'obrácená alfa'),
	'ɕ': ('c', 'kudrnaté cé'),
	'ɘ': ('e', 'obrátené é'),
	'ɚ': ('', 'šva s hákem'),
	'ɜ': ('e', 'obrácené otevřené é'),
	'ɝ': ('e', 'obrácené otevřené é s hákem'),
	'ɞ': ('e', 'zavřené obrácené otevřené é'),
	'ɟ': ('j', 'přeškrtnuté jé bez tečky'),
	'ɡ': ('g', 'skript gé'),
	'ɢ': ('g', 'malé velké gé'),
	'ɤ': ('', 'rams horn'),
	'ɥ': ('h', 'obrácené há'),
	'ɦ': ('h', 'há s hákem'),
	'ɧ': ('', 'heng s hákem'),
	'ɪ': ('i', 'malé velké í'),
	'ɫ': ('l', 'el s vlnovkou uprostřed'),
	'ɬ': ('l', 'el s páskem'),
	'ɭ': ('l', 'el s ohnutým hákem'),
	'ɮ': ('', 'lezh'),
	'ɰ': ('m', 'obrácené em s dlouhou nožičkou'),
	'ɱ': ('m', 'em s hákem'),
	'ɳ': ('n', 'n s ohnutým hákem'),
	'ɴ': ('n', 'malé velké en'),
	'ɶ': ('e', 'malé velké oé'),
	'ɷ': ('', 'zavřená omega'),
	'ɸ': ('', 'fí'),
	'ɹ': ('r', 'obrácené er'),
	'ɺ': ('r', 'obrácené er s dlouhou nožičkou'),
	'ɻ': ('r', 'obrácené er s hákem'),
	'ɼ': ('r', 'er s dlouhou nožičkou'),
	'ɽ': ('r', 'er s ocáskem'),
	'ɾ': ('r', 'er s malým hákem'),
	'ɿ': ('r', 'obrácené er s malým hákem'),
	'ʁ': ('r', 'malé obrácené velké er'),
	'ʂ': ('s', 'ess s hákem'),
	'ʄ': ('j', 'přeškrtnuté jé bez tečky s hákem'),
	'ʅ': ('', 'skrčené obrácené ezh'),
	'ʆ': ('', 'kudrnaté ezh'),
	'ʇ': ('t', 'obrácené té'),
	'ʍ': ('w', 'obrácené dvojité vé'),
	'ʎ': ('y', 'obrácené ypsylon'),
	'ʏ': ('y', 'malé velké ypsylón'),
	'ʐ': ('z', 'zet s ohnutým hákem'),
	'ʑ': ('z', 'kudrnaté zet'),
	'ʓ': ('', 'kudrnaté ezh'),
	'ʔ': ('', 'fonetický ráz'),
	'ʕ': ('', 'znělá hrdelní frikativa'),
	'ʖ': ('', 'obrácený fonetický ráz'),
	'ʗ': ('c', 'roztažené cé'),
	'ʘ': ('', 'cvaknutí rty'),
	'ʙ': ('b', 'malé velké bé'),
	'ʚ': ('e', 'zavřené otevřené é'),
	'ʛ': ('g', 'malé velké gé s hákem'),
	'ʜ': ('h', 'malé velké há'),
	'ʝ': ('j', 'jé se skříženým ocáskem'),
	'ʞ': ('k', 'obrácené ká'),
	'ʟ': ('l', 'malé velké el'),
	'ʠ': ('q', 'kvé s hákem'),
	'ʡ': ('', 'přeškrtnutý fonetický ráz'),
	'ʢ': ('', 'obrácený přeškrtnutý fonetický ráz'),
	'ʣ': ('', 'dvojhláska dé zet'),
	'ʤ': ('', 'dvojhláska desh'),
	'ʥ': ('', 'kudrnatá dvojhláska dé zet'),
	'ʦ': ('', 'dvojhláska té es'),
	'ʧ': ('', 'dvojhláska tesh'),
	'ʨ': ('', 'kudrnatá dvojhláska té cé'),
	'ʩ': ('', 'dvojhláska feng'),
	'ʪ': ('', 'dvojhláska el es'),
	'ʫ': ('', 'dvojhláska el zet'),
	'ʬ': ('', 'retní náraz'),
	'ʭ': ('', 'zubní náraz'),
	'ʮ': ('h', 'obrácené há s malým hákem'),
	'ʯ': ('h', 'obrácené há s malým hákem a ocáskem'),
}

# Path to the folder where this script is located in
folder = os.path.dirname(__file__)

baseCharSpelling = {
	"a": "a",
	"b": "bé",
	"c": "cé",
	"d": "dé",
	"e": "e",
	"f": "ef",
	"g": "gé",
	"h": "há",
	"i": "i",
	"j": "jé",
	"k": "ká",
	"l": "el",
	"m": "em",
	"n": "en",
	"o": "o",
	"p": "pé",
	"q": "kvé",
	"r": "er",
	"s": "es",
	"t": "té",
	"u": "ú",
	"v": "vé",
	"w": "dvojité vé",
	"x": "ix",
	"y": "ypsilon",
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
