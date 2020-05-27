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


file(GLOB_RECURSE SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# database files, included into needed places
list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/emoji_data.cpp")
list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/unidata.cpp")
list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/userdict_parser.c")


if(ENABLE_MAGE)
	set(CPACK_DEBIAN_AUDIO_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIBMAGE_PACKAGE_NAME}" PARENT_SCOPE)
else()
	list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/mage_hts_engine_impl.cpp")
endif()

add_library(RHVoice_core SHARED "${SRCFILES}")
set(libs2link "sonic" "libhts_engine")
target_include_directories(RHVoice_core PUBLIC "${INCLUDE_DIR}" PRIVATE "${HTS_LABELS_KIT_INCLUDES}")

set(package_name "RHVoice_core")

if(WIN32)
	set("DATA_PATH" "data")
	set("CONFIG_PATH" "config")
else()
	set(DATA_PATH "${data_dir}")
	set(CONFIG_PATH "${config_dir}")
endif()

set(VERSION 1.2.0)

target_compile_definitions(RHVoice_core PRIVATE DATA_PATH="${DATA_PATH}" CONFIG_PATH="${CONFIG_PATH}" VERSION="${VERSION}")

if(ENABLE_MAGE)
	target_compile_definitions(RHVoice_core PRIVATE ENABLE_MAGE)
	list(APPEND libs2link "libmage")
endif()

target_link_libraries(RHVoice_core "${libs2link}")
harden("RHVoice_core")

install(TARGETS "RHVoice_core"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "core"
)

set("CPACK_DEBIAN_CORE_PACKAGE_NAME" "librhvoice-core" PARENT_SCOPE)
