# -*- coding: utf-8; mode: Python; indent-tabs-mode: t -*-

# Copyright (C) 2013, 2014, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

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
from collections import OrderedDict
from .common import *

class app_packager(packager):
	def __init__(self,name,outdir,env,display_name,version,data_package=False):
		package_name="{}-v{}-setup".format(name,version)
		super(app_packager,self).__init__(package_name,outdir,env,"exe")
		self.product_name=name
		self.display_name=display_name
		self.version=version
		self.script=self.outdir.File(name+".nsi")
		self.data_package=data_package
		self.reg_values=list()
		self.uninstaller="uninstall-{}.exe".format(name)
		self.uninst_reg_key=r'Software\Microsoft\Windows\CurrentVersion\Uninstall\{}'.format(name)
		uninst_info=self.get_uninstall_info()
		for n,v in uninst_info.items():
			self.add_reg_value("HKLM",self.uninst_reg_key,n,v)

	def add_reg_value(self,root_key,key,name,value,x64=False):
		self.reg_values.append((root_key,key,name,value,x64))

	def add_line(self,*args):
		self.lines.append(" ".join(args))

	def get_uninstall_info(self):
		i=OrderedDict()
		i["DisplayName"]=self.display_name
		i["UninstallString"]=r'$$INSTDIR\uninstall\{}'.format(self.uninstaller)
		i["Publisher"]="Olga Yakovleva"
		i["InstallLocation"]="$$INSTDIR"
		i["DisplayVersion"]=self.version
		i["URLUpdateInfo"]="http://github.com/Olga-Yakovleva/RHVoice"
		i["NoModify"]=1
		i["NoRepair"]=1
		return i

	def gen_includes(self):
		self.add_line('!include "LogicLib.nsh"')
		if not self.data_package:
			self.add_line('!include "Library.nsh"')
			self.add_line('!include "x64.nsh"')

	def gen_language_includes(self):
		for lang in ["English","Russian"]:
			self.add_line(r'LoadLanguageFile "$${{NSISDIR}}\Contrib\Language files\{}.nlf"'.format(lang))

	def gen_settings(self):
		self.add_line("SetCompressor /solid lzma")
		self.add_line("SetOverwrite on")
		self.add_line("AllowSkipFiles off")
		self.add_line("AutoCloseWindow false")
		self.add_line("CRCCheck on")
		self.add_line("ShowInstDetails show")
		self.add_line("ShowUninstDetails show")
		self.add_line(r'InstallDir "$$PROGRAMFILES\RHVoice"')
		self.add_line(r'InstallDirRegKey HKLM "Software\RHVoice" "path"')
		self.add_line('Name','"{} v{}"'.format(self.display_name,self.version))
		self.add_line('OutFile','"'+os.path.basename(self.outfile.path)+'"')
		self.add_line('RequestExecutionLevel admin')

	def gen_pages(self):
		if not self.data_package:
			self.add_line('Page directory')
		self.add_line("Page instfiles")
		self.add_line("UninstPage uninstConfirm")
		self.add_line("UninstPage instfiles")

	def gen_uninst_prev(self):
		self.add_line('ReadRegStr $$R0 HKLM "{}" "UninstallString"'.format(self.uninst_reg_key))
		self.add_line('$${If} $$R0 != ""')
		self.add_line('ReadRegStr $$R1 HKLM "{}" "InstallLocation"'.format(self.uninst_reg_key))
		self.add_line('$${If} $$R1 != ""')
		self.add_line("ExecWait '\"$$R0\" /S _?=$$R1'")
		self.add_line('Delete "$$R0"')
		self.add_line('$${EndIf}')
		self.add_line('$${EndIf}')

	def gen_inst_section(self):
		self.add_line('Section')
		self.gen_uninst_prev()
		for f in self.files:
			srcpath=os.path.relpath(f.infile.path,self.outdir.path)
			dir,basename=os.path.split(f.outpath)
			outdir=os.path.join('$$INSTDIR',dir) if dir else '$$INSTDIR'
			self.add_line('SetOutPath','"'+outdir+'"')
			if f.get("regdll"):
				if f.get("x64"):
					self.add_line('!define LIBRARY_X64')
					self.add_line('$${If} $${RunningX64}')
				self.add_line('!insertmacro installLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED {} "{}" $$INSTDIR'.format(srcpath,basename))
				if f.get("x64"):
					self.add_line('$${EndIf}')
					self.add_line('!undef LIBRARY_X64')
			else:
				self.add_line('File',srcpath)
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
		self.add_line(r'SetOutPath $$INSTDIR\uninstall')
		self.add_line(r'WriteUninstaller "uninstall\{}"'.format(self.uninstaller))
		self.add_line('SectionEnd')

	def gen_uninst_section(self):
		self.add_line("Function un.onInit")
		self.add_line(r'ReadRegStr $$INSTDIR HKLM "Software\RHVoice" "path"')
		self.add_line("FunctionEnd")
		self.add_line('Section UnInstall')
		dirs=set(['$$INSTDIR',r'$$INSTDIR\uninstall'])
		for f in self.files:
			if f.get("regdll"):
				if f.get("x64"):
					self.add_line('!define LIBRARY_X64')
					self.add_line('$${If} $${RunningX64}')
				self.add_line(r'!insertmacro UnInstallLib REGDLL NOTSHARED NOREBOOT_NOTPROTECTED "$$INSTDIR\{}"'.format(f.outpath))
				if f.get("x64"):
					self.add_line('$${EndIf}')
					self.add_line('!undef LIBRARY_X64')
			else:
				self.add_line('Delete','"'+os.path.join('$$INSTDIR',f.outpath)+'"')
			dir=os.path.dirname(f.outpath)
			while dir:
				dirs.add(os.path.join('$$INSTDIR',dir))
				dir=os.path.dirname(dir)
		self.add_line(r'Delete "$$INSTDIR\uninstall\{}"'.format(self.uninstaller))
		for dir in reversed(sorted(dirs)):
			self.add_line('Rmdir','"'+dir+'"')
		reg_keys=set([(r[0],r[1]) for r in self.reg_values])
		reg_keys_x64=set([(r[0],r[1]) for r in self.reg_values if r[-1]])
		for root_key,key in reversed(sorted(reg_keys)):
			cmd='DeleteRegKey {} "{}"'.format(root_key,key)
			self.add_line(cmd)
			if self.env["enable_x64"] and ((root_key,key) in reg_keys_x64):
				self.add_line('$${If} $${RunningX64}')
				self.add_line('SetRegView 64')
				self.add_line(cmd)
				self.add_line('SetRegView 32')
				self.add_line('$${EndIf}')
		self.add_line('SectionEnd')

	def package(self):
		self.lines=list()
		self.gen_includes()
		self.gen_language_includes()
		self.gen_settings()
		self.gen_pages()
		self.gen_inst_section()
		self.gen_uninst_section()
		s=self.env.Textfile(self.script,self.lines,TEXTFILESUFFIX=".nsi")
		self.env.Command(self.outfile,s,"$makensis $SOURCE")
