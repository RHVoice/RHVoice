# Copyright (C) 2010, 2011, 2012, 2013, 2014, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

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


cmake_minimum_required(VERSION 3.13 FATAL_ERROR) # target_link_directories
project("RHVoice" VERSION "1.0.2")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")
include(FindPkgConfig)
include(GNUInstallDirs)
include(CPackComponent)
include(Hardening)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)

macro(pass_through_cpack_vars)
	get_cmake_property(cpackVarsToPassthrough VARIABLES)
	foreach(varName ${cpackVarsToPassthrough})
		if(varName MATCHES "^CPACK_")
			set("${varName}" "${${varName}}" PARENT_SCOPE)
		endif()
	endforeach()
endmacro()

#def create_languages_user_var():
#    names=sorted(os.listdir(Dir("#data").Dir("languages").abspath))
#    langs=[name.lower() for name in names]
#    name_map=dict(zip(names,langs))
#    def_langs=langs
#    if sys.platform!="win32":
#        def_langs=[lang for lang in langs if lang not in["georgian"]]
#    help="Which languages to install"
#    return ListVariable("languages",help,def_langs,langs,name_map)

if(CMAKE_INSTALL_PREFIX EQUAL "/usr")
	set(CMAKE_INSTALL_SYSCONFDIR "/etc")
endif()

set("dev" OFF CACHE BOOL "The build will only be used for development: no global installation, run from the source directory, compile helper utilities")
set("ENABLE_MAGE" ON CACHE BOOL "Build with MAGE")
#set("spd_version" validate_spd_version CACHE ??? "Speech dispatcher version")
set("release" ON CACHE BOOL "Whether we are building a release")
set("enable_xp_compat" ON CACHE BOOL "Target Windows XP")
set("msi_repo" "" CACHE PATH "Where the msi packages are kept for reuse")
set("servicedir" "${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services" CACHE PATH ".service file installation directory")

set("data_dir" "${CMAKE_INSTALL_FULL_DATADIR}/RHVoice")
set("languages_dir" "${data_dir}/languages")
set("voices_dir" "${data_dir}/voices")

set("config_dir" "${CMAKE_INSTALL_FULL_SYSCONFDIR}/RHVoice")
set("common_doc_dir" "${CMAKE_INSTALL_FULL_DATAROOTDIR}/doc")

set(SubpackagesDir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set("licenses_dir" "${CMAKE_CURRENT_SOURCE_DIR}/licenses")
set("voice_licenses_dir" "${licenses_dir}/voices")

add_subdirectory("${SubpackagesDir}")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/config")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/data")

set(CPACK_PACKAGE_CHECKSUM "SHA512")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
option(CPACK_RPM_COMPONENT_INSTALL "Split into multiple rpm packages" ON)
option(CPACK_DEB_COMPONENT_INSTALL "Split into multiple deb packages" ON)
set(CPACK_PACKAGE_VENDOR "Olga Yakovleva")
set(CPACK_PACKAGE_CONTACT "https://github.com/Olga-Yakovleva/RHVoice")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Olga-Yakovleva/RHVoice")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/licenses/README")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")

set(CPACK_DEBIAN_PACKAGE_NAME "RHVoice")
set(CPACK_COMPONENTS_GROUPING "IGNORE")

set(CPACK_MONOLITHIC_INSTALL OFF)
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")

include(CPack)
