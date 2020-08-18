set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
configure_file("${INCLUDE_DIR}/core/config.h.in" "${INCLUDE_DIR}/core/config.h")

add_subdirectory("third-party")
set(HTS_LABELS_KIT_INCLUDES "${UTF8_INCLUDE_DIR}" "${RAPIDXML_INCLUDE_DIR}")

add_subdirectory("core")
add_subdirectory("audio")
add_subdirectory("lib")
add_subdirectory("bin")
add_subdirectory("utils")
add_subdirectory("test")

IF (WIN32)
	#add_subdirectory("sapi")
	#add_subdirectory("nvda-synthDriver")
	#add_subdirectory("wininst")
else()
	add_subdirectory("service")
	add_subdirectory("sd_module")
endif()

pass_through_cpack_vars()