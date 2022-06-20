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


#pkg_check_modules(LIBGLIMM REQUIRED glibmm-2.4)
pkg_check_modules(LIBGIOMM REQUIRED giomm-2.4)

add_executable("RHVoice-service" "${CMAKE_CURRENT_SOURCE_DIR}/service.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/common.cpp")
target_include_directories("RHVoice-service" PRIVATE "${LIBGIOMM_INCLUDE_DIRS}" "${HTS_LABELS_KIT_INCLUDES}")
target_link_libraries("RHVoice-service" "RHVoice_core" "${LIBGIOMM_LIBRARIES}")
harden("RHVoice-service")
add_sanitizers("RHVoice-service")

install(TARGETS "RHVoice-service"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT "service"
)
set(SERVICE_FILE_NAME "com.github.OlgaYakovleva.RHVoice.service")

set(serviceInputFile "${CMAKE_CURRENT_SOURCE_DIR}/${SERVICE_FILE_NAME}.in")
set(serviceOutputFile "${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_FILE_NAME}")

set(bindir "${CMAKE_INSTALL_FULL_BINDIR}")
set("servicedir" "${CMAKE_INSTALL_DATADIR}/dbus-1/services" CACHE PATH ".service file installation directory")
set("servicedir_full" "${CMAKE_INSTALL_PREFIX}/${servicedir}")
configure_file("${serviceInputFile}" "${serviceOutputFile}")
install(FILES "${serviceOutputFile}"
	DESTINATION "${servicedir}"
	COMPONENT "service"
)
set("CPACK_DEBIAN_SERVICE_PACKAGE_NAME" "rhvoice-systemd-service" PARENT_SCOPE)
