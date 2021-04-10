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

template< typename T>
class Delayer
{
  using InternalStorage = std::queue<T>;

  public:

  using value_type = T;
  using size_type = typename InternalStorage::size_type; 

  Delayer() = delete;

  Delayer( size_type size ) : 
    mSensorDelay{ size }
  {
  }

  T pop() 
  {
    while ( mStorage.size() > mSensorDelay ) { mStorage.pop(); }
    const bool isEmpty = ( 0 == mStorage.size() );
    return isEmpty ? static_cast<T>(0) : mStorage.front(); 
  }

  void push( const T val )
  {
    mStorage.push( std::move( val ));
  }

  size_t size() const {
    return mSensorDelay;
  }

  private:

  T mDefaultResult;
  size_type mSensorDelay;
  InternalStorage mStorage;
}; 

}
}

#endif

