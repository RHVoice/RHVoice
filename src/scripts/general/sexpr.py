# -*- coding: utf-8; mode: Python; indent-tabs-mode: t; tab-width: 4; python-indent: 4 -*-

# Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import codecs
import re
import collections

token_regex=re.compile(r"""\s*(;[^\n]*|[()']|"(?:\\.|[^\\"])*"|[^\s()"';]+)""")

def tokenize(file_path):
	tokens=collections.deque()
	with codecs.open(file_path,"r","utf-8") as f:
		chars=f.read()
		for m in token_regex.finditer(chars):
			token=m.group(1)
			if token=="'" or token[0]==";":
				continue
			tokens.append(token)
	return tokens

def atom(token):
	if token[0]=='"':
		return re.sub(r"\\(.)",r"\1",token[1:-1])
	try:
		return int(token)
	except ValueError:
		try:
			return float(token)
		except ValueError:
			return token

def parse(tokens):
	if len(tokens)==0:
		raise RuntimeError("Unexpected end of file")
	token=tokens.popleft()
	if token=="(":
		lst=list()
		while tokens[0]!=")":
			lst.append(parse(tokens))
		tokens.popleft()
		return lst
	elif token==")":
		raise RuntimeError("Unexpected )")
	else:
		return atom(token)

def read(file_path):
	return parse(tokenize(file_path))
