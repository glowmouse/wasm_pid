#
# Test CMAKE file
# 
# To use. emcmake cmake ../wasm_pid from a build folder
# 
# make 
#
# should build a minimal index.html web page
#
# make run
#
# will launch a minimum web server 
#

cmake_minimum_required(VERSION 3.12)
project (index.html CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED ENV{EMSDK}) 
	message( FATAL_ERROR "emsdk environment wasn't found - missing $EMSDK Environment Variable")
endif()

set(EMSDK $ENV{EMSDK})

add_compile_options("-O2")
add_link_options("SHELL:-s WASM=1")
add_link_options("SHELL:-s USE_SDL=2")
add_link_options(--shell-file ${EMSDK}/upstream/emscripten/src/shell_minimal.html)

set (PIDSIM_SOURCES 
	"pid_sim.cpp"
	"sdlgui/button.cpp"
	"sdlgui/checkbox.cpp"
	"sdlgui/colorpicker.cpp"
	"sdlgui/colorwheel.cpp"
	"sdlgui/combobox.cpp"
	"sdlgui/common.cpp"
	"sdlgui/dropdownbox.cpp"
	"sdlgui/graph.cpp"
	"sdlgui/imagepanel.cpp"
	"sdlgui/imageview.cpp"
	"sdlgui/label.cpp"
	"sdlgui/layout.cpp"
	"sdlgui/loadimages.cpp"
	"sdlgui/messagedialog.cpp"
	"sdlgui/nanovg.c"
	"sdlgui/popupbutton.cpp"
	"sdlgui/popup.cpp"
	"sdlgui/progressbar.cpp"
	"sdlgui/resources.cpp"
	"sdlgui/screen.cpp"
	"sdlgui/slider.cpp"
	"sdlgui/stackedwidget.cpp"
	"sdlgui/switchbox.cpp"
	"sdlgui/tabheader.cpp"
	"sdlgui/tabwidget.cpp"
	"sdlgui/textbox.cpp"
	"sdlgui/theme.cpp"
	"sdlgui/vscrollpanel.cpp"
	"sdlgui/widget.cpp"
	"sdlgui/window.cpp"
	)


add_executable( index.html ${PIDSIM_SOURCES} ) 
set_target_properties( index.html PROPERTIES SUFFIX "")
include_directories(SYSTEM ${CMAKE_SOURCE_DIR})

add_custom_target(run python3 -m http.server)

