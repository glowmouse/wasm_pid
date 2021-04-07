#include "pidsim_backend.h"

PidSimBackEnd::PidSimBackEnd( nanogui::ref<PidSimFrontEnd> frontEnd ) : mFrontEnd{ frontEnd }
{
  reset();
}

void PidSimBackEnd::update( std::chrono::duration<double> delta )
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
 
void PidSimBackEnd::softReset()
{
  mPidP = mFrontEnd->getP();
  mPidI = mFrontEnd->getI();
  mPidD = mFrontEnd->getD();
  mRollingFriction = mFrontEnd->getRollingFriction()/50.0;
  mStaticFriction= mFrontEnd->getStaticFriction();
  mTargetAngle = degToRad(mFrontEnd->getTargetAngle());
}

void PidSimBackEnd::reset()
{
  mArmState.reset( degToRad(mFrontEnd->getStartAngle() ));
  mFrontEnd->resetErrorRecord();
  mIError = 0;
  softReset();
}

void PidSimBackEnd::updateOneTick()
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
    mArmState.bump( 3 );
  }
  if ( mFrontEnd->isNudgeDown()) {
    mArmState.bump( -3 );
  }
  if ( mFrontEnd->isWackUp()) {
    mArmState.bump( 10 );
  }
  if ( mFrontEnd->isWackDown()) {
    mArmState.bump( -10 );
  }

  ++mCounter0;
  if ( mSlowTime ) {
    if ( (mCounter0 % 10 ) != 0 ) {
      return;
    }
  }

  ++mCounter1;

  // Run simulation about 50x a second.
  double timeSlice = 1.0/((double) updatesPerSecond);

  mArmState.resetAngAccel();
  mArmState.applyGravity( timeSlice );

  // Compute and record the error.

  int sampleInterval = updatesPerSecond / mFrontEnd->getSamplesPerSecond();

  double pError = mArmState.getAngle() - mTargetAngle;
  mIError += pError;
  double iError = mIError * timeSlice; 
  double dError = (pError - mLastPError) / timeSlice / 5.0;
  mLastPError = pError;

  double pTerm = pError * mPidP;
  double iTerm = iError * mPidI;
  double dTerm = dError * mPidD;

  double all = pTerm + iTerm + dTerm;
  all = std::max(-4.0, std::min( all, 4.0 ));

  mArmState.applyMotor( -all / 5, timeSlice );

  // Accellaration to Velocity integration
  mArmState.updateAngleVel( timeSlice );

  mArmState.applyFriction( mStaticFriction, mRollingFriction, timeSlice );

  // Velocity to Angle integration
  mArmState.updateAngle( timeSlice );
  mArmState.imposePositionHardLimits();

  if( (mCounter1 % sampleInterval ) == 0 ) {
    mFrontEnd->recordActualError( radToDeg(pError), radToDeg( iError ), radToDeg( dError), -all*25);
  }
  updateFrontEnd();
}

void PidSimBackEnd::updateFrontEnd()
{
  mFrontEnd->setArmAngle( mArmState.getAngle() );
}


