#include <gtest/gtest.h>
#include "../pidsim/pidsim_utils.h"

//
// A simple test for moving averages.
// 
TEST( UTILS, MovingAverage_class_works )
{
  PidSim::Utils::MovingAverage<double> test(4, 1.0 );

  ASSERT_DOUBLE_EQ( 1.0, test.getAverage() );
  test.newValue( 2.0 );
  ASSERT_DOUBLE_EQ( 1.25, test.getAverage() );
  test.newValue( 3.0 );
  ASSERT_DOUBLE_EQ( 1.75, test.getAverage() );
  test.newValue( 2.0 );
  ASSERT_DOUBLE_EQ( 2.0 , test.getAverage() );
  test.newValue( 4.0 );
  ASSERT_DOUBLE_EQ( 2.75, test.getAverage() );
  test.newValue( 4.0 );
  ASSERT_DOUBLE_EQ( 3.25, test.getAverage() );
  test.newValue( 4.0 );
  ASSERT_DOUBLE_EQ( 3.5, test.getAverage() );
  test.newValue( 4.0 );
  ASSERT_DOUBLE_EQ( 4.0, test.getAverage() );
  test.newValue( 4.0 );
  ASSERT_DOUBLE_EQ( 4.0, test.getAverage() );
}

TEST( UTILS, DelayerClass_works )
{
  PidSim::Utils::Delayer<int> delayer( 4 );

  // Time=1, T[0]=10.
  delayer.push( 10 );
  // Desired Time is T[1-4], T[0] is the best we have
  ASSERT_EQ( 10, delayer.pop() );
  // Time=2, T[0]=10 T[1]=11.
  delayer.push( 11 );
  // Desired Time is T[2-4], T[0] is the best we have
  ASSERT_EQ( 10, delayer.pop() );
  // Time = 3, T[0]=10 T[1]=11 T[2]=12.
  delayer.push( 12 );
  // Desired Time is T[3-4], T[0] is the best we have
  ASSERT_EQ( 10, delayer.pop() );
  // Time = 4, T[0]=10 T[1]=11 T[2]=12 T[3]=13
  delayer.push( 13 );
  // Desired Time is T[4-4], T[0] is what we want
  ASSERT_EQ( 10, delayer.pop() );
  // Time = 5, T[1]=11 T[2]=12 T[3]=13 T[4]=14
  delayer.push( 14 );
  // Desired Time is T[5-4], Return T[1]
  ASSERT_EQ( 11, delayer.pop() );
  // Time = 6, T[2]=12 T[3]=13 T[4]=14 T[5]=15
  delayer.push( 15 );
  // Desired Time is T[6-4], Return T[2]
  ASSERT_EQ( 12, delayer.pop() );
  // Calling pop over and over doesn't change the time
  ASSERT_EQ( 12, delayer.pop() );
  ASSERT_EQ( 12, delayer.pop() );
  ASSERT_EQ( 12, delayer.pop() );
  // Time = 7, T[3]=13 T[4]=14 T[5]=15 T[6]=16
  delayer.push( 16 );
  // Time = 8, T[4]=14 T[5]=15 T[6]=16 T[7]=17
  delayer.push( 17 );
  // Time = 9, T[5]=15 T[6]=16 T[7]=17 T[8]=18
  delayer.push( 18 ); 
  // Desired Time is T[9-4], Return T[5]
  ASSERT_EQ( 15, delayer.pop() );
} 

