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

import struct

operators={"is":1,"=":1,"<":2,">":3,"in":4}

def is_string(obj):
	return (isinstance(obj,str) and (len(obj)>0) and (len(obj)<256))

def is_number(obj):
	return (isinstance(obj,int) and (obj>=0) and (obj<256))

def is_single_value(obj):
	return (is_string(obj) or is_number(obj))

def is_list(obj):
	return (isinstance(obj,list) and (len(obj)>0) and all(is_single_value(x) for x in obj))

def is_question(obj):
	if not isinstance(obj,list):
		return False
	if len(obj)!=3:
		return False
	if not is_string(obj[0]):
		return false
	if obj[1] in ["is","="]:
		return is_single_value(obj[2])
	elif obj[1] in ["<",">"]:
		return is_number(obj[2])
	elif obj[1]=="in":
		return is_list(obj[2])
	else:
		return False

def is_leaf(node):
	return (isinstance(node,list) and len(node)==1 and isinstance(node[0],list) and is_single_value(node[0][-1]))

def is_node(node):
	return (is_leaf(node) or (isinstance(node,list) and len(node)==3 and is_question(node[0]) and is_node(node[1]) and is_node(node[2])))

def write_single_value(dest,val,typed=False):
	if isinstance(val,str):
		if typed:
			dest.write(b"\x00")
		s=val.encode("utf-8")
		dest.write(struct.pack("{}p".format(len(s)+1),s))
	elif isinstance(val,int):
		if typed:
			dest.write(b"\x01")
		dest.write(struct.pack("B",val))
	else:
		raise RuntimeError("Unexpected value type")

def write_value(dest,val,typed=False):
	if isinstance(val,list):
		if typed:
			write_single_value(dest,2)
		write_single_value(dest,len(val))
		for obj in val:
			write_single_value(dest,obj,True)
	else:
		write_single_value(dest,val,typed)

def write_node(dest,node):
	if is_leaf(node):
		write_value(dest,0)
		write_value(dest,node[0][-1],True)
	else:
		write_value(dest,operators[node[0][1]])
		write_value(dest,node[0][0])
		write_value(dest,node[0][2],True)
		write_node(dest,node[1])
		write_node(dest,node[2])

def dump(dest,tree):
	if not is_node(tree):
		raise RuntimeError("Unsupported tree format")
	write_node(dest,tree)
