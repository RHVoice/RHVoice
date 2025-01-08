# -*- coding: utf-8; mode: Python; indent-tabs-mode: t -*-

# Copyright (C) 2013, 2018, 2021  Olga Yakovleva <yakovleva.o.v@gmail.com>
# Copyright (C) 2019  Beqa Gozalishvili <beqaprogger@gmail.com>

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
from .archiving import archiver
from .common import names

class addon_packager(archiver):
	def __init__(self,name,outdir,env,display_name,summary,description,version,data_package=False):
		package_name="{}-{}".format(name,version)
		super(addon_packager,self).__init__(package_name,outdir,env,"nvda-addon")
		self.set_string("name",display_name)
		self.set_string("summary",'"{}"'.format(summary))
		self.set_string("description",'"""{}"""'.format(description))
		self.set_string("author", names.def_with_email)
		self.set_string("url", names.url)
		self.set_string("version",version)
		self.set_string("minimumNVDAVersion", "2021.1.0")
		if data_package:
			self.set_string("lastTestedNVDAVersion", "2099.4.0")
		else:
			self.set_string("lastTestedNVDAVersion", "2025.1.0")

	def build_manifest(self,lang=None):
		if lang:
			properties=self.translations[lang]
			outdir=os.path.join("locale",lang)
		else:
			properties=self.strings
			outdir=None
		contents=u"".join(u"{} = {}\n".format(k,v) for k,v in properties.items()).encode("UTF-8")
		self.add("manifest.ini",outdir,contents)

	def package(self):
		self.build_manifest()
		for lang in self.translations.keys():
			self.build_manifest(lang)
		return super(addon_packager,self).package()
