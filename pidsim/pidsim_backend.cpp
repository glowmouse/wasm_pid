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
 
void BackEnd::softReset()
{
  mPidP = mFrontEnd->getP();
  mPidI = mFrontEnd->getI();
  mPidD = mFrontEnd->getD();
  mRollingFriction = mFrontEnd->getRollingFriction()/50.0;
  mTargetAngle = Utils::degToRad(mFrontEnd->getTargetAngle());
  mArmState->setSensorNoise( mFrontEnd->getSensorNoise() );
  mArmState->setSensorDelay( mFrontEnd->getSensorDelay() );
  mArmState->setMotorDelay( mFrontEnd->getMotorDelay() );
}

void BackEnd::reset()
{
  mArmState = std::make_unique< BackEndState >( 
    Utils::degToRad(mFrontEnd->getStartAngle() ));
  mFrontEnd->resetErrorRecord();
  mIError = 0;
  softReset();
}

void BackEnd::updateOneTick()
{
  if ( mFrontEnd->isReset() ) {
    reset();
  }
  softReset();

  if ( mLastPidI != mPidI ) {
    mIError = 0;
    mLastPidI = mPidI;
  }

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

  ++mCounter0;
  if ( mSlowTime ) {
    if ( (mCounter0 % 10 ) != 0 ) {
      return;
    }
  }

  // Run simulation about 50x a second.
  double timeSlice = 1.0/((double) updatesPerSecond);

  // Compute and record the error.

  int sampleInterval = updatesPerSecond / mFrontEnd->getSamplesPerSecond();

  double pError = mArmState->getSensorAngle() - mTargetAngle;
  mIError += pError;
  double iError = mIError * timeSlice; 
  double dError = (pError - mLastPError) / timeSlice / 5.0;
  mLastPError = pError;

  double pTerm = pError * mPidP;
  double iTerm = iError * mPidI;
  double dTerm = dError * mPidD;

  double allTerms = pTerm + iTerm + dTerm;
  double motor = std::max(-4.0, std::min( -allTerms, 4.0 )) / 5.0;

  mArmState->startSimulationIteration();
  mArmState->applyGravity( timeSlice );
  mArmState->applyMotor( motor, timeSlice );
  mArmState->updateAngleVel( timeSlice );
  mArmState->applyFriction( mRollingFriction, timeSlice );
  mArmState->updateAngle( timeSlice );
  mArmState->imposePositionHardLimits();
  mArmState->endSimulationIteration();

  ++mCounter1;
  if( (mCounter1 % sampleInterval ) == 0 ) {
    mFrontEnd->recordActualError( Utils::radToDeg(pError), Utils::radToDeg( iError ), Utils::radToDeg( dError), mArmState->getMotorPower() * 150 );
  }
  updateFrontEnd();
}

void BackEnd::updateFrontEnd()
{
  mFrontEnd->setArmAngle( mArmState->getActualAngle() );
}

}


