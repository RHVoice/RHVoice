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
find_package(Sonic)
set(SONIC_DIR "${CMAKE_SOURCE_DIR}/external/libs/sonic")

if(SONIC_FOUND)
	message(STATUS "Sonic found, using from the distribution packages. Set Sonic_FORCE_COMPILATION if you want to use it from a submodule.")
	set(Sonic_FORCE_COMPILATION OFF)
else()
	message(STATUS "Sonic NOT found, building an own one.")
	set(Sonic_FORCE_COMPILATION ON)
endif()

set(Sonic_FORCE_COMPILATION CACHE BOOL "Force libsonic compilation")

set(DEB_LIBSONIC_PACKAGE "libsonic0")
set(DEB_LIBSONIC_PACKAGES "${DEB_LIBSONIC_PACKAGE}, libsonic")

if(Sonic_FORCE_COMPILATION)
	add_library(sonic SHARED "${SONIC_DIR}/sonic.c")

	set(LIBSONIC_VERSION_MAJOR "0")
	set(LIBSONIC_VERSION_MINOR "1")
	set(LIBSONIC_VERSION_PATCH "17")
	set(LIBSONIC_VERSION "${LIBSONIC_VERSION_MAJOR}.${LIBSONIC_VERSION_MINOR}.${LIBSONIC_VERSION_PATCH}")

	set_target_properties(sonic PROPERTIES VERSION "${LIBSONIC_VERSION}" SOVERSION "${LIBSONIC_VERSION_MAJOR}")
	set(CPACK_COMPONENT_LIBSONIC_VERSION_MAJOR "${LIBSONIC_VERSION_MAJOR}")
	set(CPACK_COMPONENT_LIBSONIC_VERSION_MINOR "${LIBSONIC_VERSION_MINOR}")
	set(CPACK_COMPONENT_LIBSONIC_VERSION_PATCH "${LIBSONIC_VERSION_PATCH}")
	set(CPACK_COMPONENT_LIBSONIC_VERSION "${LIBSONIC_VERSION}")

	target_include_directories(sonic PUBLIC "${SONIC_DIR}")
	harden(sonic)
	add_sanitizers(sonic)
	set(CPACK_DEBIAN_LIBSONIC_PACKAGE_NAME "RHVoice-libsonic")
	set(CPACK_DEBIAN_LIBSONIC_PACKAGE_REPLACES "${DEB_LIBSONIC_PACKAGES}")
	set(CPACK_DEBIAN_LIBSONIC_PACKAGE_CONFLICTS "${CPACK_DEBIAN_LIBSONIC_PACKAGE_REPLACES}")
	set(CPACK_DEBIAN_LIBSONIC_PACKAGE_BREAKS "${CPACK_DEBIAN_LIBSONIC_PACKAGE_REPLACES}")
else()
	add_library(sonic SHARED IMPORTED)
	set_property(TARGET sonic PROPERTY IMPORTED_LOCATION "${Sonic_LIBRARIES}")
	target_include_directories(sonic INTERFACE "${Sonic_INCLUDE_DIRS}")
	set(CPACK_DEBIAN_LIBSONIC_PACKAGE_NAME "${DEB_LIBSONIC_PACKAGE}")
endif()

if(WIN32 OR Sonic_FORCE_COMPILATION)
	cpack_add_component(libsonic 
		DISPLAY_NAME "libsonic"
		DESCRIPTION "libsonic - a library for changing rate of speech. This component may contain a modified version of libsonic, for the modifications see the RHVoice GitHub repo. The original resides here: https://github.com/waywardgeek/sonic"
		GROUP "third_party_libs"
	)
	list(APPEND CPACK_COMPONENTS_ALL "libsonic")

	install(TARGETS "sonic"
		COMPONENT "libsonic"
		LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	)
endif()
pass_through_cpack_vars()
