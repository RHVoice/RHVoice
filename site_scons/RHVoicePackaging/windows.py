# -*- coding: utf-8; mode: Python; indent-tabs-mode: t -*-

# Copyright (C) 2013, 2014, 2018, 2019, 2021  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

import codecs
import os.path
from collections import OrderedDict
import uuid
from lxml import etree
from .common import *

uuid_namespace=uuid.UUID(hex="de5ba08b-8b9f-46e3-b0ee-d6bbac37017c")

ns_wix="{http://schemas.microsoft.com/wix/2006/wi}"
ns_bal="{http://schemas.microsoft.com/wix/BalExtension}"

class windows_packager(packager):
	def __init__(self,name,outdir,env,display_name,version):
		self.short_name=name
		package_name="{}-v{}-setup".format(name,version)
		super(windows_packager,self).__init__(package_name,outdir.Dir(self.get_file_ext()),env,self.get_file_ext())
		self.display_name=display_name
		self.version=version
		self.tmp_dir=self.outdir.Dir("tmp")
		self.src=self.tmp_dir.Dir("src").File(name+"."+self.get_file_ext()+"."+self.get_src_ext())
		self.msi_repo=env.get("msi_repo",None)
		if self.msi_repo:
			self.msi_repo=Dir(self.msi_repo)

	def make_uuid(self,name):
		return str(uuid.uuid5(uuid_namespace,name)).upper()

class wix_packager(windows_packager):
	def __init__(self,upgrade_code,name,outdir,env,display_name,version):
		super(wix_packager,self).__init__(name,outdir,env,display_name,version)
		self.upgrade_code=upgrade_code
		self.obj=self.tmp_dir.Dir("obj").File(name+"."+self.get_file_ext()+".wixobj")
		self.root=etree.Element(ns_wix+"Wix")
		self.root.text="\n"
		self.doc=etree.ElementTree(self.root)

	def SubElement(self,parent,tag,ns=ns_wix,empty=False):
		el=etree.SubElement(parent,ns+tag)
		if not empty:
			el.text="\n"
		el.tail="\n"
		return el

	def get_arch(self):
		return "x64" if self.is_64_bit() else "x86"

	def is_64_bit(self):
		return False

	def get_src_ext(self):
		return "wxs"

	def make_src(self,target,source,env):
		self.doc.write(str(target[0]),encoding="utf-8",xml_declaration=True)

	def get_exts(self):
		return {}

	def package(self):
		if "WIX" not in self.env:
			return
		self.do_package()
		text=etree.tostring(self.root,encoding="utf-8")
		value=self.env.Value(text,text)
		src=self.env.Command(self.src,value,self.make_src)
		exts=" "+" ".join(["-ext "+ext for ext in self.get_exts()])+" "
		obj=self.env.Command(self.obj,src,r'"${WIX}\candle.exe" -nologo -arch '+self.get_arch()+exts+' -out $TARGET $SOURCE')
		self.env.Command(self.outfile,obj,r'"${WIX}\light.exe" -nologo -sacl -spdb'+exts+" -out $TARGET $SOURCE")

