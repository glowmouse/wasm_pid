#ifndef __PIDSIM_BACKEND_H__
#define __PIDSIM_BACKEND_H__

#include "pidsim_frontend.h"
#include "pidsim_utils.h"
#include <queue>
#include <numeric>

class PidSimBackEndState
{
  public:

  void reset( double resetAngle ) {
    mAngle = resetAngle;
    mAngleVel = 0.0;
    mMaxNoiseInRadians = 0.0;
    setMotorDelay( 0.0 );
  }

  void bump( double bumpVel ) {
    mAngleVel += bumpVel;
  }

  void startSimulationIteration() {
    mAngleAccel = 0.0;
  }

  void endSimulationIteration() {
    double noise = static_cast<double>((rand() % 2000)-1000) / 1000.0 * mMaxNoiseInRadians;
    mPastAngles.push(mAngle + noise );
  }

  void setSensorDelay( double sensorDelayInMs )
  {
    const double sensorDelayInSeconds = sensorDelayInMs / 1000.0;
    mSensorDelayInUpdates = 1+static_cast<int>(sensorDelayInSeconds * 50.0);
  }

  void setMotorDelay( double motorDelayInMs )
  {
    const double motorDelayInSeconds = motorDelayInMs / 1000.0;
    mMotorDelayInUpdates = 1+static_cast<int>(motorDelayInSeconds * 50.0);
    const double currentMotorAvg = mMotorDelay.size() ?
      std::accumulate( mMotorDelay.begin(), mMotorDelay.end(), 0.0 ) / ((double) mMotorDelay.size()) :
      0.0;
    mMotorDelay.clear();
    for ( int i = 0; i < mMotorDelayInUpdates; ++i ) {
      mMotorDelay.push_back( currentMotorAvg );
    }
    mMotorDelayIndex = 0;
  }

  void setSensorNoise( double maxNoiseInDegrees ) {
    mMaxNoiseInRadians = degToRad(maxNoiseInDegrees);
  }

  void applyGravity( double timeSlice )
  {
    const double armX = cos( mAngle );
    const double armY = sin( mAngle );
    // Gravity = < 0   , -9.8 >
    // Arm     = < armX, armY >
    // Angular accelleration = Arm x Gravity
    mAngleAccel += armX * -9.8;
  }

  double getActualMotor()
  {
      return std::accumulate( mMotorDelay.begin(), mMotorDelay.end(), 0.0 ) / ((double) mMotorDelay.size());
  }

  void applyMotor( double motorPower, double timeSlice )
  {
    mMotorDelay.at( mMotorDelayIndex ) = motorPower;
    mMotorDelayIndex = ( mMotorDelayIndex + 1 ) % mMotorDelayInUpdates;

    mAngleAccel += getActualMotor() / timeSlice;
  }

  void applyFriction( double staticFriction, double rollingFriction, double timeSlice )
  {
    if ( mAngleVel > 0 ) {
      mAngleVel = std::max(mAngleVel - staticFriction * timeSlice, 0.0 );
    }
    else {
      mAngleVel = std::min(mAngleVel + staticFriction * timeSlice, 0.0 );
    }
    // TODO, include timeslice.
    mAngleVel *= 1.0 - rollingFriction;
  }

  void updateAngleVel( double timeSlice )
  {
    mAngleVel += mAngleAccel * timeSlice;
    mAngleAccel = 0.0;
  }

  void updateAngle( double timeSlice )
  {
    mAngle += mAngleVel * timeSlice;
  }

  void imposePositionHardLimits()
  {
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
  }

  double getSensorAngle() 
  {
    while ( mSensorDelayInUpdates < mPastAngles.size() )
    {
      mPastAngles.pop();
    }
    if ( 0 == mPastAngles.size() ) {
      return 0.0;
    }
    return mPastAngles.front();
  }

  double getAngle() 
  {
    return mAngle;
  }


  private:

  double mAngle = 0.0;
  double mAngleVel = 0.0;
  double mAngleAccel = 0.0;
  double mMaxNoiseInRadians = 0;
  int mSensorDelayInUpdates = 1;
  std::queue<double> mPastAngles;
  std::vector<double> mMotorDelay;
  int mMotorDelayInUpdates = 1;
  int mMotorDelayIndex = 0;
};

class PidSimBackEnd
{
  public:

  PidSimBackEnd( nanogui::ref<PidSimFrontEnd> frontEnd );

  void update( std::chrono::duration<double> delta );
 
  private:

  void softReset();
  void reset();
  void updateOneTick();
  void updateFrontEnd();

  private:

  static constexpr int updatesPerSecond = 50;

  PidSimBackEndState mArmState;

  double time = 0.0f;
  // map mAngle to "screen space" with <x,y> = <cos(mAngle),sin(mAngle)>

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

