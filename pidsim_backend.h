#ifndef __PIDSIM_BACKEND_H__
#define __PIDSIM_BACKEND_H__

#include "pidsim_frontend.h"
#include "pidsim_utils.h"

class PidSimBackEnd
{
  public:

  PidSimBackEnd( nanogui::ref<PidSimFrontEnd> frontEnd ) : mFrontEnd{ frontEnd }
  {
    reset();
  }

  void update( std::chrono::duration<double> delta )
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
 
  private:

  void softReset()
  {
    mPidP = mFrontEnd->getP();
    mPidI = mFrontEnd->getI();
    mPidD = mFrontEnd->getD();
    mRollingFriction = mFrontEnd->getRollingFriction()/50.0;
    mStaticFriction= mFrontEnd->getStaticFriction();
    mTargetAngle = degToRad(mFrontEnd->getTargetAngle());
  }

  void reset()
  {
    mAngle       = degToRad(mFrontEnd->getStartAngle());
    mAngleVel= 0;
    mFrontEnd->resetErrorRecord();
    mIError = 0;
    softReset();
  }

  void updateOneTick()
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

  void updateFrontEnd()
  {
    mFrontEnd->setArmAngle( mAngle );
  }

  private:

  static constexpr int updatesPerSecond = 50;

  double time = 0.0f;
  // map mAngle to "screen space" with <x,y> = <cos(mAngle),sin(mAngle)>
  double mAngle = 0;
  double mAngleVel = 0;

  double mPidP = 0;
  double mPidI = 0;
  double mLastPidI = 0;
  double mPidD = 0;
  double mRollingFriction = 0;
  double mStaticFriction= 0;
  double mIError = 0;
  double mLastPError = 0;
  double mTargetAngle = 0;

  bool mSlowTime = false;
  unsigned int mCounter0=0;
  unsigned int mCounter1=0;

  nanogui::ref<PidSimFrontEnd> mFrontEnd;
};

#endif

