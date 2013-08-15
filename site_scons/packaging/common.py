# -*- coding: utf-8; mode: Python; indent-tabs-mode: t; tab-width: 4; python-indent: 4 -*-

# Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from collections import defaultdict,OrderedDict
import os
import os.path
from SCons.Script import File,Dir,Entry

class file_info(object):
	def __init__(self,infile,outdir,attrs):
		inpath=infile.path
		filename=os.path.basename(inpath)
		self.outpath=os.path.join(outdir,filename) if outdir else filename
		self.infile=infile
		for name,value in attrs.iteritems():
			setattr(self,name,value)

class packager(object):
	def __init__(self,name,env,ext=None):
		self.name=name
		self.ext=ext
		self.env=env
		self.outdir=Dir(name)
		self.outfile =File(name+"."+ext) if ext else None
		self.config=OrderedDict()
		self.strings=OrderedDict()
		self.translations=defaultdict(OrderedDict)
		self.files=list()

	def add(self,obj,outdir=None,**attrs):
		if isinstance(obj,list):
			for obj0 in obj:
				self.add(obj0,outdir,**attrs)
			return
		if isinstance(obj,basestring):
			infile=File(obj)
		else:
			infile=obj
		f=file_info(infile,outdir,attrs)
		c=self.env.InstallAs(os.path.join(str(self.outdir),f.outpath),f.infile)
		if self.outfile:
			self.env.Depends(self.outfile,c)
		self.files.append(f)

	def configure(self,key,value):
		self.config[key]=value

	def set_string(self,key,value):
		self.strings[key]=value

	def translate_string(self,key,lang,value):
		self.translations[lang][key]=value

	def package(self):
		raise NotImplementedError
