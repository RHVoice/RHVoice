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

import os
import os.path
from SCons.Script import Copy,Mkdir
from packaging.common import *
from packaging.archiving import archiver

package_path=os.path.dirname(__file__)
template_path=os.path.relpath(os.path.join(package_path,"template"),os.path.join(package_path,"..","..",".."))

class data_packager(packager):
	def __init__(self,name,outdir,env,language,format,revision):
		self.type ="language" if name==language else "voice"
		if self.type=="language":
			self.project_name="RHVoice-language-{}".format(name)
		else:
			self.project_name="RHVoice-voice-{}-{}".format(language,name)
		self.template_dir=Dir("#"+template_path)
		self.res_template_dir=self.template_dir.Dir("res")
		super(data_packager,self).__init__(self.project_name,outdir.Dir(self.type+"s"),env)
		self.res_outdir=self.outdir.Dir("res")
		self.arch=archiver("data",self.outdir.Dir("res").Dir("raw"),env)
		self.params={"__name__":name.lower(),
					 "__Name__":name,
					 "__type__":self.type,
					 "__language__":language.lower(),
					 "__version_code__":str(100*int(format)+int(revision)),
					 "__version_name__":"{}.{}".format(format,revision)}

	def add(self,obj,outdir=None):
		self.arch.add(obj,outdir)

	def package(self):
		self.env.Substfile(self.outdir.File("AndroidManifest.xml"),self.template_dir.File("AndroidManifest.xml"),SUBST_DICT=self.params)
		self.env.Command(self.outdir.File("project.properties"),self.template_dir.File("project.properties"),Copy("$TARGET","$SOURCE"))
		self.env.Command(self.outdir.Dir("src"),[],Mkdir("$TARGET"))
		for res_subpath in os.listdir(self.res_template_dir.abspath):
			if res_subpath.startswith("values") and os.path.isdir(os.path.join(self.res_template_dir.abspath,res_subpath)):
				filename="{__language__}-{__type__}.xml".format(**self.params)
				infile=self.res_template_dir.Dir(res_subpath).File(filename)
				outfile=self.res_outdir.Dir(res_subpath).File(filename)
				if os.path.isfile(infile.abspath):
					self.env.Substfile(outfile,infile,SUBST_DICT=self.params)
		self.arch.package()
