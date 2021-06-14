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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import os.path
import zipfile
from SCons.Script import Value
from .common import *

def archive(target,source,env):
	with zipfile.ZipFile(str(target[0]),"w",zipfile.ZIP_DEFLATED) as f:
		for i in range(0, len(source), 3):
			infile=source[i]
			outpath=source[i+1].read()
			on_disk=source[i+2].read()
			if on_disk:
				f.write(str(infile), outpath)
			else:
				f.writestr(outpath, infile.read())

class archiver(packager):
	def __init__(self,name,outdir,env,ext="zip"):
		super(archiver,self).__init__(name,outdir,env,ext)

	def package(self):
		sources=list()
		for f in self.files:
			sources.append(f.infile)
			sources.append(Value(f.outpath))
			sources.append(Value(f.on_disk))
		return self.env.Command(self.outfile,sources,archive)[0]
