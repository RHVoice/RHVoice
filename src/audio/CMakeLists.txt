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

set(SOURCES "audio.cpp;file_playback_stream_impl.cpp;playback_stream.cpp")

set(audio_libs "libao;pulse;portaudio")

pkg_check_modules(LIBAO "ao")
pkg_check_modules(LIBPORTAUDIO "portaudio-2.0")
pkg_check_modules(LIBPULSESIMPLE "libpulse-simple")
option(WITH_LIBAO "Support libao" "${LIBAO_FOUND}")
option(WITH_PULSE "Support pulseaudio" "${LIBPULSESIMPLE_FOUND}")
option(WITH_PORTAUDIO "Support portaudio" "${LIBPORTAUDIO_FOUND}")

if(WITH_LIBAO)
	list(APPEND SOURCES "libao.cpp")
endif()
if(WITH_PULSE)
	list(APPEND SOURCES "pulse.cpp")
endif()
if(WITH_PORTAUDIO)
	list(APPEND SOURCES "portaudio.cpp")
endif()

add_library(RHVoice_audio SHARED "${SOURCES}")
set_target_properties(RHVoice_audio PROPERTIES VERSION "${RHVOICE_VERSION}" SOVERSION "${RHVOICE_VERSION_MAJOR}")
harden("RHVoice_audio")
add_sanitizers("RHVoice_audio")
target_include_directories(RHVoice_audio PRIVATE "${INCLUDE_DIR}")

if(WITH_LIBAO)
	target_link_libraries(RHVoice_audio "${LIBAO_LIBRARIES}")
	message(STATUS "${LIBAO_INCLUDE_DIRS}")
	target_include_directories(RHVoice_audio PRIVATE "${LIBAO_INCLUDE_DIRS}")
endif()
if(WITH_PULSE)
	target_compile_definitions(RHVoice_audio PUBLIC WITH_PULSE)
	target_link_libraries(RHVoice_audio "${LIBPULSESIMPLE_LIBRARIES}")
	target_include_directories(RHVoice_audio PRIVATE "${LIBPULSESIMPLE_INCLUDE_DIRS}")
endif()
if(WITH_PORTAUDIO)
	target_compile_definitions(RHVoice_audio PUBLIC WITH_PORTAUDIO)
	target_link_libraries(RHVoice_audio "${LIBPORTAUDIO_LIBRARIES}")
	target_include_directories(RHVoice_audio PRIVATE "${LIBPORTAUDIO_INCLUDE_DIRS}")
endif()

cpack_add_component(audio
	DISPLAY_NAME "Text-to-speech-sound library"
	DESCRIPTION "Provides a library that abstracts the way speech is output. It is also provides a high-level facade for speech synthesis"
	GROUP "libraries"
	DEPENDS "libhts_engine"
)
list(APPEND CPACK_COMPONENTS_ALL "audio")

install(TARGETS "RHVoice_audio"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "audio"
)

set("CPACK_DEBIAN_AUDIO_PACKAGE_NAME" "librhvoice-audio")
set(CPACK_DEBIAN_AUDIO_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}")
pass_through_cpack_vars()
