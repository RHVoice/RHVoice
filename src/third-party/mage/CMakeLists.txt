# -*- mode: Python; indent-tabs-mode: t; tab-width: 4; python-indent: 4 -*-
# Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

file(GLOB_RECURSE SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(libmage SHARED "${SRCFILES}")

set(LIBMAGE_VERSION_MAJOR "2")
set(LIBMAGE_VERSION_MINOR "0")
set(LIBMAGE_VERSION_PATCH "0")
set(LIBMAGE_VERSION "${LIBMAGE_VERSION_MAJOR}.${LIBMAGE_VERSION_MINOR}-RHVoice")
set_target_properties(libmage PROPERTIES VERSION "${LIBMAGE_VERSION}" SOVERSION "${LIBMAGE_VERSION_MAJOR}")

set(CPACK_COMPONENT_LIBMAGE_VERSION_MAJOR "${LIBMAGE_VERSION_MAJOR}" PARENT_SCOPE)
set(CPACK_COMPONENT_LIBMAGE_VERSION_MINOR "${LIBMAGE_VERSION_MINOR}" PARENT_SCOPE)
set(CPACK_COMPONENT_LIBMAGE_VERSION_PATCH "${LIBMAGE_VERSION_PATCH}" PARENT_SCOPE)
set(CPACK_COMPONENT_LIBMAGE_VERSION "${LIBMAGE_VERSION}" PARENT_SCOPE)

target_include_directories(libmage PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(libmage "libhts_engine")
harden(libmage)
set_target_properties("libmage" PROPERTIES PREFIX "")
target_compile_definitions(libmage PRIVATE RHVOICE)
install(TARGETS "libmage"
	COMPONENT "libmage"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
set("CPACK_DEBIAN_LIBMAGE_PACKAGE_NAME" "libmage" PARENT_SCOPE)
set(CPACK_DEBIAN_LIBMAGE_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIBHTS_ENGINE_PACKAGE_NAME}" PARENT_SCOPE)
