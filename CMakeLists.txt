cmake_minimum_required(VERSION 3.5)

project (index.html CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NANOFLAGS "-DNANOGUI_LINUX -DNANOVG_GLES3_IMPLEMENTATION")
set(GLFLAGS   "-DGLFW_INCLUDE_ES3 -DGLFW_INCLUDE_GLEXT -s USE_GLFW=3 -s FULL_ES3=1 -s USE_WEBGL2=1 -s ASSERTIONS=1 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/pidsim/shell_minimal.html" )
set(EMFLAGS   "--std=c++17 -O -s WASM=1" )
set(LDFLAGS   "-lGL -lm -lGLEW" )

set(ALL_COMPILE_FLAGS "${EMFLAGS} ${NANOFLAGS} ${GLFLAGS}")
message(STATUS "${ALL_COMPILE_FLAGS}")

set(CMAKE_C_COMPILER      "em++" )
set(CMAKE_C_FLAGS       "${ALL_COMPILE_FLAGS}" )

set(CMAKE_CXX_COMPILER    "em++" )
set(CMAKE_CXX_FLAGS     "${ALL_COMPILE_FLAGS}" )

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}/include 
  ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen 
  ${CMAKE_CURRENT_SOURCE_DIR}/ext/nanovg 
)

# Just compile the library right in there.
set ( NANOGUI_LIB_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/button.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/checkbox.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/colorpicker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/colorwheel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/combobox.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/common.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/glcanvas.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/glutil.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/graph.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/imagepanel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/imageview.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/label.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/layout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/messagedialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/nanogui_resources.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/popupbutton.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/popup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/progressbar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/screen.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/serializer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/slider.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/stackedwidget.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/tabheader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/tabwidget.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/textbox.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/theme.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/vscrollpanel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/widget.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/nanogui_src/window.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ext/nanovg.c
)

set ( SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/pidsim/pidsim_backend.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pidsim/pidsim_backend_physics_sim.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pidsim/pidsim_backend_pid_controller.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pidsim/pidsim_frontend.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pidsim/pidsim_main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/pidsim/pidsim_model.cpp
)

foreach(SRC_ IN LISTS SOURCES)
  set_source_files_properties(${SRC_} PROPERTIES COMPILE_OPTIONS "-Wall" )
  set_source_files_properties(${SRC_} PROPERTIES COMPILE_OPTIONS "-Wextra" )
endforeach()

add_executable(index.html ${SOURCES} ${NANOGUI_LIB_SOURCES})

add_custom_target(server COMMAND python3 -m http.server)
add_custom_target(webinstall COMMAND cp index.html index.js index.wasm ${CMAKE_CURRENT_SOURCE_DIR}/docs )

