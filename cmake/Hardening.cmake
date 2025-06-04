#This is free and unencumbered software released into the public domain.
#Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.
#In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright interest in the software to the public domain. We make this dedication for the benefit of the public at large and to the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity of all present and future rights to this software under copyright law.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#For more information, please refer to <https://unlicense.org/>

include(CheckCXXCompilerFlag)
include(CheckIPOSupported)

set(HardeningClangWorkaroundLinkerPathFileName "HardeningClangWorkaroundLinker_qDhJsKjQn.txt")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${HardeningClangWorkaroundLinkerPathFileName}" "${CMAKE_LINKER}")
endif()

option(HARDENING_PIC "Enable position-independent code and ASLR." ON)
option(HARDENING_SSE2 "Enable hardening flags requiring at least SSE2 support for target" OFF)

check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
	find_program(
		POWERSHELL_PATH
		"powershell"
	)
	if(NOT POWERSHELL_PATH STREQUAL "POWERSHELL_PATH-NOTFOUND")
		set(CLANG_WORKAROUND_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/clangLinkerWorkaround.ps1")
	else()
		set(CLANG_WORKAROUND_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/clangLinkerWorkaround.js") # has issues with stdout and stderr, that can be worked around using third-party COM components. Kept here only for legacy systems.
	endif()
else()
	set(CLANG_WORKAROUND_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/clangLinkerWorkaround.sh")
endif()

function(determineSupportedHardeningFlags property)
	set(FLAGS_HARDENING "")
	foreach(flag ${ARGN})
		unset(var_name)
		string(REPLACE "\\" "_backslash_" var_name "${flag}")
		string(REPLACE "/" "_slash_" var_name "${var_name}")
		string(REPLACE "=" "_eq_" var_name "${var_name}")
		string(REPLACE "," "_comma_" var_name "${var_name}")
		string(REPLACE "\"" "_quot_" var_name "${var_name}")
		set(var_name "SUPPORTS_HARDENING_${var_name}")
		#message(STATUS "var_name ${var_name} flag ${flag}")
		check_cxx_compiler_flag("${flag}" "${var_name}")#since linker flags and other flags are in the form of compiler flags
		if(${${var_name}})
			list(APPEND FLAGS_HARDENING "${flag}")
		endif()
	endforeach(flag)
	string(REPLACE ";" " " FLAGS_HARDENING "${FLAGS_HARDENING}")
	#message(STATUS "FLAGS_HARDENING ${FLAGS_HARDENING}")
	set(HARDENING_${property} "${FLAGS_HARDENING}" PARENT_SCOPE)
endfunction(determineSupportedHardeningFlags)

function(processFlagsList target property cache)
	get_target_property(FLAGS_UNHARDENED ${target} ${property})
	if(FLAGS_UNHARDENED MATCHES "FLAGS_UNHARDENED-NOTFOUND")
		set(FLAGS_UNHARDENED "")
	endif()
	#message(STATUS "processFlagsList ${target} ${property} ${FLAGS_UNHARDENED}")
	#message(STATUS "HARDENING_${property} ${HARDENING_${property}}")
	
	if(cache)
		if(DEFINED CACHE{HARDENING_${property}})
			#message(STATUS "Using cached HARDENING_${property} ${HARDENING_${property}}")
		else()
			determineSupportedHardeningFlags(${property} ${ARGN})
			set(HARDENING_${property} "${HARDENING_${property}}" CACHE STRING "Hardening flags")
		endif()
	else()
		determineSupportedHardeningFlags(${property} ${ARGN})
	endif()
	
	set(FLAGS_HARDENED ${FLAGS_UNHARDENED})
	list(APPEND FLAGS_HARDENED ${HARDENING_${property}})
	string(REPLACE ";" " " FLAGS_HARDENED "${FLAGS_HARDENED}")
	#message(STATUS "${target} PROPERTIES ${property} ${FLAGS_HARDENED}")
	set_target_properties(${target} PROPERTIES ${property} "${FLAGS_HARDENED}")
endfunction(processFlagsList)

function(setupPIC target)
	set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON) # bad, doesn't work
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		get_target_property(type ${target} TYPE)
		if(type STREQUAL "EXECUTABLE")
			list(APPEND HARDENING_PIC_COMPILE_FLAGS
				"-fPIE"
			)
		else()
			list(APPEND HARDENING_PIC_COMPILE_FLAGS
				"-fPIC"
			)
		endif()
		if(type STREQUAL "EXECUTABLE")
			# https://mropert.github.io/2018/02/02/pic_pie_sanitizers/
			list(APPEND HARDENING_PIC_LINKER_FLAGS
				"-Wl,-pie"
			)
			if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
				message(STATUS "Working around Clang bug https://bugs.llvm.org/show_bug.cgi?id=44594 ...")
				if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11)
					list(APPEND HARDENING_PIC_LINKER_FLAGS
						"--ld-path=\"${CLANG_WORKAROUND_SCRIPT}\""
					)
				else()
					list(APPEND HARDENING_PIC_LINKER_FLAGS
						"-fuse-ld=\"${CLANG_WORKAROUND_SCRIPT}\""
					)
				endif ()
			endif()
			if(CMAKE_SYSTEM_NAME MATCHES "Windows")
				list(APPEND HARDENING_PIC_LINKER_FLAGS
					"-Wl,--dynamicbase"
				)
				if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				#	list(APPEND HARDENING_LINKER_FLAGS "-Wl,--image-base,0x140000000") # doesn't work for this project
				endif()
			endif()
		endif()
	elseif(MSVC)
		list(APPEND HARDENING_PIC_COMPILE_FLAGS
			"/dynamicbase" "/HIGHENTROPYVA"
		)
	else()
		message(ERROR "The compiler is not supported")
	endif()
	processFlagsList(${target} COMPILE_FLAGS OFF ${HARDENING_PIC_COMPILE_FLAGS})
	processFlagsList(${target} LINK_FLAGS OFF ${HARDENING_PIC_LINKER_FLAGS})
endfunction(setupPIC)

function(harden target)
	if(HARDENING_PIC)
		setupPIC("${target}")
	endif()
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		list(APPEND HARDENING_COMPILER_FLAGS
			"-Wall" "-Wextra" "-Wconversion" "-Wformat" "-Wformat-security" "-Werror=format-security" "-Wpointer-to-int-cast" "-Wuninitialized-const-reference"
			"-fno-strict-aliasing" "-fno-common"
			#"-fstack-check"
			#"-fcf-protection=full" # conflicts to "-mindirect-branch"
			"-fcf-runtime-abi=full"

			"-ffp-exception-behavior=strict"
			"-fstack-clash-protection"
			"-mcet"
			"-fsanitize=cfi"
			"-fsanitize=cfi-cast-strict"
			"-fsanitize=cfi-derived-cast"
			"-fsanitize=cfi-unrelated-cast"
			"-fsanitize=cfi-nvcall"
			"-fsanitize=cfi-vcall"
			"-fsanitize=cfi-icall"
			"-fsanitize=cfi-mfcall"
			
			# CLang-ish flags
			"-ftrivial-auto-var-init=zero"
			
			#"-fsanitize=safe-stack;compiler-rt" # https://clang.llvm.org/docs/SafeStack.html
			"-fsanitize=address" # https://clang.llvm.org/docs/AddressSanitizer.html
			
			# TODO implement compiler flag dependence on libs linking
			#"-fsanitize=undefined;ubsan" # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html , gcc also has it
			
			#"-fsanitize=thread" # https://clang.llvm.org/docs/ThreadSanitizer.html , 15x slowdown and 10x memory overhead
			#"-fsanitize=memory" # https://clang.llvm.org/docs/MemorySanitizer.html 3x slowdown
			#"-fsanitize=dataflow" # https://clang.llvm.org/docs/DataFlowSanitizer.html, taint analysis, requires explicit annotation of code
			
			#"-fvtable-verify=std;vtv"
			
			# this conflicts with gcc which now has -fcf-protection=full hardcoded
			"-fcf-protection=none -mindirect-branch"
			"-fcf-protection=none -mindirect-branch=thunk-extern"
			"-fcf-protection=none -mindirect-branch=thunk-inline"
			"-fcf-protection=none -mindirect-return"
			"-fcf-protection=none -mindirect-branch-register"
			"-fcf-protection=none -mindirect-branch-loop"
			
			"-x86-speculative-load-hardening"
			"-mno-indirect-branch-register"
		)

		if(HARDENING_SSE2)
			list(APPEND HARDENING_COMPILER_FLAGS
				"-mlfence-after-load=yes"
				"-mlfence-before-indirect-branch=all"
				"-mlfence-before-ret=not"
			)
		endif(HARDENING_SSE2)

		# some flags are bugged in GCC
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		else()
			list(APPEND HARDENING_COMPILER_FLAGS
				"-ftrapv" # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=35412
			)
		endif()

		# they are mutually exclusive, -mlvi-hardening is supported by CLang 11+, retpoline is supported by both
		# Do -mlvi-hardening and -mlvi-cfi mitigate against Spectre too?
		if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
			list(APPEND HARDENING_COMPILER_FLAGS
				"-mlvi-cfi"
				"-mlvi-hardening"
			)
		else()
			list(APPEND HARDENING_COMPILER_FLAGS
				"-mretpoline"
				"-mspeculative-load-hardening" # https://llvm.org/docs/SpeculativeLoadHardening.html
			)
		endif()
		
		# GCC 9 has removed these flags
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10))
			message(STATUS "GCC 9 removes some hardening flags but doesn't fail if they are present, instead shows deprecation message. In order to not to put garbage into warnings we don't insert them. See the code of Harden.cmake for the details.")
		else()
			list(APPEND HARDENING_COMPILER_FLAGS
				"-mmitigate-rop" 
				"-fcheck-pointer-bounds"
				"-fchkp-treat-zero-size-reloc-as-infinite"
				"-fchkp-first-field-has-own-bounds"
				"-fchkp-narrow-bounds"
				"-fchkp-narrow-to-innermost-array"
				"-fchkp-optimize"
				"-fchkp-use-fast-string-functions"
				"-fchkp-use-nochk-string-functions"
				"-fchkp-use-static-const-bounds"
			)
		endif()
		
		list(APPEND HARDENING_LINKER_FLAGS
			"-Wl,-O1"
			"-Wl,--sort-common"
			"-Wl,--as-needed"
		)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON PARENT_SCOPE)
		if(CMAKE_SYSTEM_NAME MATCHES "Windows")
			list(APPEND HARDENING_LINKER_FLAGS
				"-Wl,--export-all-symbols"
				"-Wl,--nxcompat"
			)
		elseif(CMAKE_SYSTEM_NAME MATCHES "Linux") # other using ELF too?
			list(APPEND HARDENING_COMPILER_FLAGS
				# on MinGW hello world works, but more complex things just exit without any output or crash in the middle of execution
				"-fstack-protector"
				"-fstack-protector-strong"
			)
			
			list(APPEND HARDENING_LINKER_FLAGS
				# not present in MinGW
				"-Wl,-z,relro"
				"-Wl,-z,now"
				"-Wl,-z,ibtplt"
				"-Wl,-z,ibt"
				"-Wl,-z,shstk"
				"-Wl,-z,notext"  # may be required for PIC
			)
		endif()
		list(APPEND HARDENING_MACRODEFS
			"-D_FORTIFY_SOURCE=2"
			"-D_GLIBCXX_ASSERTIONS"
		)
	elseif(MSVC)
		set(HARDENING_COMPILER_FLAGS "/sdl" "/GS" "/SafeSEH" "/guard:cf" "/HIGHENTROPYVA")
		set(HARDENING_LINKER_FLAGS "/guard:cf")
	else()
		message(ERROR "The compiler is not supported")
	endif()

	processFlagsList(${target} COMPILE_FLAGS ON ${HARDENING_COMPILER_FLAGS})
	processFlagsList(${target} LINK_FLAGS ON ${HARDENING_LINKER_FLAGS})
	
	#list(JOIN HARDENING_MACRODEFS " " HARDENING_MACRODEFS) # unneeded, list is needed, not string
	set(HARDENING_MACRODEFS "${HARDENING_MACRODEFS}" CACHE STRING "Hardening flags CMake list (not string!)")
	target_compile_definitions(${target} PRIVATE $CACHE{HARDENING_MACRODEFS})
endfunction(harden)
