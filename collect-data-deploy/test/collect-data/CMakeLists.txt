cmake_minimum_required(VERSION 3.3)

project(ml)
message("PROJECT_NAME is ${PROJECT_NAME}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(SCIP_DIR /storage/home/hcoda1/0/sbayramoglu3/p-nsahinidis3-0/scipoptsuite-8.0.0/install/lib/cmake/scip)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(SCIP_DIR /storage/home/hcoda1/0/sbayramoglu3/p-nsahinidis3-0/scipoptsuite-8.0.0/install-dbg/lib/cmake/scip)
endif()

message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message("SCIP_DIR is ${SCIP_DIR}")
message("TARGET: ${TARGET}")

if(TARGET SCIP::SCIP)
  # find package by SCIP PATH
  find_package(SCIP CONFIG PATHS ${SCIP_BINARY_DIR} REQUIRED)
else()
  find_package(SCIP REQUIRED)
endif()

include_directories(${SCIP_INCLUDE_DIRS})

add_executable(data
   ../../../src/cmain.c
   ../../../src/cutsel_features.c
   ../../../src/helper_functions.c
   ../../../src/branching_stop.c
   ../../../src/solver_settings.c
   )

message("LIBM: ${LIBM}")
# link to math library if it is available
find_library(LIBM m)
message("LIBM: ${LIBM}")
if(NOT LIBM)
  set(LIBM "")
endif()

set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Wshadow -fstrict-aliasing -Wstrict-aliasing -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -fstrict-aliasing -Wstrict-aliasing -Wshadow -g")
set(CMAKE_VERBOSE_MAKEFILE)

add_definitions(-DCOLLECT_DATA_MODE)
add_definitions(-DTEST)

target_link_libraries(data ${SCIP_LIBRARIES} ${LIBM})
