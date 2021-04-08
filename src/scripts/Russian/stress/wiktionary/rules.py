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

import sys
import codecs
import collections
import struct

class state(object):
	def __init__(self):
		self.transitions=dict()
		self.fail=0
		self.next=0
		self.word=None
		self.data=None

class rules(object):
	def __init__(self,file_path):
		self.states=[state()]
		with codecs.open(file_path,"r","utf-8") as f:
			for line in f:
				entry=line.strip()
				if entry:
					word=entry.lower()
					n=next(i for i in xrange(len(entry)) if entry[i].isupper())
					self.add(word,n)
		self.build()

	def add(self,word,n):
		s=0
		for c in word:
			if c in self.states[s].transitions:
				s=self.states[s].transitions[c]
			else:
				self.states.append(state())
				self.states[s].transitions[c]=len(self.states)-1
				s=len(self.states)-1
		self.states[s].data=n
		self.states[s].word=word

	def go_to(self,s,c):
		t=s
		while True:
			if c in self.states[t].transitions:
				t=self.states[t].transitions[c]
				break
			elif t==0:
				break
			else:
				t=self.states[t].fail
		return t

	def build(self):
		q=collections.deque(self.states[0].transitions.itervalues())
		while q:
			s=q.popleft()
			for c,t in self.states[s].transitions.iteritems():
				q.append(t)
				f=self.go_to(self.states[s].fail,c)
				self.states[t].fail=f
				if self.states[f].data is not None:
					self.states[t].next=f
				else:
					self.states[t].next=self.states[f].next

	def match(self,word):
		matches=[None]*len(word)
		s=0
		for i in xrange(len(word)):
			c=word[i]
			s=self.go_to(s,c)
			if self.states[s].data is not None:
				o=s
			else:
				o=self.states[s].next
			while o!=0:
				st=self.states[o]
				matches[i+1-len(st.word)]=st
				o=st.next
		result=list()
		i=0
		while i<len(matches):
			if matches[i] is not None:
				st=matches[i]
				result.append((i,len(st.word),st.data))
				i+=len(st.word)
			else:
				i+=1
		return result

	def save(self,file_path):
		with open(file_path,"wb") as f:
			f.write(struct.pack(">I",len(self.states)))
			for st in self.states:
				f.write(struct.pack(">B",len(st.transitions)))
				for c,s in sorted(st.transitions.iteritems()):
					f.write(struct.pack(">II",ord(c),s))
				f.write(struct.pack(">II",st.fail,st.next))
				if st.word is not None:
					f.write(struct.pack(">BB",len(st.word),st.data))
				else:
					f.write(struct.pack(">B",0))
