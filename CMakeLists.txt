cmake_minimum_required (VERSION 3.12.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project (RHVoice)
add_definitions(-DENABLE_MAGE=1)
add_definitions(-DRHVOICE=1)
add_definitions(-DDATA_PATH="~/.local/share/")
add_definitions(-DCONFIG_PATH="~/.local/etc/")
add_definitions(-DVERSION="0.7.1")
find_package(LibAO REQUIRED)
include_directories("src/include" "src/audio" "src/core"
"src/third-party/rapidxml" "src/third-party/tclap"
"src/third-party/utf8" "src/third-party/sonic"
"src/third-party/hts_engine" "src/third-party/mage")
file(GLOB RHVOICE_SRC    "src/audio/*.cpp" 
"src/core/*.cpp" "src/core/*.c" "src/test/*.cpp"
"src/third-party/hts_engine/*.c" "src/third-party/mage/*.cpp"
"src/third-party/sonic/*.c")
list(FILTER RHVOICE_SRC EXCLUDE REGEX ".*unidata.cpp$")
list(FILTER RHVOICE_SRC EXCLUDE REGEX ".*userdict_parser.c")
add_library(RHVoice SHARED ${RHVOICE_SRC})
target_compile_options(RHVoice PRIVATE "-Wno-deprecated")
target_link_libraries(RHVoice ${LIBAO_LIBRARIES})

