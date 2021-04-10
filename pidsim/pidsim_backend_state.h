#ifndef __PIDSIM_BACKEND_STATE_H__
#define __PIDSIM_BACKEND_STATE_H__

#include <queue>
#include <vector>
#include <assert.h>
#include <iostream>
#include "pidsim_utils.h"

namespace PidSim {

class BackEndState
{
  public:

  /// @brief Constructor
  ///
  /// startAngle - The angle the Arm starts at
  ///
  BackEndState(double startAngle); 

  // Delete default operators that don't want to expose.
  BackEndState() = delete;
  BackEndState( const BackEndState& other ) = delete;
  BackEndState& operator=( const BackEndState& other ) = delete;

  ///
  /// Setters
  /// 

  /// @brief Add a suddent velocity to the arm.
  /// 
  /// @param[in] bumpVel - Velocity to add in radians/s 
  /// 
  void bump( double bumpVel );

  /// @brief Set a new Sensor Delay time window
  ///
  /// @param[in] sensorDelayInMs - The time the simulation waits
  ///     before letting the PID controller see the current sensor
  ///     value.
  ///
  void setSensorDelay( double sensorDelayInMs );

  /// @brief Set the simulated sensor noise
  /// 
  /// @param[in] maxNoiseInDegrees - The amount of noise to add to
  ///     sensor readings. The simulation will add a random value
  ///     between -maxNoiseInDegrees and +maxNoiseInDegrees.
  ///
  void setSensorNoise( double maxNoiseInDegrees );

  ///
  /// @brief Set the moving average time for motor output
  ///
  /// @param[in] motorDelayInMs - Sets the moving average window for
  ///     motor output.  All raw motor entries within this window
  ///     are averaged together.
  ///
  void setMotorDelay( double motorDelayInMs );

  ///
  /// @brief Get the sensor of angle (which may be delayed)
  /// 
  /// @return The sensor angle in Radians.
  /// 
  double getSensorAngle();
  /// 
  /// @brief Get the actual angle the arm is pointed at
  ///
  /// @return The angle the arm is pointed at, in Radians.
  /// 
  double getActualAngle();

  ///
  /// @brief The the simulated Motor Output
  ///
  double getMotorPower();

  void startSimulationIteration();
  void applyGravity( double timeSlice );
  void applyMotor( double motorPower, double timeSlice );
  void applyFriction( double staticFriction, double rollingFriction, double timeSlice );
  void updateAngleVel( double timeSlice );
  void updateAngle( double timeSlice );
  void imposePositionHardLimits();
  void endSimulationIteration();

  private:

  double mAngle = 0.0;
  double mAngleVel = 0.0;
  double mAngleAccel = 0.0;
  double mMaxNoiseInRadians = 0.0;

  Utils::MovingAverage<double> mMotorDelay;
  Utils::Delayer<double> mSensorDelay;
};

}

#endif

