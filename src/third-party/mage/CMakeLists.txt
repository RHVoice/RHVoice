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
add_library(libmage "${SRCFILES}")

target_include_directories(libmage PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(libmage "libhts_engine")
harden(libmage)
set_target_properties("libmage" PROPERTIES PREFIX "")
target_compile_definitions(libmage PRIVATE "-DRHVOICE=1")
install(TARGETS "libmage"
	COMPONENT "libmage"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
set("CPACK_DEBIAN_LIBMAGE_PACKAGE_NAME" "libmage" PARENT_SCOPE)
set(CPACK_DEBIAN_LIBMAGE_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIBHTS_ENGINE_PACKAGE_NAME}" PARENT_SCOPE)
