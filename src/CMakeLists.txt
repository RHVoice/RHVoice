pkg_check_modules(LIBGIOMM giomm-2.4)

CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/socket.h" HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE("sys/un.h" HAVE_SYS_UN_H)

if(HAVE_SYS_TYPES_H AND HAVE_SYS_STAT_H AND HAVE_SYS_SOCKET_H AND HAVE_SYS_UN_H)
	set(HAVE_HEADERS_FOR_UNIX_CLIENT_SERVER ON)
endif()

if(WIN32 OR HAVE_HEADERS_FOR_UNIX_CLIENT_SERVER)
	set(HAVE_HEADERS_FOR_CLIENT ON)
endif()

if(NOT WIN32)  # doesn't depend on speech-dispatcher headers, but depends RHVoice internal C++ wrapper for threading. Probably may be possible to make it buildable after it is eliminated in favour of std::thread
	set(BUILD_SPEECHDISPATCHER_MODULE ON)
endif()

set(BUILD_SOCKET_SERVER "${HAVE_HEADERS_FOR_UNIX_CLIENT_SERVER}" CACHE BOOL "Build standalone RHVoice-client application")
set(BUILD_SOCKET_CLIENT "${HAVE_HEADERS_FOR_CLIENT}" CACHE BOOL "Build standalone RHVoice-client application")
set(BUILD_UTILS ON CACHE BOOL "Build some useful utils")
set(BUILD_TESTS ON CACHE BOOL "Build test applications")
set(BUILD_SERVICE "${LIBGIOMM_FOUND}" CACHE BOOL "Build RHVoice server application")
set(BUILD_SPEECHDISPATCHER_MODULE "${BUILD_SPEECHDISPATCHER_MODULE}" CACHE BOOL "Build SpeechDispatcher module")

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
configure_file("${INCLUDE_DIR}/core/config.h.in" "${INCLUDE_DIR}/core/config.h")

set(UTF8_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/utf8")

cpack_add_component_group("libraries"
	DISPLAY_NAME "libraries"
	DESCRIPTION "Libraries that implement the needed functionality"
	EXPANDED
)

cpack_add_component_group("binaries"
	DISPLAY_NAME "Programs"
	DESCRIPTION "Programs to be called by users directly"
	EXPANDED
)

add_subdirectory("third-party")
set(HTS_LABELS_KIT_INCLUDES "${UTF8_INCLUDE_DIR}" "${RAPIDXML_INCLUDE_DIR}")

add_subdirectory("core")
add_subdirectory("audio")
add_subdirectory("lib")

if(BUILD_SOCKET_CLIENT OR BUILD_SOCKET_SERVER)
	add_subdirectory("bin")
endif()

if (BUILD_UTILS)
	add_subdirectory("utils")
endif(BUILD_UTILS)

if (BUILD_TESTS)
	add_subdirectory("test")
endif(BUILD_TESTS)

IF (WIN32)
	#add_subdirectory("sapi")
	#add_subdirectory("nvda-synthDriver")
	#add_subdirectory("wininst")
endif()

if(BUILD_SERVICE)
	add_subdirectory("service")
endif(BUILD_SERVICE)

if(BUILD_SPEECHDISPATCHER_MODULE)
	add_subdirectory("sd_module")
endif(BUILD_SPEECHDISPATCHER_MODULE)

pass_through_cpack_vars()
