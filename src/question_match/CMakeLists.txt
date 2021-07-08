# -*- mode: Python; indent-tabs-mode: t; tab-width: 4 -*-
# Copyright (C) 2012, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com>

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


add_library(RHVoice_question_match SHARED "${SRCFILES}")
set_target_properties(RHVoice_question_match PROPERTIES VERSION "${RHVOICE_VERSION}" SOVERSION "${RHVOICE_VERSION_MAJOR}")
target_include_directories(RHVoice_question_match PUBLIC "${INCLUDE_DIR}" PRIVATE "${HTS_LABELS_KIT_INCLUDES}")

set(VERSION "${RHVOICE_VERSION}")

harden("RHVoice_question_match")

cpack_add_component(question_match
	DISPLAY_NAME "Question matcher"
	DESCRIPTION "Provides a library that contains common code for question matcher. Used by both core and hts engine"
	GROUP "libraries"
	DEPENDS "io"
)
list(APPEND CPACK_COMPONENTS_ALL "question_match")

install(TARGETS "RHVoice_question_match"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	COMPONENT "question_match"
)

set("CPACK_DEBIAN_QUESTION_MATCH_PACKAGE_NAME" "librhvoice-question_match")
pass_through_cpack_vars()
