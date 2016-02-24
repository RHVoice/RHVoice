# -*- coding: utf-8; mode: Python; indent-tabs-mode: t; tab-width: 4; python-indent: 4 -*-

import sys
import json
import numpy
from scipy.signal import firwin

def lpf(sr):
	with open("training.cfg","r") as f:
		settings=json.load(f)
		mvf=settings.get("mvf",6000)
	n=99
	coefs=firwin(n,mvf,nyq=(sr/2))
	return coefs

if __name__=="__main__":
	sr=float(sys.argv[1])
	do_hpf=(sys.argv[2]=="1")
	coefs=lpf(sr)
	n=len(coefs)
	if do_hpf:
		apf=numpy.zeros(n)
		apf[n/2]=1
		coefs=apf-coefs
	print(" ".join(["{}".format(c) for c in coefs]))
