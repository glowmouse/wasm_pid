#include "pidsim_backend_pid_controller.h"
#include <algorithm>

namespace PidSim {

// See header for interface
void PidController::updatePidSettings( double pidP, double pidI, double pidD, double targetAngle )
{
  // If the new I setting is different than the old one, reset the accumulated error.
  if ( pidI != mPidI ) { mIError = 0; }

  mPidP = pidP;
  mPidI = pidI;
  mPidD = pidD;
  mTargetAngle = targetAngle;
}

// See header for interface.
PidController::Output
PidController::updatePidController( double timeSlice, double sensorInputAngle )
{
  // Compute new values for the P, I, and D errors
  const double pError = sensorInputAngle - mTargetAngle;
  mIError += pError;
  const double iError = mIError * timeSlice; 
  const double dError = (pError - mLastPError) / timeSlice / 5.0;

  // Record the current proportional error so we can compute derivative error next iteration
  mLastPError = pError;

  // Compute the P, I, and D gains, them add them together.
  const double pGain = pError * mPidP;
  const double iGain = iError * mPidI;
  const double dGain = dError * mPidD;
  const double allGains = pGain + iGain + dGain;
  
  // Compute the new motorPower.  Make sure the output power is within some reasonable
  // range.  Motors are not infinitely powerful.  TODO - clean up constants here.
  const double motorPower = std::max(-4.0, std::min( -allGains, 4.0 )) / 5.0;

  return Output{ pError, iError, dError, motorPower };
}

// see header for interface.
void PidController::reset()
{
  // Reset the accumulated integral error in the controller.
  mIError = 0;
}


}


