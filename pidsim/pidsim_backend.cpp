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
  mIError = 0;
}

void BackEnd::getInputFromFrontEnd()
{
  if ( mFrontEnd->isReset() ) {
    reset();
  }

  mPidP = mFrontEnd->getP();
  mPidI = mFrontEnd->getI();
  mPidD = mFrontEnd->getD();
  mRollingFriction = mFrontEnd->getRollingFriction()/50.0;
  mTargetAngle = Utils::degToRad(mFrontEnd->getTargetAngle());
  mArmState->setSensorNoise( mFrontEnd->getSensorNoise() );
  mArmState->setSensorDelay( mFrontEnd->getSensorDelay() );
  mArmState->setMotorDelay( mFrontEnd->getMotorDelay() );
  mSlowTime = mFrontEnd->isSlowTime();

  if ( mLastPidI != mPidI ) {
    mIError = 0;
    mLastPidI = mPidI;
  }

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
  getInputFromFrontEnd();

  ++mCounter0;
  if ( mSlowTime ) {
    if ( (mCounter0 % 10 ) != 0 ) {
      return;
    }
  }

  // Run simulation about 50x a second.
  const double timeSlice = 1.0/((double) updatesPerSecond);
  const double motorPower = updatePidController( timeSlice );
  updateRobotArmSimulation( timeSlice, motorPower  );
  updateFrontEnd();
}


double BackEnd::updatePidController( double timeSlice )
{
  const double pError = mArmState->getSensorAngle() - mTargetAngle;
  mIError += pError;
  const double iError = mIError * timeSlice; 
  const double dError = (pError - mLastPError) / timeSlice / 5.0;
  mLastPError = pError;

  const double pTerm = pError * mPidP;
  const double iTerm = iError * mPidI;
  const double dTerm = dError * mPidD;

  const double allTerms = pTerm + iTerm + dTerm;
  const double motorPower = std::max(-4.0, std::min( -allTerms, 4.0 )) / 5.0;
  sendErrorToFrontEnd( pError, iError, dError );
  return motorPower;
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

void BackEnd::sendErrorToFrontEnd( double pError, double dError, double iError )
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


}


