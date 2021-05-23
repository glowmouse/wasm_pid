cmake_minimum_required(VERSION 3.5)

project( CXX C )

add_subdirectory( pidsim )

# Testing
find_package (GTest)

IF (GTEST_FOUND)
  add_definitions( -DGTEST_FOUND )
  MESSAGE (STATUS  "GTEST found, running unit tests")
  ADD_SUBDIRECTORY(pidsim_tests)
ELSE()
  MESSAGE (STATUS  "GTEST not found, skipping unit tests")
ENDIF (GTEST_FOUND)

