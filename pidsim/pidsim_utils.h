#ifndef __PIDSIM_UTILS__
#define __PIDSIM_UTILS__

#include <math.h>

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

} // End Utils
} // End PidSim

#endif

