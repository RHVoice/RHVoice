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

set("CPACK_DEBIAN_SERVER_PACKAGE_NAME" "rhvoice-socket-server")
set("CPACK_DEBIAN_CLIENT_PACKAGE_NAME" "rhvoice-socket-client")

if(BUILD_SOCKET_SERVER)
	add_executable("RHVoice-socket-client" "${CMAKE_CURRENT_SOURCE_DIR}/rhvoice-client.c")
	target_include_directories("RHVoice-socket-client" PRIVATE "${DBUS_INCLUDE_DIRS}")
	target_link_libraries("RHVoice-socket-client" PRIVATE "RHVoice_core" "${DBUS_LIBRARIES}")
	harden("RHVoice-socket-client")
	add_sanitizers("RHVoice-socket-client")
	target_compile_definitions("RHVoice-socket-client" PRIVATE "-DVERSION=\"${VERSION}\"")

	cpack_add_component(socket-client
		DISPLAY_NAME "RHVoice socket client"
		DESCRIPTION "Provides a client allowing to speak text using RHVoice socket server. Deprecated. Use speech-dispatcher."
		DISABLED
		GROUP "binaries"
		DEPENDS "core"
	)
	list(APPEND CPACK_COMPONENTS_ALL "socket-client")

	install(TARGETS "RHVoice-socket-client"
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
		COMPONENT "socket-client"
	)

	set(CPACK_DEBIAN_CLIENT_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, ${CPACK_DEBIAN_AUDIO_PACKAGE_NAME}")
	set(CPACK_DEBIAN_CLIENT_PACKAGE_SUGGESTS "${CPACK_DEBIAN_SERVER_PACKAGE_NAME}")
endif()


if(FALSE AND BUILD_SOCKET_SERVER)
	add_executable("RHVoice-socket-server" "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
	target_link_libraries("RHVoice-socket-server" "RHVoice_core")
	target_compile_definitions(RHVoice-socket-server PRIVATE "-DPACKAGE=\"RHVoice\"")
	harden("RHVoice-socket-server")
	add_sanitizers("RHVoice-socket-server")

	cpack_add_component(socket-server
		DISPLAY_NAME "RHVoice socket server"
		DESCRIPTION "Provides a socket server allowing other processes to speak text via IPC. Deprecated. Use speech-dispatcher."
		DISABLED
		GROUP "binaries"
		DEPENDS "core"
	)
	list(APPEND CPACK_COMPONENTS_ALL "socket-server")

	install(TARGETS "RHVoice-socket-server"
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
		COMPONENT "socket-server"
	)

	set(CPACK_DEBIAN_SERVER_PACKAGE_DEPENDS "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, ${CPACK_DEBIAN_AUDIO_PACKAGE_NAME}")
	set(CPACK_DEBIAN_SERVER_PACKAGE_SUGGESTS "${CPACK_DEBIAN_CLIENT_PACKAGE_NAME}")
endif()

pass_through_cpack_vars()
