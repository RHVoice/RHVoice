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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

pkg_check_modules(SPEECHD REQUIRED speech-dispatcher)

file(GLOB_RECURSE SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(sd_rhvoice "${SRCFILES}")
target_include_directories(sd_rhvoice PRIVATE "${INCLUDE_DIR}" "${HTS_LABELS_KIT_INCLUDES}" "${SPEECHD_INCLUDE_DIRS}")
target_link_libraries(sd_rhvoice "RHVoice_core" "RHVoice_audio" "${SPEECHD_LIBRARIES}" pthread)
harden(sd_rhvoice)
add_sanitizers(sd_rhvoice)

pkg_get_variable(modulebindir ${SPEECHD_MODULE_NAME} modulebindir)
if (NOT modulebindir)
	pkg_get_variable(libdir ${SPEECHD_MODULE_NAME} libdir)
	set(modulebindir "${libdir}/speech-dispatcher-modules")
endif()
set(SPEECH_DISPATCHER_MODULES_DIR "${modulebindir}" CACHE PATH "Speech Dispatcher's module directory path")


cpack_add_component(sd_module
	DISPLAY_NAME "RHVoice speech-dispatcher middleware module"
	DESCRIPTION "Provides a way to use RHVoice using speech-dispacher middleware. It is the recommended way to use RHVoice on *nix hosts."
	GROUP "binaries"
	DEPENDS "core" "audio"
)
list(APPEND CPACK_COMPONENTS_ALL "sd_module")

install(TARGETS "sd_rhvoice"
	RUNTIME DESTINATION "${SPEECH_DISPATCHER_MODULES_DIR}"
	COMPONENT "sd_module"
)

set("CPACK_DEBIAN_SD_MODULE_PACKAGE_NAME" "speech-dispatcher-rhvoice")
set(CPACK_DEBIAN_SD_MODULE_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, speech-dispatcher")
set(CPACK_DEBIAN_SD_MODULE_PACKAGE_SUGGESTS "spd-say, python3-speechd")
pass_through_cpack_vars()
