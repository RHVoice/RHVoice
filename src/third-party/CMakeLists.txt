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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


if(ENABLE_SONIC)
	add_subdirectory("sonic")
endif()

set(RAPIDXML_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rapidxml" PARENT_SCOPE)

pkg_check_modules(TCLAP tclap)
if("${TCLAP_FOUND}" AND NOT "${TCLAP_FORCE_COMPILATION}")
	set(TCLAP_INCLUDE_DIR "${TCLAP_INCLUDEDIR}" PARENT_SCOPE)
else()
	set(TCLAP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tclap" PARENT_SCOPE)
endif()

pass_through_cpack_vars()
