# -*- mode: Python; indent-tabs-mode: t -*-
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


add_executable("RHVoice-transcribe-sentences" "${CMAKE_CURRENT_SOURCE_DIR}/transcribe-sentences.cpp")
target_include_directories("RHVoice-transcribe-sentences" PRIVATE "${TCLAP_INCLUDE_DIR}" "${HTS_LABELS_KIT_INCLUDES}" )
target_link_libraries("RHVoice-transcribe-sentences" "RHVoice_core")
harden("RHVoice-transcribe-sentences")
add_sanitizers("RHVoice-transcribe-sentences")

if(NOT (MINGW AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11))
	add_executable("RHVoice-make-hts-labels" "${CMAKE_CURRENT_SOURCE_DIR}/make-hts-labels.cpp")
	add_sanitizers("RHVoice-make-hts-labels")
	target_include_directories("RHVoice-make-hts-labels" PRIVATE "${TCLAP_INCLUDE_DIR}" "${HTS_LABELS_KIT_INCLUDES}" )
	target_link_libraries("RHVoice-make-hts-labels" "RHVoice_core" "libhts_engine")
	harden("RHVoice-make-hts-labels")
	if(WITH_CLI11)
		target_compile_definitions(RHVoice-make-hts-labels PRIVATE WITH_CLI11)
	endif(WITH_CLI11)
	install(TARGETS "RHVoice-transcribe-sentences" "RHVoice-make-hts-labels"
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
		COMPONENT "utils"
	)
else()
	message(WARNING "RHVoice-make-hts-labels building fails on MinGW + Clang 11 with `undefined symbol: __Unwind_Resume` IDK why (even if exceptions model is set right). So we don't compile it for now. Please fix this issue, if you can.")
endif()

cpack_add_component(utils
	DISPLAY_NAME "Utilities for voice creation"
	DESCRIPTION "Contain some utilities needed for the ones creating voices."
	DISABLED
	GROUP "binaries"
	DEPENDS "core" 
)
list(APPEND CPACK_COMPONENTS_ALL "utils")

set("CPACK_DEBIAN_UTILS_PACKAGE_NAME" "rhvoice-tools")
set(CPACK_DEBIAN_UTILS_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, ${CPACK_DEBIAN_AUDIO_PACKAGE_NAME}")

pass_through_cpack_vars()
