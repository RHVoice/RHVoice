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

file(GLOB_RECURSE SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/HTS_*.c")

# core/str_hts_engine_impl.cpp contains an impl of the methods defined in HTS_audio.c
# RHVoice build will fail if hts_engine is built and linked as a shared library on Windows.
# TBH some refactoring is needed in order to provide these functions properly, but for now it is easier to compile hts_engine as an OBJECT to be linked to the RHVoice_core. It may be suboptimal though, and surely is dirty.
add_library(libhts_engine OBJECT "${SRCFILES}")
#target_compile_definitions(libhts_engine PRIVATE "-DAUDIO_PLAY_NONE") # Don't activate this before the proper refactoring of HTS_engine is done!!!

set(LIBHTS_VERSION_MAJOR 1)
set(LIBHTS_VERSION_MINOR 06)
set(LIBHTS_VERSION_PATCH 0)
set(LIBHTS_VERSION "${LIBHTS_VERSION_MAJOR}.${LIBHTS_VERSION_MINOR}-RHVoice")
set_target_properties(libhts_engine PROPERTIES VERSION "${LIBHTS_VERSION}" SOVERSION "${LIBHTS_VERSION_MAJOR}")

set(CPACK_COMPONENT_LIBHTS_ENGINE_VERSION_MAJOR "${LIBHTS_VERSION_MAJOR}")
set(CPACK_COMPONENT_LIBHTS_ENGINE_VERSION_MINOR "${LIBHTS_VERSION_MINOR}")
set(CPACK_COMPONENT_LIBHTS_ENGINE_VERSION_PATCH "${LIBHTS_VERSION_PATCH}")
set(CPACK_COMPONENT_LIBHTS_ENGINE_VERSION "${LIBHTS_VERSION}")

target_include_directories(libhts_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${INCLUDE_DIR}")
if(NOT MSVC)
	target_link_libraries(libhts_engine PUBLIC m)
endif()
harden(libhts_engine)
add_sanitizers(libhts_engine)
set_target_properties("libhts_engine" PROPERTIES PREFIX "")


cpack_add_component(libhts_engine
	DISPLAY_NAME "HTS engine"
	DESCRIPTION "hts_engine is software to synthesize speech waveform from HMMs trained by the HMM-based speech synthesis system (HTS). This software is released under the Modified BSD license. This component contains a modified version of HTS engine, for the modifications see the RHVoice GitHub repo."
	GROUP "third_party_libs"
)
#list(APPEND CPACK_COMPONENTS_ALL "libhts_engine")

install(TARGETS "libhts_engine"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "libhts_engine"
)
set("CPACK_DEBIAN_LIBHTS_ENGINE_PACKAGE_NAME" "libhtsengine")
pass_through_cpack_vars()
