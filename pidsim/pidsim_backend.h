#ifndef __PIDSIM_BACKEND_H__
#define __PIDSIM_BACKEND_H__

#include "pidsim_frontend.h"
#include "pidsim_backend_pid_controller.h"

namespace PidSim {

class BackEndState;

class BackEnd
{
  public:

  BackEnd( nanogui::ref<FrontEnd> frontEnd );
  ~BackEnd();

  // Remove operations people shouldn't be using.
  BackEnd() = delete;
  BackEnd( const BackEnd& other ) = delete;
  BackEnd& operator=( BackEnd& other ) = delete;

  void update( std::chrono::duration<double> delta );

  private:

  void reset();
  void updateOneTick();
  void updateFrontEnd();
  void updateRobotArmSimulation( double timeSlice, double motorPower );
  void getInputFromFrontEnd();
  void sendErrorToFrontEnd( double pError, double iError, double dError );

  static constexpr int            updatesPerSecond = 50;
  static constexpr unsigned int   slowTimeScale    = 10;

  double                          time              = 0.0f;
  double                          mRollingFriction  = 0;
  bool                            mSlowTime         = false;

  unsigned int                    mCounter0         =0;
  unsigned int                    mCounter1         =0;

  PidController                   mPidController;
  nanogui::ref<FrontEnd>          mFrontEnd;
  std::unique_ptr<BackEndState>   mArmState;
};

}

#endif

