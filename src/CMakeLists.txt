set(BUILD_CLIENT ON CACHE BOOL "Build standalone RHVoice-client application")
set(BUILD_UTILS ON CACHE BOOL "Build some useful utils")
set(BUILD_TESTS ON CACHE BOOL "Build test applications")
set(BUILD_SERVICE ON CACHE BOOL "Build RHVoice server application")
set(BUILD_SPEECHDISPATCHER_MODULE ON CACHE BOOL "Build SpeechDispatcher module")

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

add_subdirectory("io")
add_subdirectory("filter_manager")
add_subdirectory("question_match")

add_subdirectory("third-party")
set(HTS_LABELS_KIT_INCLUDES "${UTF8_INCLUDE_DIR}" "${RAPIDXML_INCLUDE_DIR}")

add_subdirectory("core")
add_subdirectory("audio")
add_subdirectory("lib")

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
	#add_subdirectory("nvda-synthDriver")
	#add_subdirectory("wininst")
else()
	if(BUILD_SERVICE)
		add_subdirectory("service")
	endif(BUILD_SERVICE)

	if(BUILD_SPEECHDISPATCHER_MODULE)
		add_subdirectory("sd_module")
	endif(BUILD_SPEECHDISPATCHER_MODULE)
endif()

pass_through_cpack_vars()
