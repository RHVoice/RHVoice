pkg_check_modules(DBUS dbus)
pkg_check_modules(LIBGIOMM giomm-2.4)
pkg_check_modules(SPEECHD speech-dispatcher)

set(BUILD_CLIENT "${DBUS_FOUND}" CACHE BOOL "Build standalone RHVoice-client application")
set(BUILD_UTILS ON CACHE BOOL "Build some useful utils")
set(BUILD_TESTS ON CACHE BOOL "Build test applications")
set(BUILD_SERVICE "${LIBGIOMM_FOUND}" CACHE BOOL "Build RHVoice server application")
set(BUILD_SPEECHDISPATCHER_MODULE "${SPEECHD_FOUND}" CACHE BOOL "Build SpeechDispatcher module")

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
configure_file("${INCLUDE_DIR}/core/config.h.in" "${INCLUDE_DIR}/core/config.h")

find_package(utf8cpp)
if (utf8cpp_FOUND)
    get_target_property(UTF8_INCLUDE_DIR utf8cpp INTERFACE_INCLUDE_DIRECTORIES)
else(utf8cpp_FOUND)
    set(UTF8_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/utf8")
endif(utf8cpp_FOUND)

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
add_subdirectory("hts_engine")

if (BUILD_CLIENT)
	add_subdirectory("bin")
endif(BUILD_CLIENT)

if (BUILD_UTILS)
	add_subdirectory("utils")
endif(BUILD_UTILS)

if (BUILD_TESTS)
	add_subdirectory("test")
endif(BUILD_TESTS)

IF (WIN32)
	#add_subdirectory("sapi")
	#add_subdirectory("nvda-addon")
	#add_subdirectory("wininst")
endif()

if(BUILD_SERVICE)
	add_subdirectory("service")
endif(BUILD_SERVICE)

if(BUILD_SPEECHDISPATCHER_MODULE)
	add_subdirectory("sd_module")
endif(BUILD_SPEECHDISPATCHER_MODULE)

pass_through_cpack_vars()
