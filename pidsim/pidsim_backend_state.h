#ifndef __PIDSIM_BACKEND_STATE_H__
#define __PIDSIM_BACKEND_STATE_H__

#include <queue>
#include <vector>
#include <assert.h>
#include <iostream>

namespace PidSim {
namespace Utils {
 
/// @brief a simple Moving Average class
///
/// @param[in] T  = the type for the moving average (i.e., double, int)
///
template< typename T >
class MovingAverage
{
  using InternalStorage = std::vector<T>;

  public:
  using value_type = T;
  using size_type = typename InternalStorage::size_type; 

  /// @brief Constructor
  ///
  /// @param[in] size       The number of entries in the average
  /// @param[in] initValue  The initial average
  ///  
  MovingAverage( size_type size, const T& initValue ) :
    mStorage( size, initValue ),
    mNumEntries{ static_cast<T>(size) },
    mCurrentSum { mNumEntries * initValue }
  {
    assert( size > 0 );   // Moving average of 0 makes no sense.
  }
  MovingAverage() = delete;

  /// 
  /// @brief Get the current moving average
  ///
  /// @return The average
  ///
  T getAverage() const 
  {
    return mCurrentSum / mNumEntries; 
  }

  ///
  /// @brief Size of moving average
  ///
  /// @return The number of entries we're averaging together
  /// 
  size_type size() const 
  {
    return mStorage.size();
  }

  ///
  /// @brief Add a new value to the moving average list
  ///
  /// @param[in]  value - the new value.
  ///
  /// Replaces the oldest value in the average with the new value.
  /// 
  void newValue( const T& value )
  {
    mCurrentSum -= mStorage.at( mCurrentIndex );
    mStorage.at( mCurrentIndex ) = value;
    mCurrentSum += mStorage.at( mCurrentIndex );
    mCurrentIndex = ( mCurrentIndex + 1 ) % mStorage.size();
  }

  private:
  InternalStorage       mStorage;
  size_type             mCurrentIndex{0};
  T                     mNumEntries;
  T                     mCurrentSum;
};
}
}

namespace PidSim {

class BackEndState
{
  public:

  BackEndState(double startAngle); 

  // Delete default operators that don't want to expose.
  BackEndState() = delete;
  BackEndState( const BackEndState& other ) = delete;
  BackEndState& operator=( const BackEndState& other ) = delete;

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
  double mMaxNoiseInRadians = 0.0;
  int mSensorDelayInUpdates = 1;
  std::queue<double> mPastAngles;

  Utils::MovingAverage<double> mMotorDelay;
};

}

#endif

