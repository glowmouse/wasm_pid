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
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <emscripten.h>
#include "model.h"  // auto generated code.
#include <memory>
#include <utility>
#include <chrono>
#include <numeric>
#include "pidsim_frontend.h"
#include "pidsim_utils.h"

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

class PidSimBackEnd
{
  public:

  PidSimBackEnd( nanogui::ref<PidSimFrontEnd> frontEnd ) : mFrontEnd{ frontEnd }
  {
    reset();
  }

  void update( std::chrono::duration<double> delta )
  {
    double intervalSeconds= delta.count();
    int lastTicks= (int) (time * 50);
    time += intervalSeconds;
    int curTicks= (int) (time * 50);
    int ticksDiff = curTicks - lastTicks;
    for ( int i = 0; i < ticksDiff; ++i ) {
      updateOneTick();
    }
  } 
 
  private:

  void softReset()
  {
    mPidP = mFrontEnd->getP();
    mPidI = mFrontEnd->getI();
    mPidD = mFrontEnd->getD();
    mRollingFriction = mFrontEnd->getRollingFriction()/50.0;
    mStaticFriction= mFrontEnd->getStaticFriction();
    mTargetAngle = degToRad(mFrontEnd->getTargetAngle());
  }

  void reset()
  {
    mAngle       = degToRad(mFrontEnd->getStartAngle());
    mAngleVel= 0;
    mFrontEnd->resetErrorRecord();
    mIError = 0;
    softReset();
  }

  void updateOneTick()
  {
    if ( mFrontEnd->isReset() ) {
      reset();
    }
    softReset();

    if ( mLastPidI != mPidI ) {
      mIError = 0;
      mLastPidI = mPidI;
    }

    //if ( mFrontEnd->isNewSettings()) {
    //}
    mSlowTime = mFrontEnd->isSlowTime();
    if ( mFrontEnd->isNudgeUp()) {
      mAngleVel += 3;
    }
    if ( mFrontEnd->isNudgeDown()) {
      mAngleVel -= 3;
    }
    if ( mFrontEnd->isWackUp()) {
      mAngleVel += 10;
    }
    if ( mFrontEnd->isWackDown()) {
      mAngleVel -= 10;
    }

    ++mCounter0;
    if ( mSlowTime ) {
      if ( (mCounter0 % 10 ) != 0 ) {
        return;
      }
    }

    ++mCounter1;

    // Run simulation 50x a second.
    double timeSlice = 1.0/((double) updatesPerSecond);

    double armX = cos( mAngle );
    double armY = sin( mAngle );
    // Gravity = < 0   , -9.8 >
    // Arm     = < armX, armY >
    // Angular accelleration = Arm x Gravity
    double AngleAccel = armX * -9.8;

    // Compute and record the error.

    int sampleInterval = updatesPerSecond / mFrontEnd->getSamplesPerSecond();

    double pError = mAngle - mTargetAngle;
    mIError += pError;
    double iError = mIError * timeSlice; 
    double dError = (pError - mLastPError) / timeSlice / 5.0;
    mLastPError = pError;

    double pTerm = pError * mPidP;
    double iTerm = iError * mPidI;
    double dTerm = dError * mPidD;

    double all = pTerm + iTerm + dTerm;
    all = std::max(-4.0, std::min( all, 4.0 ));

    AngleAccel -= all/5/timeSlice;

    // Accellaration to Velocity integration
    mAngleVel += AngleAccel * timeSlice;
    if ( mAngleVel > 0 ) {
      mAngleVel = std::max(mAngleVel - mStaticFriction * timeSlice, 0.0 );
    }
    else {
      mAngleVel = std::min(mAngleVel + mStaticFriction * timeSlice, 0.0 );
    }
    mAngleVel *= 1-mRollingFriction;

    // Velocity to Angle integration
    mAngle += mAngleVel * timeSlice;

    //
    // Impose some hard limits.  -120 degrees is about the angle where
    // the final segment of the robot arm hits the second segment,
    // visually.
    //
    if ( mAngle < degToRad( -120 )) {
      mAngle    = degToRad( -120 );
      mAngleVel = 0.0f;  // A hard stop kills all velocity
    }

    //
    // Second limit, where the final segment hits the second segment, but
    // in the other direction.
    //
    if ( mAngle > degToRad( 210 )) {
      mAngle    = degToRad( 210 );
      mAngleVel = 0.0f;  // Again, a hard stop kills all velocity
    }


    if( (mCounter1 % sampleInterval ) == 0 ) {
      mFrontEnd->recordActualError( radToDeg(pError), radToDeg( iError ), radToDeg( dError), -all*25);
    }
    updateFrontEnd();
  }

  void updateFrontEnd()
  {
    mFrontEnd->setArmAngle( mAngle );
  }

  private:

  static constexpr int updatesPerSecond = 50;

  double time = 0.0f;
  // map mAngle to "screen space" with <x,y> = <cos(mAngle),sin(mAngle)>
  double mAngle = 0;
  double mAngleVel = 0;

  double mPidP = 0;
  double mPidI = 0;
  double mLastPidI = 0;
  double mPidD = 0;
  double mRollingFriction = 0;
  double mStaticFriction= 0;
  double mIError = 0;
  double mLastPError = 0;
  double mTargetAngle = 0;

  bool mSlowTime = false;
  unsigned int mCounter0=0;
  unsigned int mCounter1=0;

  nanogui::ref<PidSimFrontEnd> mFrontEnd;
};

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
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
