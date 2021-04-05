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

docs/index.html: main.cpp nanovg.wasm button.wasm checkbox.wasm colorpicker.wasm colorwheel.wasm combobox.wasm common.wasm glcanvas.wasm glutil.wasm graph.wasm imagepanel.wasm imageview.wasm label.wasm layout.wasm messagedialog.wasm popup.wasm popupbutton.wasm progressbar.wasm screen.wasm serializer.wasm slider.wasm stackedwidget.wasm tabheader.wasm tabwidget.wasm textbox.wasm theme.wasm vscrollpanel.wasm widget.wasm window.wasm nanogui_resources.wasm model.wasm
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ --preload-file ./icons

nanovg.wasm: ext/nanovg.c
	$(CC) $(CFLAGS) -c $< -o $@

%.wasm: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	$(RM) *.wasm

