#ifndef __PIDSIM_BACKEND_H__
#define __PIDSIM_BACKEND_H__

#include "pidsim_frontend.h"

namespace PidSim {

class BackEndState;

class BackEnd
{
  public:

  BackEnd( nanogui::ref<FrontEnd> frontEnd );
  BackEnd() = delete;
  BackEnd( const BackEnd& other ) = delete;
  BackEnd& operator=( BackEnd& other ) = delete;
  ~BackEnd();

  void update( std::chrono::duration<double> delta );
 

  private:

  void reset();
  void updateOneTick();
  void updateFrontEnd();
  void sendErrorToFrontEnd( double pError, double dError, double iError );
  void updateRobotArmSimulation( double timeSlice, double motorPower );
  void getInputFromFrontEnd();
  double updatePidController( double timeSlice );

  static constexpr int updatesPerSecond = 50;
  static constexpr unsigned int slowTimeScale    = 10;

  double time = 0.0f;
  // map mAngle to "screen space" with <x,y> = <cos(mAngle),sin(mAngle)>

  double mPidP = 0;
  double mPidI = 0;
  double mLastPidI = 0;
  double mPidD = 0;
  double mRollingFriction = 0;
  double mIError = 0;
  double mLastPError = 0;
  double mTargetAngle = 0;

  bool mSlowTime = false;
  unsigned int mCounter0=0;
  unsigned int mCounter1=0;

  nanogui::ref<FrontEnd> mFrontEnd;
  std::unique_ptr<BackEndState> mArmState;
};

}

#endif

