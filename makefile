# Very quick/dirty makefile for building the nanogui main.cpp in emscripten
# Puts the result in the `docs/` directory
#
# Makefile tips:
#
#   * GNU make wants tab indentation, will error on spaces
#   * `$@` means substitute the target
#   * `$<` means substitute first thing in dependency list
#   * `$^` means substitute everything in the dependency list

CC=em++

NANOFLAGS=-DNANOGUI_LINUX -DNANOVG_GLES3_IMPLEMENTATION
GLFLAGS=-DGLFW_INCLUDE_ES3 -DGLFW_INCLUDE_GLEXT -s USE_GLFW=3 -s FULL_ES3=1 -s USE_WEBGL2=1 -s ASSERTIONS=1 --shell-file ${EMSDK}/upstream/emscripten/src/shell_minimal.html
EMFLAGS=--std=c++17 -O -s WASM=1
INCFLAGS=-Iinclude/ -Iext/nanovg/ -Iext/eigen/
LDFLAGS=-lGL -lm -lGLEW

CFLAGS=$(INCFLAGS) $(EMFLAGS) $(NANOFLAGS) $(GLFLAGS)

docs/index.html: pidsim/pidsim_main.wasm pidsim/pidsim_backend.wasm pidsim/pidsim_frontend.wasm pidsim/pidsim_model.wasm nanovg.wasm nanogui_src/button.wasm nanogui_src/checkbox.wasm nanogui_src/colorpicker.wasm nanogui_src/colorwheel.wasm nanogui_src/combobox.wasm nanogui_src/common.wasm nanogui_src/glcanvas.wasm nanogui_src/glutil.wasm nanogui_src/graph.wasm nanogui_src/imagepanel.wasm nanogui_src/imageview.wasm nanogui_src/label.wasm nanogui_src/layout.wasm nanogui_src/messagedialog.wasm nanogui_src/popup.wasm nanogui_src/popupbutton.wasm nanogui_src/progressbar.wasm nanogui_src/screen.wasm nanogui_src/serializer.wasm nanogui_src/slider.wasm nanogui_src/stackedwidget.wasm nanogui_src/tabheader.wasm nanogui_src/tabwidget.wasm nanogui_src/textbox.wasm nanogui_src/theme.wasm nanogui_src/vscrollpanel.wasm nanogui_src/widget.wasm nanogui_src/window.wasm nanogui_src/nanogui_resources.wasm
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

nanovg.wasm: ext/nanovg.c
	$(CC) $(CFLAGS) -c $< -o $@

%.wasm: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	$(RM) *.wasm nanogui_src/*.wasm pidsim/*.wasm

