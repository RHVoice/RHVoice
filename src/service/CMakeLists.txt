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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

add_executable("RHVoice-service" "${CMAKE_CURRENT_SOURCE_DIR}/service.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/common.cpp")
target_include_directories("RHVoice-service" PRIVATE "${glibmm_INCLUDE_DIRS}" "${HTS_LABELS_KIT_INCLUDES}")
target_link_libraries("RHVoice-service" "RHVoice_core" "${glibmm_LIBRARIES}")
harden("RHVoice-service")

install(TARGETS "RHVoice-service"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT "service"
)
set(SERVICE_FILE_NAME "com.github.OlgaYakovleva.RHVoice.service")

set(serviceInputFile "${CMAKE_CURRENT_SOURCE_DIR}/${SERVICE_FILE_NAME}.in")
set(serviceOutputFile "${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_FILE_NAME}")

set(bindir "${BINDIR}")
configure_file("${serviceInputFile}" "${serviceOutputFile}")
install(FILES "${serviceOutputFile}"
	DESTINATION "${servicedir}/systemd/system"
	COMPONENT "service"
)
set("CPACK_DEBIAN_SERVICE_PACKAGE_NAME" "rhvoice-systemd-service" PARENT_SCOPE)
