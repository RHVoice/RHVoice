# Copyright (C) 2010, 2011, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

add_library(RHVoice SHARED "${CMAKE_CURRENT_SOURCE_DIR}/lib.cpp")
set_target_properties(RHVoice PROPERTIES VERSION "${RHVOICE_VERSION}" SOVERSION "${RHVOICE_VERSION_MAJOR}")
harden("RHVoice")
add_sanitizers("RHVoice")
target_include_directories(RHVoice PRIVATE "${INCLUDE_DIR}" "${HTS_LABELS_KIT_INCLUDES}")
target_link_libraries("RHVoice" "RHVoice_core")

install(TARGETS "RHVoice"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "lib"
)

set("CPACK_DEBIAN_LIB_PACKAGE_NAME" "librhvoice" PARENT_SCOPE)
set(CPACK_DEBIAN_LIB_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, ${CPACK_DEBIAN_AUDIO_PACKAGE_NAME}"  PARENT_SCOPE)
