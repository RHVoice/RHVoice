# #This is free and unencumbered software released into the public domain.
#Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.
#In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright interest in the software to the public domain. We make this dedication for the benefit of the public at large and to the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity of all present and future rights to this software under copyright law.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#For more information, please refer to <https://unlicense.org/>

FIND_PACKAGE(Git)
function(getVersionFromGit variablesPrefix defaultVersion)
	if(${variablesPrefix}_VERSION_FROM_GIT)
		set(VERSION_FROM_GIT "${${variablesPrefix}_VERSION_FROM_GIT}")
		set(VERSION_TIME "${${variablesPrefix}_VERSION_TIME}")
		set(VERSION_EXPORT "${${variablesPrefix}_VERSION_EXPORT}")
		message(STATUS "Using cached versions: ${VERSION_FROM_GIT}, ${VERSION_TIME}, ${VERSION_EXPORT}")
	else()
		if(GIT_FOUND)
			execute_process(
				COMMAND ${GIT_EXECUTABLE} describe --dirty --long --tags
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				OUTPUT_VARIABLE VERSION_FROM_GIT
				RESULT_VARIABLE GIT_RESULT
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			if(NOT GIT_RESULT EQUAL 0)
				message(WARNING "git returned error ${VERSION_FROM_GIT}")
				set(VERSION_FROM_GIT "unknown")
			endif()

			execute_process(
				COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%ct
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				OUTPUT_VARIABLE VERSION_TIME
				RESULT_VARIABLE GIT_RESULT
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			if(NOT GIT_RESULT EQUAL 0)
				set(VERSION_TIME "0000000000" PARENT_SCOPE)
			endif()

			execute_process(
				COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%D
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				OUTPUT_VARIABLE VERSION_EXPORT
				RESULT_VARIABLE GIT_RESULT
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			if(NOT GIT_RESULT EQUAL 0)
				set(VERSION_EXPORT "HEAD -> master")
			endif()
		else(GIT_FOUND)
			set(VERSION_FROM_GIT ${defaultVersion} PARENT_SCOPE)
		endif(GIT_FOUND)
		set(${variablesPrefix}_VERSION_FROM_GIT "${VERSION_FROM_GIT}" CACHE STRING "Version from git")
		set(${variablesPrefix}_VERSION_TIME "${VERSION_TIME}" CACHE STRING "Commit time from git")
		set(${variablesPrefix}_VERSION_EXPORT "${VERSION_EXPORT}" CACHE STRING "Commit time from git")
	endif()

	if(VERSION_EXPORT)
	else()
		set(VERSION_EXPORT "HEAD -> master")
	endif()
	if(VERSION_TIME)
	else()
		set(VERSION_TIME "0000000000")
	endif()

	string(REGEX MATCH "^v?[0-9]+(\\.[0-9]+)+" VERSION ${VERSION_FROM_GIT})
	set(${variablesPrefix}_VERSION "${VERSION}" PARENT_SCOPE)
	message(STATUS "${variablesPrefix} version: ${VERSION}")

	string(REGEX MATCHALL "[0-9]+" PARSED_VER ${VERSION})

	list(LENGTH PARSED_VER PARSED_VER_LEN)
	if(PARSED_VER_LEN GREATER 0)
		list(GET PARSED_VER 0 VERSION_MAJOR)
	else()
		set(VERSION_MAJOR 0)
	endif()
	set(${variablesPrefix}_VERSION_MAJOR "${VERSION_MAJOR}" PARENT_SCOPE)

	if(PARSED_VER_LEN GREATER 1)
		list(GET PARSED_VER 1 VERSION_MINOR)
	else()
		set(VERSION_MINOR 0)
	endif()
	set(${variablesPrefix}_VERSION_MINOR "${VERSION_MINOR}" PARENT_SCOPE)

	if(PARSED_VER_LEN GREATER 2)
		list(GET PARSED_VER 2 VERSION_PATCH)
	else()
		set(VERSION_PATCH 0)
	endif()
	set(${variablesPrefix}_VERSION_PATCH "${VERSION_PATCH}" PARENT_SCOPE)

	if(PARSED_VER_LEN GREATER 3)
		list(GET PARSED_VER 3 VERSION_TWEAK)
	else()
		set(VERSION_TWEAK 0)
	endif()
	set(${variablesPrefix}_VERSION_TWEAK "${VERSION_TWEAK}" PARENT_SCOPE)

	set(VERSION_BIN "${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH}")
	set(${variablesPrefix}_VERSION_BIN "${VERSION_BIN}" PARENT_SCOPE)
	message(STATUS "${variablesPrefix} bin version: ${VERSION_BIN}")

	set(VERSION_FULL "${VERSION_EXPORT} ${${variablesPrefix}_VERSION_FROM_GIT}")
	set(${variablesPrefix}_VERSION_FULL "${VERSION_FULL}" PARENT_SCOPE)
	message(STATUS "version full: ${VERSION_FULL}")
endfunction()
