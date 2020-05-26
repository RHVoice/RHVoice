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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set(SOURCES "audio.cpp;playback_stream.cpp;file_playback_stream_impl.cpp")

find_package(LibAO)
find_package(Portaudio)
find_package(LIBPULSESIMPLE)

option(WITH_LIBAO "Support libao" "${LIBAO_FOUND}")
option(WITH_PULSE "Support pulseaudio" "${LIBPULSESIMPLE_FOUND}")
option(WITH_PORTAUDIO "Support portaudio" "${PORTAUDIO2_FOUND}")

if(WITH_LIBAO)
	list(APPEND SOURCES "libao.cpp")
endif()
if(WITH_PULSE)
	list(APPEND SOURCES "pulse.cpp")
endif()
if(WITH_PORTAUDIO)
	list(APPEND SOURCES "portaudio.cpp")
endif()

add_library(RHVoice_audio "${SOURCES}")
harden("RHVoice_audio")
target_include_directories(RHVoice_audio PRIVATE "${INCLUDE_DIR}")

if(WITH_LIBAO)
	target_link_libraries(RHVoice_audio "${LIBAO_LIBRARIES}")
	target_include_directories(RHVoice_audio PRIVATE "${LIBAO_INCLUDE_DIRS}")
	target_link_directories(RHVoice_audio PRIVATE ${LIBAO_LIBRARY_DIRS})
endif()
if(WITH_PULSE)
	target_compile_definitions(RHVoice_audio PUBLIC "-DWITH_PULSE=1")
	target_link_libraries(RHVoice_audio "${LIBPULSESIMPLE_LIBRARIES}")
	target_include_directories(RHVoice_audio PRIVATE "${LIBPULSESIMPLE_INCLUDE_DIRS}")
	target_link_directories(RHVoice_audio PRIVATE ${LIBPULSESIMPLE_LIBRARY_DIRS})
endif()
if(WITH_PORTAUDIO)
	target_compile_definitions(RHVoice_audio PUBLIC "-DWITH_PORTAUDIO=1")
	target_link_libraries(RHVoice_audio "${PORTAUDIO_LIBRARIES}")
	target_include_directories(RHVoice_audio PRIVATE "${PORTAUDIO_INCLUDE_DIRS}")
	target_link_directories(RHVoice_audio PRIVATE ${PORTAUDIO_LIBRARY_DIRS})
endif()

install(TARGETS "RHVoice_audio"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "audio"
)

set("CPACK_DEBIAN_AUDIO_PACKAGE_NAME" "librhvoice-audio" PARENT_SCOPE)
set(CPACK_DEBIAN_AUDIO_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}" PARENT_SCOPE)