class msi_packager(wix_packager):
	def __init__(self,upgrade_code,name,outdir,env,display_name,version,nsis_name=None):
		super(msi_packager,self).__init__(upgrade_code,name,outdir,env,display_name,version)
		msi_file_name=os.path.split(self.outfile.path)[1]
		self.product_code=self.make_uuid("Product: {}".format(msi_file_name))
		self.visible="yes"
		self.nsis_name=nsis_name if nsis_name else name
		self.nsis_uninst_reg_key=r'Software\Microsoft\Windows\CurrentVersion\Uninstall\{}'.format(self.nsis_name)
		self.nsis_uninstaller_file_name="uninstall-{}.exe".format(self.nsis_name)
		self.old_dir_props={}
		self.create_product_element()
		self.create_package_element()
		self.create_media_template_element()
		# self.create_major_upgrade_element()
		self.configure_upgrade()
		self.create_nsis_install_location_search()
		self.create_nsis_uninstaller_search()
		self.create_no_modify_property()
		self.create_msi_fast_install_property()
		self.disable_shutdowns()
		self.create_feature_element()
		self.create_directory_element()
		self.temp_directory=None

	def get_unique_subdirectory_element(self):
		return self.directory.find(ns_wix+"Directory")

	def create_product_element(self):
		self.product=self.SubElement(self.root,"Product")
		self.product.set("Id",self.product_code)
		self.product.set("Codepage","1252")
		self.product.set("Language","0")
		self.product.set("Manufacturer", names.dev)
		self.product.set("Name",self.display_name)
		self.product.set("UpgradeCode",self.upgrade_code)
		self.product.set("Version",self.version)

	def create_package_element(self):
		pkg=self.SubElement(self.product,"Package",empty=True)
		pkg.set("Compressed","yes")
		pkg.set("Description","Installs [ProductName]")
		pkg.set("InstallScope","perMachine")
		pkg.set("Manufacturer",self.product.get("Manufacturer"))
		pkg.set("Languages",self.product.get("Language"))
		pkg.set("SummaryCodepage",self.product.get("Codepage"))

	def create_media_template_element(self):
		mt=self.SubElement(self.product,"MediaTemplate",empty=True)
		mt.set("EmbedCab","yes")

	def create_major_upgrade_element(self):
		mu=self.SubElement(self.product,"MajorUpgrade",empty=True)
		mu.set("DowngradeErrorMessage","A newer version of [ProductName] is already installed.")
		mu.set("Schedule","afterInstallInitialize")

	def create_upgrade_version_element(self):
		u=self.SubElement(self.product,"Upgrade")
		u.set("Id",self.upgrade_code)
		uv=self.SubElement(u,"UpgradeVersion",empty=True)
		return uv

	def get_exts(self):
		return ["WixUtilExtension"]

	def configure_upgrade(self):
		uv1=self.create_upgrade_version_element()
		uv1.set("Minimum","0.0.0")
		uv1.set("Maximum",self.version)
		uv1.set("Property","OLDERVERSIONBEINGUPGRADED")
		self.install_execute=self.SubElement(self.product,"InstallExecuteSequence")
		rm=self.SubElement(self.install_execute,"RemoveExistingProducts",empty=True)
		rm.set("After","InstallInitialize")
		uv2=self.create_upgrade_version_element()
		uv2.set("Minimum",self.version)
		uv2.set("OnlyDetect","yes")
		uv2.set("Property","NEWERVERSIONDETECTED")
		c=self.SubElement(self.product,"CustomActionRef",empty=True)
		c.set("Id","WixExitEarlyWithSuccess")

	def create_no_modify_property(self):
		p=self.SubElement(self.product,"Property",empty=True)
		p.set("Id","ARPNOMODIFY")
		p.set("Value","1")

	def create_msi_fast_install_property(self):
		p=self.SubElement(self.product,"Property",empty=True)
		p.set("Id","MSIFASTINSTALL")
		p.set("Value","1")

	def disable_shutdowns(self):
		p=self.SubElement(self.product,"Property",empty=True)
		p.set("Id","MSIRESTARTMANAGERCONTROL")
		p.set("Value","DisableShutdown")

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
		rs.set("Id","reg_search_old_inst")
		rs.set("Root","HKLM")
		rs.set("Key",self.nsis_uninst_reg_key)
		rs.set("Name","InstallLocation")
		rs.set("Type","directory")
		ds=self.SubElement(rs,"DirectorySearch",empty=True)
		ds.set("Id","dir_search_old_inst")
		ds.set("Path","[{}]".format(p.get("Id")))
		self.old_dir_props["."]=(p,ds)

	def create_component_search(self,str_uuid):
		cmp_uuid=uuid.UUID(hex=str_uuid)
		id="cmp_search_"+cmp_uuid.hex
		p=self.SubElement(self.product,"Property")
		p.set("Id",id.upper())
		s=self.SubElement(p,"ComponentSearch")
		s.set("Id",id)
		s.set("Guid",str_uuid)
		s.set("Type","file")
		return p

	def get_directory_path_property_element(self,path):
		if path in self.old_dir_props:
			return self.old_dir_props[path]
		parts=path.rsplit(os.sep,1)
		if len(parts)==1:
			p0,ds0=self.old_dir_props["."]
		else:
			p0,ds0=self.get_directory_path_property_element(parts[0])
		p=self.SubElement(self.product,"Property")
		dsr0=self.SubElement(p,"DirectorySearchRef")
		dsr0.set("Id",ds0.get("Id"))
		dsr0.set("Parent",ds0.getparent().get("Id"))
		dsr0.set("Path",ds0.get("Path"))
		ds=self.SubElement(dsr0,"DirectorySearch")
		ds.set("Id",dsr0.get("Id")+"_"+parts[-1].replace("-","").replace("_",""))
		ds.set("Path",parts[-1])
		ds.set("AssignToProperty","yes")
		p.set("Id",ds.get("Id").upper())
		self.old_dir_props[path]=(p,ds)
		return (p,ds)

	def create_nsis_uninstall_action(self):
		a=self.SubElement(self.product,"CustomAction",empty=True)
		a.set("Id","nsis_uninstall_action")
		a.set("Impersonate","no")
		a.set("Execute","deferred")
		a.set("Return","check")
		a.set("Property",self.nsis_uninstaller_property.get("Id"))
		a.set("ExeCommand","/S /D[{}]".format(self.nsis_install_location_property.get("Id")))
		c=self.SubElement(self.install_execute,"Custom")
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

	def get_temp_directory_element(self):
		if self.temp_directory is not None:
			return self.temp_directory
		dir=self.SubElement(self.root_directory,"Directory")
		self.temp_directory=dir
		dir.set("Id","TempFolder")
		dir=self.SubElement(dir,"Directory")
		dir.set("Id","MyTempFolder")
		dir.set("Name",names.dev_dir_name)
		dir=self.SubElement(dir,"Directory")
		dir.set("Id","RHVoiceTempFolder")
		dir.set("Name","RHVoice")
		return self.temp_directory

	def create_directory_element(self):
		dir=self.SubElement(self.product,"Directory")
		dir.set("Id","TARGETDIR")
		dir.set("Name","SourceDir")
		self.root_directory=dir
		dir=self.SubElement(dir,"Directory")
		dir.set("Id",self.get_parent_directory_id())
		dir=self.SubElement(dir,"Directory")
		dir.set("Id","MyFolder")
		dir.set("Name", names.dev_dir_name)
		self.directory=self.SubElement(dir,"Directory")
		self.directory.set("Id","RHV")
		self.directory.set("Name","RHVoice")

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

	def create_remove_file_component_element(self,file_path):
		file_path=file_path.lower()
		cmp=self.SubElement(self.get_unique_subdirectory_element(),"Component")
		cmp.set("Feature","Main")
		cmp.set("Guid",self.make_uuid("Remove old file: {} (1)".format(file_path)))
		cmp.set("KeyPath","yes")
		dir_path,file_name=os.path.split(file_path)
		dp,ds=self.get_directory_path_property_element(dir_path)
		cond=self.SubElement(cmp,"Condition")
		cond.text="NOT Installed AND {}".format(dp.get("Id"))
		rmf=self.SubElement(cmp,"RemoveFile",empty=True)
		rmf.set("Id","rm_"+file_path.replace("_","").replace("-","").replace(os.sep,"_"))
		cmp.set("Id","cmp_"+rmf.get("Id"))
		rmf.set("Name",file_name)
		rmf.set("Property",dp.get("Id"))
		rmf.set("On","install")
		return cmp

	def create_remove_folder_component_element(self,dir_path):
		dir_path=dir_path.lower()
		cmp=self.SubElement(self.get_unique_subdirectory_element(),"Component")
		cmp.set("Feature","Main")
		cmp.set("Guid",self.make_uuid("Remove old folder: {}, upgradeCode={} (1)".format(dir_path,self.upgrade_code)))
		cmp.set("KeyPath","yes")
		dp,ds=self.get_directory_path_property_element(dir_path)
		cond=self.SubElement(cmp,"Condition")
		cond.text="NOT Installed AND {}".format(dp.get("Id"))
		rmf=self.SubElement(cmp,"RemoveFolder",empty=True)
		rmf.set("Id","rm_"+dir_path.replace("_","").replace("-","").replace(os.sep,"_"))
		cmp.set("Id","cmp_"+rmf.get("Id"))
		rmf.set("Property",dp.get("Id"))
		rmf.set("On","install")

	def create_remove_folder_component_elements(self):
		for dir_path in sorted(self.old_dir_props.keys()):
			self.create_remove_folder_component_element(dir_path)

	def create_file_component_elements(self):
		for f in self.files:
			self.create_file_component_element(f)

	def create_rm_reg_key(self,cmp,key,root="HKLM",id=None):
		if not id:
			id="rm_"+root.lower()+"_"+key.lower().replace("\\","_")+"_"+("64" if cmp.get("Win64","no")=="yes" else "32")
		rk=self.SubElement(cmp,"RemoveRegistryKey")
		rk.set("Id",id)
		rk.set("Action","removeOnInstall")
		rk.set("Key",key)
		rk.set("Root",root)

	def create_rm_clsid_reg_key(self,cmp,str_clsid,id=None):
		clsid=uuid.UUID(hex=str_clsid)
		key=r"Software\Classes\CLSID\{{{}}}".format(str(clsid))
		if not id:
			id="rm_clsid_"+clsid.hex+"_"+("64" if cmp.get("Win64","no")=="yes" else "32")
		self.create_rm_reg_key(cmp,key,id=id)

	def create_remove_nsis_uninstaller_component_element(self):
		cmp=self.create_remove_file_component_element(os.path.join("uninstall",self.nsis_uninstaller_file_name))
		self.create_rm_reg_key(cmp,self.nsis_uninst_reg_key,id="rm_reg_old_uninstall_key")

	def get_file_ext(self):
		return "msi"

