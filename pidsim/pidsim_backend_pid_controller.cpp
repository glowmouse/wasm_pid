#include "pidsim_backend_pid_controller.h"
#include <algorithm>

namespace PidSim {

void PidController::updatePidSettings(
    double pidP,
    double pidI,
    double pidD,
    double targetAngle )
{
  mPidP = pidP;
  if ( pidI != mPidI ) {
    mPidI = pidI;
    mIError = 0;
  }
  mPidD = pidD;
  mTargetAngle = targetAngle;
}

PidControllerOutput
PidController::updatePidController( double timeSlice, double sensorInputAngle )
{
  const double pError = sensorInputAngle - mTargetAngle;
  mIError += pError;
  const double iError = mIError * timeSlice; 
  const double dError = (pError - mLastPError) / timeSlice / 5.0;
  mLastPError = pError;

  const double pTerm = pError * mPidP;
  const double iTerm = iError * mPidI;
  const double dTerm = dError * mPidD;

  const double allTerms = pTerm + iTerm + dTerm;
  const double motorPower = std::max(-4.0, std::min( -allTerms, 4.0 )) / 5.0;
  return PidControllerOutput( pError, iError, dError, motorPower );
}

void PidController::reset()
{
  mIError = 0;
}


}


