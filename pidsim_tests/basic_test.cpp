#include <gtest/gtest.h>
#include "../pidsim/pidsim_utils.h"

//
// A simple test for moving averages.
// 
TEST( UTILS, moving_average )
{
  PidSim::Utils::MovingAverage<int> test(3, 1 );

  ASSERT_EQ( 1, test.getAverage() );
  test.newValue( 2 );
  ASSERT_EQ( 1, test.getAverage() );
  test.newValue( 3 );
  ASSERT_EQ( 2, test.getAverage() );
  test.newValue( 2 );
  ASSERT_EQ( 2, test.getAverage() );
  test.newValue( 4 );
  ASSERT_EQ( 3, test.getAverage() );
  test.newValue( 4 );
  ASSERT_EQ( 3, test.getAverage() );
  test.newValue( 4 );
  ASSERT_EQ( 4, test.getAverage() );

}

