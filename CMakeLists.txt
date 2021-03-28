#
# Test CMAKE file
# 
# To use. emcmake cmake ../wasm_game_of_life from a build folder
# 
# make 
#
# should build a minimal index.html web page
#
# make run
#
# will launch a minimum web server 
#

cmake_minimum_required(VERSION 3.1)
project (index.html CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED ENV{EMSDK}) 
	message( FATAL_ERROR "emsdk environment wasn't found - missing $EMSDK Environment Variable")
endif()

set(EMSDK $ENV{EMSDK})

add_compile_options("-O2")
add_link_options("-s WASM=1")
add_link_options("--shell-file ${EMSDK}/upstream/emscripten/src/shell_minimal.html")

set (PIDSIM_SOURCES "pid_sim.cpp") 

add_executable( index.html ${PIDSIM_SOURCES} ) 
set_target_properties( index.html PROPERTIES SUFFIX "")

add_custom_target(run python3 -m http.server)