class data_packager(msi_packager):
	def get_parent_directory_id(self):
		return "CommonAppDataFolder"

	def get_unique_subdirectory_element(self):
		return super(data_packager,self).get_unique_subdirectory_element().find(ns_wix+"Directory").find(ns_wix+"Directory")

	def do_package(self):
		self.create_file_component_elements()
		self.create_nsis_uninstall_action()
		self.create_remove_nsis_uninstaller_component_element()

class bundle_packager(wix_packager):
	def __init__(self,upgrade_code,name,outdir,env,display_name,version):
		super(bundle_packager,self).__init__(upgrade_code,name,outdir,env,display_name,version)
		self.msis=[]
		self.create_bundle_element()
		self.configure_bootstrapper_application()

	def get_file_ext(self):
		return "exe"

	def get_exts(self):
		return ["WixBalExtension"]

	def create_bundle_element(self):
		self.bundle=self.SubElement(self.root,"Bundle")
		self.bundle.set("Manufacturer", names.dev)
		self.bundle.set("Name",self.display_name)
		self.bundle.set("UpgradeCode",self.upgrade_code)
		self.bundle.set("Version",self.version)

	def configure_bootstrapper_application(self):
		ref=self.SubElement(self.bundle,"BootstrapperApplicationRef")
		ref.set("Id","WixStandardBootstrapperApplication.HyperlinkLicense")
		ref.set(ns_bal+"UseUILanguages","yes")
		app=self.SubElement(ref,"WixStandardBootstrapperApplication",ns=ns_bal,empty=True)
		app.set("LicenseUrl","")
		app.set("ShowFilesInUse","yes")
		app.set("SuppressOptionsUI","yes")

	def do_package(self):
		self.chain=self.SubElement(self.bundle,"Chain")
		for msi in self.msis:
			self.env.Depends(self.outfile,msi.outfile)
			pkg=self.SubElement(self.chain,"MsiPackage",empty=True)
			pkg.set("Compressed","yes")
			if msi.is_64_bit():
				pkg.set("InstallCondition","VersionNT64")
			pkg.set("SourceFile",msi.outfile.abspath)
			pkg.set("Visible",msi.visible)
			pkg.set("DisplayInternalUI","yes")

