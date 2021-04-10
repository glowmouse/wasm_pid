#ifndef __PIDSIM_BACKEND_H__
#define __PIDSIM_BACKEND_H__

#include "pidsim_frontend.h"

namespace PidSim {

class BackEndState;

class PidController
{
  public:

  void updatePidSettings(double PidP, double PidI, double PidD, double TargetAngle );

  std::tuple< double, double, double, double> 
  updatePidController( double timeSlice, double sensorInputAngle );
  void reset();

  private:
  double mPidP = 0;
  double mPidI = 0;
  double mPidD = 0;
  double mIError = 0;
  double mLastPError = 0;
  double mTargetAngle = 0;
};

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
  void updateRobotArmSimulation( double timeSlice, double motorPower );
  void getInputFromFrontEnd();
  void sendErrorToFrontEnd( double pError, double iError, double dError );

  static constexpr int updatesPerSecond = 50;
  static constexpr unsigned int slowTimeScale    = 10;

  double time = 0.0f;
  // map mAngle to "screen space" with <x,y> = <cos(mAngle),sin(mAngle)>

  double mRollingFriction = 0;

  bool mSlowTime = false;
  unsigned int mCounter0=0;
  unsigned int mCounter1=0;

  PidController                   mPidController;
  nanogui::ref<FrontEnd>          mFrontEnd;
  std::unique_ptr<BackEndState>   mArmState;
};

}

#endif

