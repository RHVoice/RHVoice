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

add_library(RHVoice_io SHARED "${SRCFILES}")
set_target_properties(RHVoice_io PROPERTIES VERSION "${RHVOICE_VERSION}" SOVERSION "${RHVOICE_VERSION_MAJOR}")
target_include_directories(RHVoice_io PUBLIC "${INCLUDE_DIR}" "${UTF8_INCLUDE_DIR}")
target_compile_definitions("RHVoice_io" PRIVATE "-DRHVOICE_IO_EXPORTS")

if(MINGW)
	#message(STATUS "Working around lack of VS extensions in MinGW")
	#target_compile_definitions("RHVoice_io" PRIVATE "-DMINGW_WSTRING_PATH_WORKAROUND")
endif()

harden("RHVoice_io")

cpack_add_component(io
	DISPLAY_NAME "RHVoice I/O library"
	DESCRIPTION "Common code to deal with files"
	GROUP "libraries"
)
list(APPEND CPACK_COMPONENTS_ALL "io")

install(TARGETS "RHVoice_io"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "io"
)

set("CPACK_DEBIAN_IO_PACKAGE_NAME" "librhvoice-io")

pass_through_cpack_vars()
