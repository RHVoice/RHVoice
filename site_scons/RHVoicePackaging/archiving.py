# -*- coding: utf-8; mode: Python; indent-tabs-mode: t -*-

# Copyright (C) 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

import os.path
import zipfile
from SCons.Script import Value
from .common import *

def archive(target,source,env):
	with zipfile.ZipFile(str(target[0]),"w",zipfile.ZIP_DEFLATED) as f:
		for src in source:
			infile,outfile,on_disk=src.read()
			if on_disk:
				f.write(infile,outfile)
			else:
				f.writestr(outfile,infile)

class archiver(packager):
	def __init__(self,name,outdir,env,ext="zip"):
		super(archiver,self).__init__(name,outdir,env,ext)

	def package(self):
		sources=list()
		for f in self.files:
			if f.on_disk:
				src=(f.infile.path,f.outpath,True)
			else:
				src=(f.infile.read(),f.outpath,False)
			sources.append(Value(src,src))
		return self.env.Command(self.outfile,sources,archive)[0]
