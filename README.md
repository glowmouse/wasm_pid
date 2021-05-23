# PID Simulator

A PID Simulator written in Web Assembly.

The goal of this project is to help students with a high school math 
background (specifically students competing in First Robotics) understand 
PID controllers.  Many of the descriptions of how PID controllers work
(example: https://en.wikipedia.org/wiki/PID_controller) assume
math background that isn't until the senior years of high school.

Web page link to try it out:  https://glowmouse.github.io/wasm_pid/

Building (Linux)

1.  Install the emsdk webassembly compiler (see https://emscripten.org/docs/getting_started/downloads.html for instructions)
2.  As per the instructions on https://emscripten.org/docs/getting_started/downloads.html, source emsdk_env.sh (i.e., source "emsdk/emsdk_env.sh")
3.  Clone the wasm_pid repository
4.  Make a separate build directory (i.e., mkdir build_wasm_pid) and cd into it
5.  Run cmake (i.e., cmake ../wasm_pid -B .)
6.  make

Running (Linux)

1.  In a separate window, make server.  This will start a mini web server.
2.  In a browser, go to http://localhost:8000

