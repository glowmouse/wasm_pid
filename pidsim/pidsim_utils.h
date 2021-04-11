#ifndef __PIDSIM_UTILS__
#define __PIDSIM_UTILS__

#include <math.h>
#include <queue>
#include <vector>
#include <assert.h>

namespace PidSim {
namespace Utils {

//
// Some helper/ utility functions
//

///
/// @brief Convert Degrees to Radians
/// 
/// @param[in] degrees - An angle in degrees
/// @return            - Same angle in radians
///
inline double degToRad( double degrees ) 
{
  return degrees / 180.0 * M_PI; 
}

///
/// @brief Convert Radians to Degrees
/// 
/// @param[in] radians - An angle in radians
/// @return            - Same angle in degrees
///
inline double radToDeg( double radians ) 
{
  return radians / M_PI * 180.0;
}

/// @brief a simple Moving Average class
///
/// @param[in] T  = the type for the moving average (i.e., double, int)
///
template< typename T >
class MovingAverage
{
  // Use an std::vector for internal storage.
  using InternalStorage = std::vector<T>;

  public:
  // Expose some standard types
  using value_type = T;
  using size_type = typename InternalStorage::size_type; 

  /// @brief Constructor
  ///
  /// @param[in] size       The number of entries in the average
  /// @param[in] initValue  The initial average
  ///  
  MovingAverage( size_type size, const T& initValue ) :
    mStorage    ( size, initValue ),
    mNumEntries { static_cast<T>(size) },
    mCurrentSum { mNumEntries * initValue }
  {
    assert( size > 0 );   // Moving average of 0 makes no sense.
  }

  // Remove constructors I probably never want (i.e., if used they're a bug)
  MovingAverage() = delete;

  /// 
  /// @brief Get the current moving average
  ///
  /// @return The average
  ///
  [[nodiscard]] T getAverage() const 
  {
    return mCurrentSum / mNumEntries; 
  }

  ///
  /// @brief Size of moving average
  ///
  /// @return The number of entries we're averaging together
  /// 
  [[nodiscard]] size_type size() const 
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

template< typename T>
class Delayer
{
  // The Delayer is basically a wrapper for a std::queue.
  using InternalStorage = std::queue<T>;

  public:

  // Expose some standard types
  using value_type = T;
  using size_type = typename InternalStorage::size_type; 

  /// @brief Constructor
  ///
  /// @param[in] sensorDelay - Number of simulation ticks we want
  ///     to delay the sensor output.
  /// 
  Delayer( size_type sensorDelay ) : 
    mSensorDelay{ sensorDelay }
  {
  }

  // Remove constructors I probably never want (i.e., if used they're a bug)
  Delayer() = delete;
  
  ///
  /// @brief Remove a value from the delay queue.
  ///
  /// 1. Pop values off the fron of the queue until the size is right
  /// 2. If the queue is empty, return 0.  Otherwise return the front value
  ///
  [[nodiscard]] T pop() 
  {
    // 1. Pop values off the fron of the queue until the size is right
    while ( mStorage.size() > mSensorDelay ) { mStorage.pop(); }
    // 2. If the queue is empty, return 0.  Otherwise return the front value
    const bool isEmpty = ( 0 == mStorage.size() );
    return isEmpty ? static_cast<T>(0) : mStorage.front(); 
  }

  ///
  /// @brief Add avalue to the delay queue
  /// 
  void push( T val )
  {
    mStorage.push( std::move( val ));
  }

  ///
  /// @brief  Get the number of sumulation ticks we're delaying the sensor by
  ///
  /// @return The number of simulation ticks we're delaying the sensor by...
  ///
  [[nodiscard]] size_type size() const {
    return mSensorDelay;
  }

  private:

  // How many ticks to delay
  size_type         mSensorDelay;
  // Internal storage.  Really an std::queue
  InternalStorage   mStorage;
}; 

}
}

#endif

