# PID Simulator

A PID Simulator written in Web Assembly.

## Summary

The goal of this project is to help students with a high school math 
background (specifically students competing in First Robotics) understand 
PID controllers.  Many of the descriptions of how PID controllers work
(example: https://en.wikipedia.org/wiki/PID_controller) assume
math background that isn't until the senior years of high school.

Web page link to try it out:  https://glowmouse.github.io/wasm_pid/

## Kudos

This page was built using example code from https://github.com/AE1020/nanogui-GLES-wasm

## Top level Directory Layout

| Directory   | Description                                                |
| ----------- | ---------------------------------------------------------- |
| docs        | The web page generated by the project                      |
| ext         | Libraries from external sources                            |
| include     | Include files for the nanogui toolkit                      |
| models      | 3D STL models for the PID Simulator Robot Arm + Converter  |
| nanogui_src | Source files for the nongui toolkit                        |
| pidsim      | The PID simulator source C++ source code                   |

## Building (Linux)

1.  Install the emsdk webassembly compiler (see https://emscripten.org/docs/getting_started/downloads.html for instructions)
2.  As per the instructions on https://emscripten.org/docs/getting_started/downloads.html, source emsdk_env.sh (i.e., source "emsdk/emsdk_env.sh")
3.  Clone the wasm_pid repository
4.  In the source directory, git clone https://github.com/libigl/eigen.git ext/eigen.  This copies the eigen library into the source tree.
5.  Make a separate build directory (i.e., mkdir build_wasm_pid) and cd into it
6.  Run cmake (i.e., if wasm_pid and build_wasm_pid are in the same directory cmake ../wasm_pid -B .)
7.  make

## Running (Linux)

1.  In a separate window, make server.  This will start a mini web server.
2.  In a browser, go to http://localhost:8000

The web page can be hosted by either taking the docs directory and copying the
files into the web page directory or having github host the docs directory 
See https://pages.github.com/ for information on setting up a github page.

