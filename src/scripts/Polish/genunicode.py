#!/usr/bin/python3
import csv
import subprocess
lang="Polish"
lang2="pl"
graphs={
	"a":"v",
	"A":"v",
	"e":"v",
	"E":"v",
	"i":"v",
	"I":"v",
	"o":"v",
	"O":"v",
	"u":"v",
	"U":"v",
	"y":"v",
	"Y":"v",
	"ą":"v",
	"Ą":"v",
	"ę":"v",
	"Ę":"v",
	"ó":"v",
	"Ó":"v",
	"b":"c",
	"B":"c",
	"c":"c",
	"C":"c",
	"ć":"c",
	"Ć":"c",
	"d":"c",
	"D":"c",
	"f":"c",
	"F":"c",
	"g":"c",
	"G":"c",
	"h":"c",
	"H":"c",
	"j":"c",
	"J":"c",
	"k":"c",
	"K":"c",
	"l":"c",
	"L":"c",
	"ł":"c",
	"Ł":"c",
	"m":"c",
	"M":"c",
	"n":"c",
	"N":"c",
	"ń":"c",
	"Ń":"c",
	"p":"c",
	"P":"c",
	"q":"c",
	"Q":"c",
	"r":"c",
	"R":"c",
	"s":"c",
	"S":"c",
	"ś":"c",
	"Ś":"c",
	"t":"c",
	"T":"c",
	"v":"c",
	"V":"c",
	"w":"c",
	"W":"c",
	"x":"c",
	"X":"c",
	"z":"c",
	"Z":"c",
	"ź":"c",
	"Ź":"c",
	"ż":"c",
	"Ż":"c"
} #graphs

def getTranscription(str):
	inStr="<speak xml:lang=\""+lang2+"\"><s>"+str+"</s></speak>"
	res=subprocess.run(["../../../local/bin/RHVoice-transcribe-sentences", "/dev/stdin", "/dev/stdout"], capture_output=True, input=inStr, text=True)
	res=res.stdout.strip()
	if res.startswith("pau "):
		res=res[4:]
	if res.endswith(" pau"):
		res=res[:-4]
	return res

def errExit(msg):
	print("Error: "+msg)
	exit(1)

fi=open("chardesc.tsv")
charsList=csv.reader(fi, delimiter="\t")
next(charsList) #Skip header row.
lineNum=1
translitParts=[]
translitTokParts=[]
downcaseParts=[]
lseqParts=[]
spellParts=[]
for ch in charsList:
	lineNum=lineNum+1
	lowcaseChar, upcaseChar, nativeStr, descStr, *_=ch
	lowcaseChar=lowcaseChar.strip()
	if not len(lowcaseChar):
		errExit("No lowcase char in line %d" %(lineNum))
	upcaseChar=upcaseChar.strip()
	if not len(upcaseChar):
		errExit("No upcase char in line %d" %(lineNum))
	nativeStr=nativeStr.lower().strip()
	if not len(nativeStr):
		errExit("No translit in line %d" %(lineNum))
	descStr=descStr.lower().strip()
	if not len(descStr):
		errExit("No description in line %d" %(lineNum))
	lseqStr=getTranscription(descStr)
	if not len(lseqStr):
		errExit("Can not generate transcription for description  in line %d" %(lineNum))
	translitParts.append("[%"+lowcaseChar+"|%"+upcaseChar+"] -> {"+nativeStr+"} || _ ")
	translitTokParts.append("[%"+lowcaseChar+"|%"+upcaseChar+"] -> "+nativeStr[:1]+" || _ ")
	downcaseParts.append("%"+upcaseChar+" -> %"+lowcaseChar+" || _ ")
	spellParts.append("[[%"+lowcaseChar+"|%"+upcaseChar+"]:["+descStr+"]]")
	lseqParts.append("%"+lowcaseChar+":["+lseqStr+"]")
	if nativeStr in graphs:
		chType=graphs[nativeStr]
	else:
		chType="c"
	graphs[lowcaseChar]=chType
	graphs[upcaseChar]=chType
fi.close()

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
