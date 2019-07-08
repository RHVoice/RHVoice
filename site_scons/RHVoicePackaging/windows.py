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

ns="{http://schemas.microsoft.com/wix/2006/wi}"

class msi_packager(packager):
	def __init__(self,uuid,name,outdir,env,display_name,version):
		package_name="{}-v{}-setup".format(name,version)
		super(msi_packager,self).__init__(package_name,outdir,env,"msi")
		self.display_name=display_name
		self.version=version
		self.uuid=uuid
		self.nsis_uninst_reg_key=r'Software\Microsoft\Windows\CurrentVersion\Uninstall\{}'.format(name)
		self.nsis_uninstaller_file_name="uninstall-{}.exe".format(name)
		tmp_dir=self.outdir.Dir("tmp")
		self.src=tmp_dir.Dir("src").File(name+".msi.wxs")
		self.obj=tmp_dir.Dir("obj").File(name+".msi.wixobj")
		self.root=etree.Element(ns+"Wix")
		self.root.text="\n"
		self.doc=etree.ElementTree(self.root)
		self.create_product_element()
		self.create_package_element()
		self.create_media_template_element()
		self.create_major_upgrade_element()
		self.create_nsis_uninstaller_search()
		self.create_nsis_install_location_search()
		self.create_nsis_uninstall_action()
		self.create_feature_element()
		self.create_directory_element()

	def SubElement(self,parent,tag,ns=ns,empty=False):
		el=etree.SubElement(parent,ns+tag)
		if not empty:
			el.text="\n"
		el.tail="\n"
		return el

	def create_product_element(self):
		self.product=self.SubElement(self.root,"Product")
		self.product.set("Id","*")
		self.product.set("Codepage","1252")
		self.product.set("Language","0")
		self.product.set("Manufacturer","Olga Yakovleva")
		self.product.set("Name",self.display_name)
		self.product.set("UpgradeCode",self.uuid)
		self.product.set("Version",self.version)

	def create_package_element(self):
		pkg=self.SubElement(self.product,"Package",empty=True)
		pkg.set("Compressed","yes")
		pkg.set("Description","Installs [ProductName]")
		pkg.set("InstallScope","perMachine")
		pkg.set("Manufacturer",self.product.get("Manufacturer"))
		pkg.set("SummaryCodepage",self.product.get("Codepage"))

	def create_media_template_element(self):
		mt=self.SubElement(self.product,"MediaTemplate",empty=True)
		mt.set("EmbedCab","yes")

	def create_major_upgrade_element(self):
		mu=self.SubElement(self.product,"MajorUpgrade",empty=True)
		mu.set("DowngradeErrorMessage","A newer version of [ProductName] is already installed.")
		mu.set("Schedule","afterInstallInitialize")

	def create_nsis_uninstaller_search(self):
		p=self.SubElement(self.product,"Property")
		p.set("Id","NSIS_UNINSTALLER")
		self.nsis_uninstaller_property=p
		rs=self.SubElement(p,"RegistrySearch")
		rs.set("Win64","no")
		rs.set("Id","nsis_uninstaller_registry_search")
		rs.set("Root","HKLM")
		rs.set("Key",self.nsis_uninst_reg_key)
		rs.set("Name","UninstallString")
		rs.set("Type","file")
		fs=self.SubElement(rs,"FileSearch",empty=True)
		fs.set("Id","nsis_uninstaller_file_search")
		fs.set("Name",self.nsis_uninstaller_file_name)

	def create_nsis_install_location_search(self):
		p=self.SubElement(self.product,"Property")
		p.set("Id","NSIS_INSTALL_LOCATION")
		self.nsis_install_location_property=p
		rs=self.SubElement(p,"RegistrySearch")
		rs.set("Win64","no")
		rs.set("Id","nsis_install_location_registry_search")
		rs.set("Root","HKLM")
		rs.set("Key",self.nsis_uninst_reg_key)
		rs.set("Name","InstallLocation")
		rs.set("Type","directory")
		ds=self.SubElement(rs,"DirectorySearch",empty=True)
		ds.set("Id","nsis_install_location_directory_search")
		ds.set("Path","[{}]".format(p.get("Id")))

	def create_nsis_uninstall_action(self):
		a=self.SubElement(self.product,"CustomAction",empty=True)
		a.set("Id","nsis_uninstall_action")
		a.set("Impersonate","no")
		a.set("Execute","deferred")
		a.set("Return","check")
		a.set("Property",self.nsis_uninstaller_property.get("Id"))
		a.set("ExeCommand","/S /D[{}]".format(self.nsis_install_location_property.get("Id")))
		s=self.SubElement(self.product,"InstallExecuteSequence")
		c=self.SubElement(s,"Custom")
		c.set("Action",a.get("Id"))
		c.set("After","RemoveExistingProducts")
		c.text="NOT Installed AND {} AND {}".format(self.nsis_uninstaller_property.get("Id"),self.nsis_install_location_property.get("Id"))

	def create_feature_element(self):
		f=self.SubElement(self.product,"Feature",empty=True)
		f.set("Id","Main")
		f.set("Title","Main")
		f.set("Level","1")
		f.set("Absent","disallow")

	def get_parent_directory_id(self):
		return "ProgramFilesFolder"

	def create_directory_element(self):
		dir=self.SubElement(self.product,"Directory")
		dir.set("Id","TARGETDIR")
		dir.set("Name","SourceDir")
		dir=self.SubElement(dir,"Directory")
		dir.set("Id",self.get_parent_directory_id())
		self.directory=self.SubElement(dir,"Directory")
		self.directory.set("Id","RHV")
		self.directory.set("Name","com.github.Olga-Yakovleva.RHVoice")

	def get_subdirectory_element(self,dir,path):
		p=path.split(os.sep,1)
		subdir=dir.find("*[@Name='{}']".format(p[0]))
		if subdir is None:
			subdir=self.SubElement(dir,"Directory")
			subdir.set("Id",dir.get("Id")+"_"+p[0].replace("-","").replace("_",""))
			subdir.set("Name",p[0])
		if len(p)==1:
			return subdir
		else:
			return self.get_subdirectory_element(subdir,p[1])

	def create_file_component_element(self,f):
		dir_path,file_name=os.path.split(f.outpath.lower())
		dir=self.get_subdirectory_element(self.directory,dir_path)
		cmp=self.SubElement(dir,"Component")
		file=self.SubElement(cmp,"File",empty=True)
		file.set("Id",dir.get("Id")+"_"+file_name.replace("-","").replace("_",""))
		cmp.set("Id","cmp_"+file.get("Id"))
		cmp.set("Guid","*")
		cmp.set("Feature","Main")
		file.set("KeyPath","yes")
		file.set("Name",file_name)
		file.set("Source",f.infile.abspath)
		return (cmp,file)

	def create_file_component_elements(self):
		for f in self.files:
			self.create_file_component_element(f)

	def make_src(self,target,source,env):
		self.doc.write(str(target[0]),encoding="utf-8",xml_declaration=True)

	def get_arch(self):
		return "x64" if self.is_64_bit() else "x86"

	def is_64_bit(self):
		return False

	def package(self):
		self.do_package()
		text=etree.tostring(self.root,encoding="utf-8")
		value=self.env.Value(text,text)
		src=self.env.Command(self.src,value,self.make_src)
		obj=self.env.Command(self.obj,src,r'"${WIX}bin\candle.exe" -nologo -arch '+self.get_arch()+' -out $TARGET $SOURCE')
		self.env.Command(self.outfile,obj,r'"${WIX}bin\light.exe" -nologo -sacl -out $TARGET $SOURCE')

class data_packager(msi_packager):
	def get_parent_directory_id(self):
		return "CommonAppDataFolder"

	def do_package(self):
		self.create_file_component_elements()
