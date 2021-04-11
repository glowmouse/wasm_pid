#ifndef __PIDSIM_BACKEND_PID_CONTROLLER_H__
#define __PIDSIM_BACKEND_PID_CONTROLLER_H__

namespace PidSim {

struct PidControllerOutput
{
  PidControllerOutput( double pError, double iError, double dError, double motorPower ) :
  mPError{ pError }, mIError{ iError }, mDError{ dError }, mMotorPower{ motorPower }
  {}

  double mPError;
  double mIError;
  double mDError;
  double mMotorPower;
};

class PidController
{
  public:

  void updatePidSettings(double PidP, double PidI, double PidD, double TargetAngle );

  PidControllerOutput
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

}

#endif