class nsis_bootstrapper_packager(windows_packager):
	def __init__(self,name,outdir,env,display_name,version):
		super(nsis_bootstrapper_packager,self).__init__(name,outdir,env,display_name,version)
		self.msis=[]
		self.script=["Unicode true"]
		self.languages=["English","Russian","Portuguese","Esperanto","Georgian","Tatar","Ukrainian","Macedonian", "Albanian"]
		self.add_includes()
		self.add_settings()

	def get_file_ext(self):
		return "exe"

	def get_src_ext(self):
		return "nsi"

	def write_src(self,target,source,env):
		v=source[0].read()
		with open(str(target[0]),"wb") as f:
			f.write(codecs.BOM_UTF8)
			f.write(v)

	def add_includes(self):
		self.script.append('!include "LogicLib.nsh"')
		self.script.append('!include "x64.nsh"')
		for lang in self.languages:
			self.script.append(r'LoadLanguageFile "${{NSISDIR}}\Contrib\Language files\{}.nlf"'.format(lang))

	def add_settings(self):
		self.script.append("SetOverwrite on")
		self.script.append("AllowSkipFiles off")
		self.script.append("CRCCheck on")
		self.script.append("ShowInstDetails show")
		self.script.append(r'InstallDir "$TEMP\{}\RHVoice"'.format(names.dev_dir_name))
		self.script.append(u'Name "{} V{}"'.format(self.display_name,self.version))
		self.script.append(u'OutFile "{}"'.format(self.outfile.abspath))
		self.script.append('RequestExecutionLevel admin')
		self.script.append("Page instfiles")

	def set_ui_level(self):
		self.script.extend([
			'Var /GLOBAL UILevel',
			'StrCpy $UILevel "f"',
			'${If} ${Silent}',
			'StrCpy $UILevel "n"',
			'${EndIf}'
		])

	def add_files(self):
		self.script.append("Section")
		self.set_ui_level()
		outpath=r"$INSTDIR\packages"
		self.script.append('SetOutPath {}'.format(outpath))
		for msi in self.msis:
			file_name=os.path.split(msi.outfile.path)[1]
			log_path=os.path.join(outpath,msi.short_name+".log")
			file=msi.outfile
			if self.msi_repo:
				repo_file=self.msi_repo.File(file_name)
				if os.path.isfile(repo_file.path):
					file=repo_file
			self.env.Depends(self.outfile,file)
			file_path=outpath+"\\"+file_name
			delete_command="Delete {}".format(file_path)
			abort_command=delete_command+"\nAbort"
			if msi.is_64_bit():
				self.script.append('${If} ${RunningX64}')
			self.script.append("Delete {}".format(log_path))
			self.script.append(u"File {}".format(file.abspath))
			self.script.append("ClearErrors")
			self.script.append(r"""ExecWait 'msiexec /i "{}" /l*v "{}" /q$UILevel' $0""".format(file_path,log_path))
			self.script.append("${If} ${Errors}")
			self.script.append(abort_command)
			self.script.append("${ElseIf} $0 <> 0\n${AndIf} $0 <> 1638")
			self.script.append(abort_command)
			self.script.append("${EndIf}")
			self.script.append(delete_command)
			if msi.is_64_bit():
				self.script.append("${EndIf}")
		self.script.append("SectionEnd")

	def package(self):
		if "makensis" not in self.env:
			return
		self.add_files()
		text=u"".join([u"{}\n".format(line) for line in self.script]).encode("utf-8")
		value=self.env.Value(text,text)
		src=self.env.Command(self.src,value,self.write_src)
		self.env.Command(self.outfile,src,"$makensis $SOURCE")

