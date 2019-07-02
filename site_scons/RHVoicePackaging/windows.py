# -*- coding: utf-8; mode: Python; indent-tabs-mode: t -*-

# Copyright (C) 2013, 2014, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

import codecs
import os.path
from collections import OrderedDict
import xml.etree.cElementTree as etree
from .common import *

def __save__(target,source,env):
	v=source[0].read()
	with open(str(target[0]),"wb") as f:
#		f.write(codecs.BOM_UTF8)
		f.write(v)

ns="{http://schemas.microsoft.com/wix/2006/wi}"

def SubElement(parent,tag,ns=ns,empty=False):
	el=etree.SubElement(parent,ns+tag)
	if not empty:
		el.text="\n"
	el.tail="\n"
	return el

class data_packager(packager):
	def __init__(self,uuid,name,outdir,env,display_name,version):
		package_name="{}-v{}-setup".format(name,version)
		super(data_packager,self).__init__(package_name,outdir,env,"msi")
		self.product_name=name
		self.display_name=display_name
		self.version=version
		self.uuid=uuid
		tmp_dir=self.outdir.Dir("tmp")
		self.src=tmp_dir.Dir("src").File(name+".wxs")
		self.obj=tmp_dir.Dir("obj").File(name+".wixobj")
		self.root=etree.Element(ns+"Wix")
		self.root.text="\n"
		self.doc=etree.ElementTree(self.root)
		self.create_product_element()
		self.create_package_element()
		self.create_media_template_element()
		self.create_major_upgrade_element()
		self.create_feature_element()
		self.create_directory_element()

	def create_product_element(self):
		self.product=SubElement(self.root,"Product")
		self.product.set("Id","*")
		self.product.set("Codepage","0")
		self.product.set("Language","0")
		self.product.set("Manufacturer","Olga Yakovleva")
		self.product.set("Name",self.product_name)
		self.product.set("UpgradeCode",self.uuid)
		self.product.set("Version",self.version)

	def create_package_element(self):
		pkg=SubElement(self.product,"Package",empty=True)
		pkg.set("Compressed","yes")
		pkg.set("Description","Installs "+self.product_name)
		pkg.set("InstallScope","perMachine")
		pkg.set("Manufacturer",self.product.get("Manufacturer"))
		pkg.set("SummaryCodepage","0")

	def create_media_template_element(self):
		mt=SubElement(self.product,"MediaTemplate",empty=True)
		mt.set("EmbedCab","yes")

	def create_major_upgrade_element(self):
		mu=SubElement(self.product,"MajorUpgrade",empty=True)
		mu.set("DowngradeErrorMessage","A newer version of [ProductName] is already installed.")
		mu.set("Schedule","afterInstallInitialize")

	def create_feature_element(self):
		f=SubElement(self.product,"Feature",empty=True)
		f.set("Id","Main")
		f.set("Title","Main")
		f.set("Level","1")
		f.set("Absent","disallow")

	def create_directory_element(self):
		dir=SubElement(self.product,"Directory")
		dir.set("Id","TARGETDIR")
		dir.set("Name","SourceDir")
		dir=SubElement(dir,"Directory")
		dir.set("Id","ProgramFilesFolder")
		self.directory=SubElement(dir,"Directory")
		self.directory.set("Id","RHV")
		self.directory.set("Name","com.github.Olga-Yakovleva.RHVoice")

	def get_subdirectory_element(self,dir,path):
		p=path.split(os.sep,1)
		subdir=dir.find("*[@Name='{}']".format(p[0]))
		if subdir is None:
			subdir=SubElement(dir,"Directory")
			subdir.set("Id",dir.get("Id")+"_"+p[0].replace("-","").replace("_",""))
			subdir.set("Name",p[0])
		if len(p)==1:
			return subdir
		else:
			return self.get_subdirectory_element(subdir,p[1])

	def create_file_component_element(self,f):
		dir_path,file_name=os.path.split(f.outpath.lower())
		dir=self.get_subdirectory_element(self.directory,dir_path)
		cmp=SubElement(dir,"Component")
		file=SubElement(cmp,"File",empty=True)
		file.set("Id",dir.get("Id")+"_"+file_name.replace("-","").replace("_",""))
		cmp.set("Id","cmp_"+file.get("Id"))
		cmp.set("Guid","*")
		cmp.set("Feature","Main")
		file.set("KeyPath","yes")
		file.set("Name",file_name)
		file.set("Source",f.infile.abspath)

	def process_files(self):
		for f in self.files:
			self.create_file_component_element(f)

	def make_src(self,target,source,env):
		self.doc.write(str(target[0]),encoding="utf-8",xml_declaration=True)

	def package(self):
		self.process_files()
		text=etree.tostring(self.root,encoding="utf-8")
		value=self.env.Value(text,text)
		src=self.env.Command(self.src,value,self.make_src)
		obj=self.env.Command(self.obj,src,r'"${WIX}bin\candle.exe" -nologo -arch x86 -out $TARGET $SOURCE')
		self.env.Command(self.outfile,obj,r'"${WIX}bin\light.exe" -nologo -sacl -out $TARGET $SOURCE')
