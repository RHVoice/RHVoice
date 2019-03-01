# -*- coding: utf-8; mode: Python; indent-tabs-mode: t -*-

import io
import re

regex=re.compile(r"^\s*([^\s=]+)\s*=\s*((:?\S+\s+)*\S+)\s*$")

def parse(path):
	props=dict()
	with io.open(path,"rt",encoding="utf-8") as f:
		for line in f:
			match=regex.match(line)
			if match:
				props[match.group(1)]=match.group(2)
	return props
