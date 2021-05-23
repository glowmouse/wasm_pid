cmake_minimum_required(VERSION 3.5)

project (index.html CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# Make sure the EMSDK web assembly compile is installed
#
if (NOT DEFINED ENV{EMSDK})
  message( FATAL_ERROR "emsdk environment wasn't found.  Install the emsdk web assembly compiler by following the instructions at https://emscripten.org/docs/getting_started/downloads.html.  Make source you source emsdk_env.sh before running cmake or compiling")
endif()
set(EMSDK $ENV{EMSDK})

#
# General compilation options.  Just optimize right now
#
add_compile_options(-O)

#
# NANOGUI specific defines
#
add_compile_options(-DNANOGUI_LINUX)
add_compile_options(-DNANOVG_GLES3_IMPLEMENTATION)
add_compile_options(-DGLFW_INCLUDE_ES3)
add_compile_options(-DGLFW_INCLUDE_GLEXT)

#
# Options to get emsdk to install addons that we need
#
add_compile_options("SHELL:-s USE_GLFW=3")
add_compile_options("SHELL:-s FULL_ES3=1")
add_compile_options("SHELL:-s USE_WEBGL2=1")
add_compile_options("SHELL:-s ASSERTIONS=1")

# Link in a minimal web page shell
#
add_link_options("-shell-file ${EMSDK}/upstream/emscripten/src/shell_minimal.html")

#
# Library link options
#
add_link_options("-lGL")
add_link_options("-lm")
add_link_options("-lGLEW")
add_link_options("SHELL:-s USE_GLFW=3")
add_link_options("SHELL:-s FULL_ES3=1")
add_link_options("SHELL:-s USE_WEBGL2=1")
add_link_options("SHELL:-s WASM=1")

set(CMAKE_C_COMPILER      "em++" )
set(CMAKE_CXX_COMPILER    "em++" )
set(CMAKE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

include_directories( 
  ${CMAKE_ROOT_SOURCE_DIR}/include 
  ${CMAKE_ROOT_SOURCE_DIR}/ext/eigen 
  ${CMAKE_ROOT_SOURCE_DIR}/ext/nanovg 
)

#
# The nanogui library
#
# Add the source files to the web page individually instead of creating a
# library for simplicity.
#
set ( NANOGUI_LIB_SOURCES
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/button.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/checkbox.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/colorpicker.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/colorwheel.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/combobox.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/common.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/glcanvas.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/glutil.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/graph.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/imagepanel.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/imageview.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/label.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/layout.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/messagedialog.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/nanogui_resources.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/popupbutton.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/popup.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/progressbar.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/screen.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/serializer.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/slider.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/stackedwidget.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/tabheader.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/tabwidget.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/textbox.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/theme.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/vscrollpanel.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/widget.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/nanogui_src/window.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/ext/nanovg.c
)

#
# These are the actual PID simulator sources.
#
set ( SOURCES
  ${CMAKE_ROOT_SOURCE_DIR}/pidsim/pidsim_backend.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/pidsim/pidsim_backend_physics_sim.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/pidsim/pidsim_backend_pid_controller.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/pidsim/pidsim_frontend.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/pidsim/pidsim_main.cpp
  ${CMAKE_ROOT_SOURCE_DIR}/pidsim/pidsim_model.cpp
)

#
# For the PID simulator, increase the warning level.
#
foreach(SRC_ IN LISTS SOURCES)
  set_source_files_properties(${SRC_} PROPERTIES COMPILE_OPTIONS "-Wall" )
  set_source_files_properties(${SRC_} PROPERTIES COMPILE_OPTIONS "-Wextra" )
endforeach()

#
# The web assembler build page target
#
add_executable(index.html ${SOURCES} ${NANOGUI_LIB_SOURCES})

#
# "make server" will start a web server for testing
#
add_custom_target(server COMMAND python3 -m http.server)

#
# "make webinstall" will copy the web page to the github docs page
#
add_custom_target(webinstall COMMAND cp index.html index.js index.wasm ${CMAKE_ROOT_SOURCE_DIR}/docs )

