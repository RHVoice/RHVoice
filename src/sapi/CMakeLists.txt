# -*- mode: Python; indent-tabs-mode: t; tab-width: 4 -*-
# Copyright (C) 2012, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com>

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


file(GLOB SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

message(STATUS "CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")

#find_package(SpeechSDK REQUIRED)


add_library(RHVoice_sapi SHARED "${SRCFILES}" "${CMAKE_CURRENT_SOURCE_DIR}/dll.def")
set_target_properties(RHVoice_sapi PROPERTIES VERSION "${RHVOICE_VERSION}" SOVERSION "${RHVOICE_VERSION_MAJOR}")
target_include_directories(RHVoice_sapi PRIVATE "${HTS_LABELS_KIT_INCLUDES}" "${INCLUDE_DIR}")
target_link_libraries(RHVoice_sapi PRIVATE "RHVoice_core" "-ladvapi32" "-lshell32")
target_compile_definitions(RHVoice_sapi PRIVATE -D_SAPI_VER=0x054)

set_target_properties(RHVoice_sapi PROPERTIES PREFIX "")
set_target_properties(RHVoice_sapi PROPERTIES OUTPUT_NAME "RHVoiceSvr")

if(MINGW)
	list(APPEND CMAKE_MODULE_PATH "${customCMakeModulesDir}/thirdparty/MinGWSAPI5SDK")
	include(MinGWSAPI5SDK)
	createMinGWSAPI5SDK(SAPI_SDK_FOR_MinGW)
	add_dependencies(RHVoice_sapi SAPI_SDK_FOR_MinGW)
	target_include_directories(RHVoice_sapi PRIVATE "${SAPI_SDK_FOR_MinGW_include}")
	target_link_libraries(RHVoice_sapi PRIVATE "SAPI_SDK_FOR_MinGW_uuids")
	#target_link_options(RHVoice_sapi PRIVATE "-municode")
elseif(MSVC)
	target_include_directories(RHVoice_sapi PRIVATE "${SpeechSDK_INCLUDE_DIRS}")
endif()

set(VERSION "${RHVOICE_VERSION}")

harden("RHVoice_sapi")

cpack_add_component(sapi
	DISPLAY_NAME "SAPI5 COM server"
	DESCRIPTION "Provides a SAPI5 server to allow other programs to use RHVoice using Speech API. It is the recommended way to use RHVoice on Windows hosts."
	GROUP "libraries"
	DEPENDS "core"
)
list(APPEND CPACK_COMPONENTS_ALL "sapi")

install(TARGETS "RHVoice_sapi"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "sapi"
)

include(CPackRegistry)

addRegistry_key("HKLM" "SOFTWARE\\Microsoft\\Speech\\Voices\\TokenEnums\\RHVoice" "CLSID" "string" "{d7577808-7ade-4dea-a5b7-ee314d6ef3a1}" "deleteSubkey")
CPackRegistryFinalize("${CMAKE_CURRENT_BINARY_DIR}" "Register.reg" "DeRegister.reg")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Register.reg" "${CMAKE_CURRENT_BINARY_DIR}/DeRegister.reg" TYPE SYSCONF COMPONENT "sapi")

pass_through_cpack_vars()
