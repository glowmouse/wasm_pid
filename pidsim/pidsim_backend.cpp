#include "pidsim_backend.h"
#include "pidsim_backend_state.h"
#include "pidsim_utils.h"

namespace PidSim {

BackEnd::BackEnd( nanogui::ref<FrontEnd> frontEnd ) : 
  mFrontEnd{ frontEnd },
  mArmState{ std::make_unique<BackEndState>( mFrontEnd->getStartAngle()) }
{
  reset();
}

BackEnd::~BackEnd()
{
}

void BackEnd::update( std::chrono::duration<double> delta )
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
 
void BackEnd::reset()
{
  mArmState = std::make_unique< BackEndState >( 
    Utils::degToRad(mFrontEnd->getStartAngle() ));
  mFrontEnd->resetErrorRecord();
  mPidController.reset();
}

void BackEnd::getInputFromFrontEnd()
{
  if ( mFrontEnd->isReset() ) {
    reset();
  }

  mPidController.updatePidSettings(
      mFrontEnd->getP(),
      mFrontEnd->getI(),
      mFrontEnd->getD(),
      Utils::degToRad(mFrontEnd->getTargetAngle())
  );

  mRollingFriction = mFrontEnd->getRollingFriction()/50.0;
  mArmState->setSensorNoise( mFrontEnd->getSensorNoise() );
  mArmState->setSensorDelay( mFrontEnd->getSensorDelay() );
  mArmState->setMotorDelay( mFrontEnd->getMotorDelay() );
  mSlowTime = mFrontEnd->isSlowTime();

  if ( mFrontEnd->isNudgeUp()) {
    mArmState->bump( 3 );
  }
  if ( mFrontEnd->isNudgeDown()) {
    mArmState->bump( -3 );
  }
  if ( mFrontEnd->isWackUp()) {
    mArmState->bump( 10 );
  }
  if ( mFrontEnd->isWackDown()) {
    mArmState->bump( -10 );
  }
}

void BackEnd::updateOneTick()
{
  // Check for input
  getInputFromFrontEnd();

  // Slow time logic
  ++mCounter0;
  if ( mSlowTime && ( mCounter0 % slowTimeScale ) != 0 ) { return; }

  // Advance the simulation 1/50th of a second
  const double timeSlice = 1.0/((double) updatesPerSecond);

  // Run the PID controller
  const PidControllerOutput pOut = mPidController.updatePidController( timeSlice, mArmState->getSensorAngle() );

  // Update the error graph
  sendErrorToFrontEnd( pOut.mPError, pOut.mIError, pOut.mDError );

  // Advance the robot arm simulation
  updateRobotArmSimulation( timeSlice, pOut.mMotorPower );

  // Send new positions to the front end.
  updateFrontEnd();
}

void BackEnd::updateRobotArmSimulation( double timeSlice, double motorPower )
{
  mArmState->startSimulationIteration();
  mArmState->applyGravity( timeSlice );
  mArmState->applyMotor( motorPower, timeSlice );
  mArmState->updateAngleVel( timeSlice );
  mArmState->applyFriction( mRollingFriction, timeSlice );
  mArmState->updateAngle( timeSlice );
  mArmState->imposePositionHardLimits();
  mArmState->endSimulationIteration();
}

void BackEnd::sendErrorToFrontEnd( double pError, double iError, double dError )
{
  const int sampleInterval = updatesPerSecond / mFrontEnd->getSamplesPerSecond();

  ++mCounter1;
  if( (mCounter1 % sampleInterval ) == 0 ) {
    mFrontEnd->recordActualError( 
      Utils::radToDeg( pError ), 
      Utils::radToDeg( iError ), 
      Utils::radToDeg( dError ), 
      mArmState->getMotorPower() * 150 );
  }
}

void BackEnd::updateFrontEnd()
{
  mFrontEnd->setArmAngle( mArmState->getActualAngle() );
}

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


