# -*- mode: Python; indent-tabs-mode: t; tab-width: 4 -*-
# Copyright (C) 2012, 2014, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

add_executable("RHVoice-test" "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp")
target_link_libraries("RHVoice-test" "RHVoice" "RHVoice_audio")
target_include_directories("RHVoice-test" PRIVATE "${TCLAP_INCLUDE_DIR}" "${HTS_LABELS_KIT_INCLUDES}")
harden("RHVoice-test")
add_sanitizers("RHVoice-test")

if(WITH_CLI11)
	target_compile_definitions(RHVoice-test PRIVATE WITH_CLI11)
endif(WITH_CLI11)

cpack_add_component(test
	DISPLAY_NAME "Standalone CLI application"
	DESCRIPTION "Provides a CLI application that allows you to synthesize speech using RHVoice"
	GROUP "binaries"
	DEPENDS "audio"
)
list(APPEND CPACK_COMPONENTS_ALL "test")

install(TARGETS "RHVoice-test"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT "test"
)
set("CPACK_DEBIAN_TEST_PACKAGE_NAME" "rhvoice-test")
set(CPACK_DEBIAN_TEST_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, ${CPACK_DEBIAN_AUDIO_PACKAGE_NAME}")


pass_through_cpack_vars()
