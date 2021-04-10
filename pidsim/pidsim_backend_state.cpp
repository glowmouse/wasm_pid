#include <queue>
#include <numeric>
#include "pidsim_utils.h"
#include "pidsim_backend_state.h"


PidSimBackEndState::PidSimBackEndState( double startAngle )
  : mAngle{ startAngle },
    mMotorDelay{ 1, 0.0 }
{
}

void PidSimBackEndState::bump( double bumpVel ) {
  mAngleVel += bumpVel;
}

void PidSimBackEndState::startSimulationIteration() {
  mAngleAccel = 0.0;
}

void PidSimBackEndState::endSimulationIteration() {
  double noise = static_cast<double>((rand() % 2000)-1000) / 1000.0 * mMaxNoiseInRadians;
  mPastAngles.push(mAngle + noise );
}

void PidSimBackEndState::setSensorDelay( double sensorDelayInMs )
{
  const double sensorDelayInSeconds = sensorDelayInMs / 1000.0;
  mSensorDelayInUpdates = 1+static_cast<int>(sensorDelayInSeconds * 50.0);
}

void PidSimBackEndState::setSensorNoise( double maxNoiseInDegrees ) {
  mMaxNoiseInRadians = degToRad(maxNoiseInDegrees);
}

void PidSimBackEndState::applyGravity( double timeSlice )
{
  const double armX = cos( mAngle );
  const double armY = sin( mAngle );
  // Gravity = < 0   , -9.8 >
  // Arm     = < armX, armY >
  // Angular accelleration = Arm x Gravity
  mAngleAccel += armX * -9.8;
}

void PidSimBackEndState::setMotorDelay( double MotorDelayMS )
{
  using size_type = decltype(mMotorDelay)::size_type;
  size_type newNumDelays = 1+static_cast<size_type>(MotorDelayMS / 1000.0 * 50.0);
  if ( newNumDelays != mMotorDelay.size() ) {
    const double currentAverage = mMotorDelay.getAverage();
    mMotorDelay = PidSim::Util::MovingAverage<double>( newNumDelays, currentAverage );
  }
}

double PidSimBackEndState::getActualMotor()
{
  return mMotorDelay.getAverage();
}

void PidSimBackEndState::applyMotor( double motorPower, double timeSlice )
{
  mMotorDelay.newValue( motorPower );
  mAngleAccel += getActualMotor() / timeSlice;
}

void PidSimBackEndState::applyFriction( double staticFriction, double rollingFriction, double timeSlice )
{
  if ( mAngleVel > 0 ) {
    mAngleVel = std::max(mAngleVel - staticFriction * timeSlice, 0.0 );
  }
  else {
    mAngleVel = std::min(mAngleVel + staticFriction * timeSlice, 0.0 );
  }
  mAngleVel *= 1.0 - rollingFriction;
}

void PidSimBackEndState::updateAngleVel( double timeSlice )
{
  mAngleVel += mAngleAccel * timeSlice;
  mAngleAccel = 0.0;
}

void PidSimBackEndState::updateAngle( double timeSlice )
{
  mAngle += mAngleVel * timeSlice;
}

void PidSimBackEndState::imposePositionHardLimits()
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

double PidSimBackEndState::getSensorAngle() 
{
  while ( mSensorDelayInUpdates < mPastAngles.size() )
  {
    mPastAngles.pop();
  }
  return (0 == mPastAngles.size()) ? 0.0 : mPastAngles.front();
}

double PidSimBackEndState::getAngle() 
{
  return mAngle;
}

