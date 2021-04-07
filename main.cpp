/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
#include "pidsim_backend.h"
#include <emscripten.h>
#include <iostream>

std::unique_ptr<PidSimBackEnd> backEndSingleton;

static std::chrono::high_resolution_clock::time_point lastFpsTime;
static std::chrono::high_resolution_clock::time_point lastTickTime;
double frameRateSmoothing = 1.0;
double numFrames = 0;

void mainloop(){
	std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>> (std::chrono::high_resolution_clock::now() - lastFpsTime);
  std::chrono::high_resolution_clock::time_point timeNow = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> backDelta = std::chrono::duration_cast<std::chrono::duration<double>> (timeNow - lastTickTime );
  lastTickTime = timeNow;

	numFrames++;
	if (delta.count() > frameRateSmoothing) {
    numFrames = 0;
    lastFpsTime = std::chrono::high_resolution_clock::now();
  }

  backEndSingleton->update( backDelta ); 
	nanogui::mainloop();
}

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();
        {
          //
          // Note - ref is a custom shared pointer.  The reference
          // count is integrated into nanogui's object.
          //
          nanogui::ref<PidSimFrontEnd> pidSimFrontEnd = new PidSimFrontEnd();
          backEndSingleton = std::make_unique<PidSimBackEnd>( pidSimFrontEnd );
          pidSimFrontEnd->drawAll();
          pidSimFrontEnd->setVisible(true);
          emscripten_set_main_loop(mainloop, 0,1);
        }
        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << std::endl;
        #endif
        return -1;
    }

    return 0;
}
