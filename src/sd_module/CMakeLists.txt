# -*- mode: Python; indent-tabs-mode: t; tab-width: 4 -*-
# Copyright (C) 2012, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

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
add_executable(sd_rhvoice "${SRCFILES}")
target_include_directories(sd_rhvoice PRIVATE "${INCLUDE_DIR}" "${HTS_LABELS_KIT_INCLUDES}")
target_link_libraries(sd_rhvoice "RHVoice_core" "RHVoice_audio" pthread)
harden(sd_rhvoice)

set(SPEECH_DISPATCHER_MODULES_DIR "${CMAKE_INSTALL_PREFIX}/lib/speech-dispatcher-modules")

install(TARGETS "sd_rhvoice"
	RUNTIME DESTINATION "${SPEECH_DISPATCHER_MODULES_DIR}"
	COMPONENT "sd_module"
)

set("CPACK_DEBIAN_SD_MODULE_PACKAGE_NAME" "speech-dispatcher-rhvoice" PARENT_SCOPE)
set(CPACK_DEBIAN_SD_MODULE_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, speech-dispatcher" PARENT_SCOPE)
set(CPACK_DEBIAN_SD_MODULE_PACKAGE_SUGGESTS "spd-say, python3-speechd" PARENT_SCOPE)
