#ifndef __PIDSIM_BACKEND_STATE_H__
#define __PIDSIM_BACKEND_STATE_H__

#include <queue>
#include <vector>

class PidSimBackEndState
{
  public:

  PidSimBackEndState(double startAngle); 

  // Delete default operators that don't want to expose.
  PidSimBackEndState() = delete;
  PidSimBackEndState( const PidSimBackEndState& other ) = delete;
  PidSimBackEndState& operator=( const PidSimBackEndState& other ) = delete;

  void reset( double resetAngle );
  void bump( double bumpVel );
  void startSimulationIteration();
  void endSimulationIteration();
  void setSensorDelay( double sensorDelayInMs );
  void setMotorDelay( double motorDelayInMs );
  void setSensorNoise( double maxNoiseInDegrees );

  void applyGravity( double timeSlice );
  double getActualMotor();
  void applyMotor( double motorPower, double timeSlice );
  void applyFriction( double staticFriction, double rollingFriction, double timeSlice );
  void updateAngleVel( double timeSlice );
  void updateAngle( double timeSlice );
  void imposePositionHardLimits();
  double getSensorAngle(); 
  double getAngle();

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

#endif

