#ifndef __PIDSIM_BACKEND_H__
#define __PIDSIM_BACKEND_H__

#include "pidsim_backend_state.h"

class PidSimBackEnd
{
  public:

  PidSimBackEnd( nanogui::ref<PidSimFrontEnd> frontEnd );

  void update( std::chrono::duration<double> delta );
 
  private:

  void softReset();
  void reset();
  void updateOneTick();
  void updateFrontEnd();

  private:

  static constexpr int updatesPerSecond = 50;

  PidSimBackEndState mArmState;

  double time = 0.0f;
  // map mAngle to "screen space" with <x,y> = <cos(mAngle),sin(mAngle)>

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

#endif

