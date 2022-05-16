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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


file(GLOB_RECURSE SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# database files, included into needed places
list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/emoji_data.cpp")
list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/unidata.cpp")
list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/userdict_parser.c")

if(ENABLE_SONIC)
	set(CPACK_DEBIAN_CORE_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIBSONIC_PACKAGE_NAME}")
endif()

if(ENABLE_MAGE)
	list(APPEND CPACK_DEBIAN_CORE_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIBMAGE_PACKAGE_NAME}")
else()
	list(REMOVE_ITEM SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/mage_hts_engine_impl.cpp")
endif()

add_library(RHVoice_core SHARED "${SRCFILES}")
set_target_properties(RHVoice_core PROPERTIES VERSION "${RHVOICE_VERSION}" SOVERSION "${RHVOICE_VERSION_MAJOR}")
set(libs2link "libhts_engine")
target_include_directories(RHVoice_core PUBLIC "${INCLUDE_DIR}" PRIVATE "${HTS_LABELS_KIT_INCLUDES}")

set(package_name "RHVoice_core")

set(VERSION "${RHVOICE_VERSION}")

if(ENABLE_SONIC)
	list(APPEND libs2link "sonic")
endif()
set(CONFIG_PATH "${config_dir}")
set(DATA_PATH "${data_dir}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.h")

target_link_libraries(RHVoice_core "${libs2link}")
harden("RHVoice_core")
add_sanitizers("RHVoice_core")

cpack_add_component(core
	DISPLAY_NAME "Synthesis core"
	DESCRIPTION "Provides a library that does speech synthesis"
	REQUIRED
	GROUP "libraries"
	DEPENDS "libhts_engine"
)
list(APPEND CPACK_COMPONENTS_ALL "core")

install(TARGETS "RHVoice_core"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "core"
)

if(Sonic_FORCE_COMPILATION)
	list(APPEND CPACK_COMPONENT_CORE_DEPENDS "libsonic")
endif()

string(REPLACE ";" ", " CPACK_DEBIAN_CORE_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_DEPENDS}")
set("CPACK_DEBIAN_CORE_PACKAGE_NAME" "librhvoice-core")

pass_through_cpack_vars()
