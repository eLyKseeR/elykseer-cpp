cmake_minimum_required(VERSION 3.5)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

IF(DEFINED CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release DebugProfile RelWithDebInfo MinSizeRel.")
ELSE()
	SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release DebugProfile RelWithDebInfo MinSizeRel.")
ENDIF()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

project(elykseer-cpp)

# The version number.
set (${PROJECT_NAME}_VERSION_MAJOR 1)
set (${PROJECT_NAME}_VERSION_MINOR 6)
set (${PROJECT_NAME}_VERSION_PATCH 2)
set (${PROJECT_NAME}_VERSION_STRING ${elykseer-cpp_VERSION_MAJOR}.${elykseer-cpp_VERSION_MINOR}.${elykseer-cpp_VERSION_PATCH})

# set to '-DDEBUG' to turn on debugging output
# set to '-UDEBUG' to disable debugging output
add_definitions( -UDEBUG )

# boost
add_definitions( -DBOOST_ALL_DYN_LINK )
set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.67.0 REQUIRED COMPONENTS system filesystem chrono thread date_time iostreams contract )
LINK_DIRECTORIES ( ${Boost_LIBRARY_DIRS} )
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS})

find_package(ZLIB REQUIRED) 
LINK_DIRECTORIES (${ZLIB_LIBRARY_DIRS})
INCLUDE_DIRECTORIES (${ZLIB_INCLUDE_DIRS})

# boost/contract
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  # only check preconditions and class invariants (on entry)
  ##add_definitions(-DBOOST_CONTRACT_NO_PRECONDITIONS)
  add_definitions(-DBOOST_CONTRACT_NO_POSTCONDITIONS)
  add_definitions(-DBOOST_CONTRACT_NO_EXCEPTS)
  ##add_definitions(-DBOOST_CONTRACT_NO_ENTRY_INVARIANTS)
  add_definitions(-DBOOST_CONTRACT_NO_EXIT_INVARIANTS)
  add_definitions(-DBOOST_CONTRACT_NO_CHECKS)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O")
endif (CMAKE_COMPILER_IS_GNUCC)

find_program(CPPCHECK NAMES cppcheck)
if(CPPCHECK)
    message(STATUS "Found: cppcheck")
    add_custom_target(cppcheck
        ${CPPCHECK}
        -Uassert --std=c++17 --enable=all
        ${CMAKE_SOURCE_DIR}/src/lxr/*.hpp
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/test/*.cpp
    )
else()
    message(STATUS "Cppcheck - not found")
    message(STATUS "  Build target 'cppcheck' not available.")
endif()

# OS dependent
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# other external tools
include_directories(../ext)


add_subdirectory( src )
add_subdirectory( test )
