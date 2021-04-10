#include <queue>
#include <numeric>
#include "pidsim_utils.h"
#include "pidsim_backend_state.h"

namespace  PidSim {

BackEndState::BackEndState( double startAngle )
  : mAngle{ startAngle },
    mMotorDelay{ 1, 0.0 },
    mSensorDelay{ 1 }
{
}

void BackEndState::bump( double bumpVel ) {
  mAngleVel += bumpVel;
}

void BackEndState::startSimulationIteration() {
  mAngleAccel = 0.0;
}

void BackEndState::endSimulationIteration() {
  double noise = static_cast<double>((rand() % 2000)-1000) / 1000.0 * mMaxNoiseInRadians;
  mSensorDelay.push( mAngle + noise );
}

void BackEndState::setSensorDelay( double sensorDelayInMs )
{
  using size_type = decltype( mSensorDelay )::size_type; 

  const double sensorDelayInSeconds = sensorDelayInMs / 1000.0;
  const size_type delayInUpdates = 1+static_cast<size_type>(sensorDelayInSeconds * 50.0);
  if ( delayInUpdates != mSensorDelay.size() ) {
    mSensorDelay = Utils::Delayer<double>{ delayInUpdates };
  }
}

void BackEndState::setSensorNoise( double maxNoiseInDegrees ) {
  mMaxNoiseInRadians = Utils::degToRad(maxNoiseInDegrees);
}

void BackEndState::applyGravity( double timeSlice )
{
  const double armX = cos( mAngle );
  const double armY = sin( mAngle );
  // Gravity = < 0   , -9.8 >
  // Arm     = < armX, armY >
  // Angular accelleration = Arm x Gravity
  mAngleAccel += armX * -9.8;
}

void BackEndState::setMotorDelay( double MotorDelayMS )
{
  using size_type = decltype(mMotorDelay)::size_type;
  size_type newNumDelays = 1+static_cast<size_type>(MotorDelayMS / 1000.0 * 50.0);
  if ( newNumDelays != mMotorDelay.size() ) {
    const double currentAverage = mMotorDelay.getAverage();
    mMotorDelay = Utils::MovingAverage<double>( newNumDelays, currentAverage );
  }
}

double BackEndState::getMotorPower()
{
  return mMotorDelay.getAverage();
}

void BackEndState::applyMotor( double motorPower, double timeSlice )
{
  mMotorDelay.newValue( motorPower );
  mAngleAccel += getMotorPower() / timeSlice;
}

void BackEndState::applyFriction( double staticFriction, double rollingFriction, double timeSlice )
{
  if ( mAngleVel > 0 ) {
    mAngleVel = std::max(mAngleVel - staticFriction * timeSlice, 0.0 );
  }
  else {
    mAngleVel = std::min(mAngleVel + staticFriction * timeSlice, 0.0 );
  }
  mAngleVel *= 1.0 - rollingFriction;
}

void BackEndState::updateAngleVel( double timeSlice )
{
  mAngleVel += mAngleAccel * timeSlice;
  mAngleAccel = 0.0;
}

void BackEndState::updateAngle( double timeSlice )
{
  mAngle += mAngleVel * timeSlice;
}

void BackEndState::imposePositionHardLimits()
{
  //
  // Impose some hard limits.  -120 degrees is about the angle where
  // the final segment of the robot arm hits the second segment,
  // visually.
  //
  if ( mAngle < Utils::degToRad( -120 )) {
    mAngle    = Utils::degToRad( -120 );
    mAngleVel = 0.0f;  // A hard stop kills all velocity
  }

  //
  // Second limit, where the final segment hits the second segment, but
  // in the other direction.
  //
  if ( mAngle > Utils::degToRad( 210 )) {
    mAngle    = Utils::degToRad( 210 );
    mAngleVel = 0.0f;  // Again, a hard stop kills all velocity
  }
}

double BackEndState::getSensorAngle() 
{
  return mSensorDelay.pop();
}

double BackEndState::getActualAngle() 
{
  return mAngle;
}

}


