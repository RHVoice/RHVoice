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
from common import *

class app_packager(packager):
	def __init__(self,name,env,display_name,version):
		package_name="{}-v{}-setup".format(name,version)
		super(app_packager,self).__init__(package_name,env,"exe")
		self.product_name=name
		self.display_name=display_name
		self.version=version
		self.script=self.outdir.File("script.nsi")
		self.data_package=False
		self.reg_values=list()

	def add_reg_value(root_key,key,name,value,x64=False):
		self.reg_values.append((root_key,key,name,value,x64))

	def add_line(self,*args):
		self.lines.append(" ".join(args))

	def gen_includes(self):
		if not self.data_package:
			self.add_line('!include "LogicLib.nsh"')
			self.add_line('!include "Library.nsh"')
			self.add_line('!include "x64.nsh"')

	def gen_settings(self):
		self.add_line("SetCompressor /solid lzma")
		self.add_line("SetOverwrite on")
		self.add_line("AllowSkipFiles off")
		self.add_line("AutoCloseWindow false")
		self.add_line("CRCCheck on")
		self.add_line("ShowInstDetails show")
		self.add_line("ShowUninstDetails show")
		self.add_line(r'InstallDir "$$PROGRAMFILES\RHVoice"')
		self.add_line('Name','"{} v{}"'.format(self.display_name,self.version))
		self.add_line('OutFile','"..\\'+os.path.basename(self.outfile.path)+'"')
		self.add_line('RequestExecutionLevel admin')

	def gen_pages(self):
		if not self.data_package:
			self.add_line('Page directory')
		self.add_line("Page instfiles")
		self.add_line("UninstPage uninstConfirm")
		self.add_line("UninstPage instfiles")

	def gen_inst_section(self):
		self.add_line('Section')
		for f in self.files:
			dir=os.path.dirname(f.outpath)
			outdir=os.path.join('$$INSTDIR',dir) if dir else '$$INSTDIR'
			self.add_line('SetOutPath','"'+outdir+'"')
			self.add_line('File',f.outpath)
		for root_key,key,name,value,x64 in self.reg_values:
			if isinstance(value,int):
				cmd='WriteRegDWORD {} "{}" "{}" {}'.format(root_key,key,name,value)
			else:
				cmd='WriteRegStr {} "{}" "{}" "{}"'.format(root_key,key,name,value)
			self.add_line(cmd)
			if self.env["enable_x64"] and x64:
				self.add_line('$${If} $${RunningX64}')
				self.add_line('SetRegView 64')
				self.add_line(cmd)
				self.add_line('SetRegView 32')
				self.add_line('$${EndIf}')
		self.add_line('SectionEnd')

	def gen_uninst_section(self):
		self.add_line('Section UnInstall')
		dirs=set(['$$INSTDIR'])
		for f in self.files:
			self.add_line('Delete','"'+os.path.join('$$INSTDIR',f.outpath)+'"')
			dir=os.path.dirname(f.outpath)
			if dir:
				dirs.add(os.path.join('$$INSTDIR',dir))
		for dir in reversed(sorted(dirs)):
			self.add_line('Rmdir','"'+dir+'"')
		reg_keys=set([(r[0],r[1],r[-1]) for r in self.reg_values])
		for root_key,key,x64 in reversed(sorted(reg_keys)):
			cmd='DeleteRegKey {} "{}"'.format(root_key,key)
			self.add_line(cmd)
			if self.env["enable_x64"] and x64:
				self.add_line('$${If} $${RunningX64}')
				self.add_line('SetRegView 64')
				self.add_line(cmd)
				self.add_line('SetRegView 32')
				self.add_line('$${EndIf}')
		self.add_line('SectionEnd')

	def package(self):
		self.lines=list()
		self.gen_includes()
		self.gen_settings()
		self.gen_pages()
		self.gen_inst_section()
		self.gen_uninst_section()
		self.env.Textfile(self.script,self.lines,TEXTFILESUFFIX=".nsi")
