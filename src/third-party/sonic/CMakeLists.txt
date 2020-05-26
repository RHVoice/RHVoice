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

add_library(sonic SHARED "${CMAKE_CURRENT_SOURCE_DIR}/sonic.c")

target_include_directories(sonic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
harden(sonic)

install(TARGETS "sonic"
	COMPONENT "libsonic"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
set(CPACK_DEBIAN_LIBSONIC_PACKAGE_NAME "libsonic" PARENT_SCOPE)