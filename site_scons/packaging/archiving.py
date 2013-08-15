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

import os.path
from distutils.archive_util import make_archive
from common import *

def archive(target,source,env):
	full_name=str(target[0])
	name,ext=os.path.splitext(full_name)
	dir=str(source[0])
	make_archive(name,"zip",dir)
	if ext!="zip":
		os.rename(name+".zip",full_name)

class archiver(packager):
	def __init__(self,name,env,ext="zip"):
		super(archiver,self).__init__(name,env,ext)

	def package(self):
		return self.env.Command(self.outfile,self.outdir,archive)[0]
