#ifndef __PIDSIM_BACKEND_H__
#define __PIDSIM_BACKEND_H__

#include "pidsim_frontend.h"

namespace PidSim {

// Forward declare the PID Controller & Simulation classess.
class BackEndState;
class PidController;

class BackEnd
{
  public:

  /// @brief Constructor
  ///
  /// @param[in/out] frontEnd - A nanogui smart ptr to the GUI
  ///
  BackEnd( nanogui::ref<FrontEnd> frontEnd );

  /// @brief Destructor
  ///
  ~BackEnd();

  // Remove operations people shouldn't be using.
  BackEnd() = delete;
  BackEnd( const BackEnd& other ) = delete;
  BackEnd& operator=( BackEnd& other ) = delete;

  /// @brief Update the backend state.
  ///
  /// @param[in] delta - The time in seconds since the last call to update
  ///
  void update( std::chrono::duration<double> delta );

  private:

  void reset();
  void updateOneTick();
  void updateFrontEnd();
  void updateRobotArmSimulation( double timeSlice, double motorPower );
  void getInputFromFrontEnd();
  void sendErrorToFrontEnd( double pError, double iError, double dError );

  static constexpr int        updatesPerSecond = 50;      // 50 sim updates/ sec
  static constexpr unsigned   slowTimeScale    = 10;      // "slow time" slows by 10x
  bool                        mSlowTime         = false;  // slow time starts disabled

  double                      time              = 0.0;    // time since sim start, in secs
  double                      mRollingFriction  = 0.0;    // friction slowing the robot arm

  unsigned int                mCounter0         =0;       // A counter the flow control
  unsigned int                mCounter1         =0;       // A separate counter :)

  nanogui::ref<FrontEnd>           mFrontEnd;             // The front end GUI
  std::unique_ptr<BackEndState>    mArmState;             // Robot arm physics simulation
  std::unique_ptr<PidController>   mPidController;        // PID controller
};

}

#endif

