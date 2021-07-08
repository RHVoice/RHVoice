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

add_library(RHVoice_filter_manager SHARED "${SRCFILES}")
set_target_properties(RHVoice_filter_manager PROPERTIES VERSION "${RHVOICE_VERSION}" SOVERSION "${RHVOICE_VERSION_MAJOR}")
target_include_directories(RHVoice_filter_manager PUBLIC "${INCLUDE_DIR}")

set(VERSION "${RHVOICE_VERSION}")

target_link_libraries("RHVoice_filter_manager" "RHVoice_io")
harden("RHVoice_filter_manager")

cpack_add_component(filter_manager
	DISPLAY_NAME "Filter manager"
	DESCRIPTION "Provides a library that loads band-pass (in fact any filter, but in the code it is called band-pass filter) filter coefficients from a file and manages the object in memory"
	GROUP "libraries"
	DEPENDS "io"
)
list(APPEND CPACK_COMPONENTS_ALL "filter_manager")

install(TARGETS "RHVoice_filter_manager"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "filter_manager"
)

set(CPACK_COMPONENT_FILTER_MANAGER_DEPENDS "io")
set("CPACK_DEBIAN_FILTER_MANAGER_PACKAGE_NAME" "librhvoice-filter_manager")
pass_through_cpack_vars()
