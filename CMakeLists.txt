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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.9)
project("RHVoice")

set(customCMakeModulesDir "${CMAKE_SOURCE_DIR}/cmake")
set(thirdPartyCMakeModulesDir "${customCMakeModulesDir}/thirdParty")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${customCMakeModulesDir}" "${thirdPartyCMakeModulesDir}/sanitizers/cmake")
add_subdirectory("${thirdPartyCMakeModulesDir}/CCache")

include(FindPkgConfig)
include(GNUInstallDirs)
include(CPackComponent)
include(Hardening)
include(VersionFromGit)
find_package(Sanitizers)
getVersionFromGit("RHVOICE" "1.2.2")

if(${CMAKE_VERSION} VERSION_GREATER "3.12")
	set(CMAKE_CXX_STANDARD 20)
else()
	set(CMAKE_CXX_STANDARD 17)
endif()


set(CMAKE_C_STANDARD 11)


macro(pass_through_cpack_vars)
	get_cmake_property(cpackVarsToPassthrough VARIABLES)
	foreach(varName ${cpackVarsToPassthrough})
		if(varName MATCHES "^CPACK_")
			set("${varName}" "${${varName}}" PARENT_SCOPE)
		endif()
	endforeach()
endmacro()

if(CMAKE_INSTALL_PREFIX EQUAL "/usr")
	set(CMAKE_INSTALL_SYSCONFDIR "/etc")
endif()


set("dev" OFF CACHE BOOL "The build will only be used for development: no global installation, run from the source directory, compile helper utilities")
set("WITH_CLI11" OFF CACHE BOOL "Use CLI11 instead of bundled tclap")
set("ENABLE_SONIC" OFF CACHE BOOL "Build with libsonic.")
set("WITH_DATA" ON CACHE BOOL "Build packages with data. Long to compress. When debugging RHVoice itself it makes sense to disable it.")
#set("spd_version" validate_spd_version CACHE ??? "Speech dispatcher version")
set("release" ON CACHE BOOL "Whether we are building a release")
set("enable_xp_compat" ON CACHE BOOL "Target Windows XP")
set("msi_repo" "" CACHE PATH "Where the msi packages are kept for reuse")
set("servicedir" "${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services" CACHE PATH ".service file installation directory")

set("data_dir" "${CMAKE_INSTALL_FULL_DATADIR}/RHVoice")
set("languages_dir" "${data_dir}/languages")
set("voices_dir" "${data_dir}/voices")

set("config_dir" "${CMAKE_INSTALL_FULL_SYSCONFDIR}/RHVoice")
set("common_doc_dir" "${CMAKE_INSTALL_FULL_DATAROOTDIR}/doc" CACHE PATH "Documentation directory path")

set(SubpackagesDir "${CMAKE_CURRENT_SOURCE_DIR}/src")
set("licenses_dir" "${CMAKE_CURRENT_SOURCE_DIR}/licenses")
set("voice_licenses_dir" "${licenses_dir}/voices")

# Working around a bug with CPACK_COMPONENTS_ALL not being properly populated by populating manually
set(CPACK_COMPONENTS_ALL "")

add_subdirectory("${SubpackagesDir}")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/config")
if(WITH_DATA)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/data")
endif()

set(CPACK_PACKAGE_CHECKSUM "SHA512")
set(CPACK_GENERATOR "" CACHE STRING "Set generator for CPack")
set_property(CACHE CPACK_GENERATOR PROPERTY STRINGS "IFW;NSIS;NSIS64;NuGet;WIX;STGZ;7Z;TBZ2;TGZ;TXZ;TZ;TZST;ZIP;DEB;RPM;PackageMaker;CygwinBinary;DEB;DragNDrop;External;OSXX11;Bundle")
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
set(CPACK_NSIS_COMPRESSOR "lzma")

option(CPACK_NSIS_COMPONENT_INSTALL "Split into multiple rpm packages" ON)
option(CPACK_RPM_COMPONENT_INSTALL "Split into multiple rpm packages" ON)
option(CPACK_DEB_COMPONENT_INSTALL "Split into multiple deb packages" ON)
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)


set(CPACK_PACKAGE_VENDOR "Olga Yakovleva")
set(CPACK_PACKAGE_CONTACT "https://github.com/RHVoice/RHVoice")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/RHVoice/RHVoice")
set(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_PACKAGE_HOMEPAGE_URL}")
#set(CPACK_NSIS_MUI_FINISHPAGE_RUN "echo \'Проверка связи!\" | RHVoice-test > %CSIDL_DEFAULT_DESKTOP%/RHVoice_test.wav")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")

set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_NSIS_DISPLAY_NAME "RHVoice")
set(CPACK_DEBIAN_PACKAGE_NAME "RHVoice")
set(CPACK_PACKAGE_VERSION_MAJOR "${RHVOICE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${RHVOICE_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${RHVOICE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_TWEAK "${RHVOICE_VERSION_TWEAK}")

set(CPACK_COMPONENTS_GROUPING "IGNORE")

set(CPACK_MONOLITHIC_INSTALL OFF)
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CPACK_RESOURCE_FILE_README}")

include(CPack)
