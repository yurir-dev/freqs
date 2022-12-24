cmake_minimum_required(VERSION 3.10)


include_directories(../.)
include_directories(./.)

#include_directories(${CMAKE_SOURCE_DIR} . ../ )

# Files common to all tests
set (COMMON_SOURCES )

set(TEST_BASIC test_basic)
add_executable(${TEST_BASIC} test_cmake_template.cpp ${COMMON_SOURCES})

set(exes ${TEST_BASIC})

if (UNIX)
foreach (exe IN LISTS exes)
	target_link_libraries(${exe} pthread)
endforeach()
endif()

foreach (exe IN LISTS exes)
	add_test(${exe} ${exe})
endforeach()
