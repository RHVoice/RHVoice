# -*- coding: utf-8; mode: Python; indent-tabs-mode: t -*-

# Copyright (C) 2013, 2018, 2021  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

from collections import defaultdict,OrderedDict
import os
import os.path
from SCons.Script import File,Dir,Entry,Value

try:
	base_string_type=basestring
except NameError:
	base_string_type=str

class file_info(object):
	def __init__(self,infile,outdir,contents,attrs):
		global base_string_type
		if contents is not None:
			self.infile=Value(contents,contents)
			self.on_disk=False
			filename=infile
		else:
			self.infile=File(infile) if isinstance(infile,base_string_type) else infile
			self.on_disk=True
			filename=os.path.basename(self.infile.path)
		self.outpath=os.path.join(outdir,filename) if outdir else filename
		for name,value in attrs.items():
			setattr(self,name,value)

	def get(self,name):
		return getattr(self,name,None)

class packager(object):
	def __init__(self,name,outdir,env,ext=None):
		self.name=name
		self.ext=ext
		self.env=env
		if ext:
			self.outdir=outdir
			self.outfile=self.outdir.File(name+"."+ext)
		else:
			self.outdir=outdir.Dir(name)
			self.outfile=None
		self.config=OrderedDict()
		self.strings=OrderedDict()
		self.translations=defaultdict(OrderedDict)
		self.files=list()

	def add(self,obj,outdir=None,contents=None,**attrs):
		if isinstance(obj,list):
			for obj0 in obj:
				self.add(obj0,outdir,None,**attrs)
			return
		f=file_info(obj,outdir,contents,attrs)
		if self.outfile:
			self.env.Depends(self.outfile,f.infile)
		self.files.append(f)

	def configure(self,key,value):
		self.config[key]=value

	def set_string(self,key,value):
		self.strings[key]=value

	def translate_string(self,key,lang,value):
		self.translations[lang][key]=value

	def package(self):
		raise NotImplementedError

class names:
	dev="Contributors to the RHVoice project"
	dev_dir_name="rhvoice.org"
	email="rhvoice@rhvoice.org"
	def_with_email="{} <{}>".format(dev, email)
	src_url="https://github.com/RHVoice/RHVoice"
	url="https://rhvoice.org"
