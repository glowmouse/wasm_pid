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
    mAngle       = degToRad(mFrontEnd->getStartAngle());
    mAngleVel= 0;
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

    //if ( mFrontEnd->isNewSettings()) {
    //}
    mSlowTime = mFrontEnd->isSlowTime();
    if ( mFrontEnd->isNudgeUp()) {
      mAngleVel += 3;
    }
    if ( mFrontEnd->isNudgeDown()) {
      mAngleVel -= 3;
    }
    if ( mFrontEnd->isWackUp()) {
      mAngleVel += 10;
    }
    if ( mFrontEnd->isWackDown()) {
      mAngleVel -= 10;
    }

    ++mCounter0;
    if ( mSlowTime ) {
      if ( (mCounter0 % 10 ) != 0 ) {
        return;
      }
    }

    ++mCounter1;

    // Run simulation 50x a second.
    double timeSlice = 1.0/((double) updatesPerSecond);

    double armX = cos( mAngle );
    double armY = sin( mAngle );
    // Gravity = < 0   , -9.8 >
    // Arm     = < armX, armY >
    // Angular accelleration = Arm x Gravity
    double AngleAccel = armX * -9.8;

    // Compute and record the error.

    int sampleInterval = updatesPerSecond / mFrontEnd->getSamplesPerSecond();

    double pError = mAngle - mTargetAngle;
    mIError += pError;
    double iError = mIError * timeSlice; 
    double dError = (pError - mLastPError) / timeSlice / 5.0;
    mLastPError = pError;

    double pTerm = pError * mPidP;
    double iTerm = iError * mPidI;
    double dTerm = dError * mPidD;

    double all = pTerm + iTerm + dTerm;
    all = std::max(-4.0, std::min( all, 4.0 ));

    AngleAccel -= all/5/timeSlice;

    // Accellaration to Velocity integration
    mAngleVel += AngleAccel * timeSlice;
    if ( mAngleVel > 0 ) {
      mAngleVel = std::max(mAngleVel - mStaticFriction * timeSlice, 0.0 );
    }
    else {
      mAngleVel = std::min(mAngleVel + mStaticFriction * timeSlice, 0.0 );
    }
    mAngleVel *= 1-mRollingFriction;

    // Velocity to Angle integration
    mAngle += mAngleVel * timeSlice;

    //
    // Impose some hard limits.  -120 degrees is about the angle where
    // the final segment of the robot arm hits the second segment,
    // visually.
    //
    if ( mAngle < degToRad( -120 )) {
      mAngle    = degToRad( -120 );
      mAngleVel = 0.0f;  // A hard stop kills all velocity
    }

    //
    // Second limit, where the final segment hits the second segment, but
    // in the other direction.
    //
    if ( mAngle > degToRad( 210 )) {
      mAngle    = degToRad( 210 );
      mAngleVel = 0.0f;  // Again, a hard stop kills all velocity
    }


    if( (mCounter1 % sampleInterval ) == 0 ) {
      mFrontEnd->recordActualError( radToDeg(pError), radToDeg( iError ), radToDeg( dError), -all*25);
    }
    updateFrontEnd();
  }

  void PidSimBackEnd::updateFrontEnd()
  {
    mFrontEnd->setArmAngle( mAngle );
  }


