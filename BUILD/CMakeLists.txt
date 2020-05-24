cmake_minimum_required(VERSION 3.5)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

IF(DEFINED CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release DebugProfile RelWithDebInfo MinSizeRel.")
ELSE()
	SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release DebugProfile RelWithDebInfo MinSizeRel.")
ENDIF()

project(elykseer-cpp)

# The version number.
set (${PROJECT_NAME}_VERSION_MAJOR 1)
set (${PROJECT_NAME}_VERSION_MINOR 0)
set (${PROJECT_NAME}_VERSION_PATCH 42)
set (${PROJECT_NAME}_VERSION_STRING ${elykseer-cpp_VERSION_MAJOR}.${elykseer-cpp_VERSION_MINOR}.${elykseer-cpp_VERSION_PATCH})

add_definitions( -UDEBUG )

# boost
#set(BOOST_ROOT "${CMAKE_HOME_DIRECTORY}/ext/boost_1_69_0/")
#set(BOOST_LIBRARYDIR "${CMAKE_HOME_DIRECTORY}/ext/boost_1_69_0/stage/lib")
add_definitions( -DBOOST_ALL_DYN_LINK )
#set(Boost_ADDITIONAL_VERSIONS "1.49")
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

add_library(libpugixml STATIC IMPORTED)
SET_TARGET_PROPERTIES(libpugixml PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/../ext/pugixml/libpugixml.a)

#add_library(libelykseer-crypto STATIC IMPORTED)
#SET_TARGET_PROPERTIES(libelykseer-crypto PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/../ext/elykseer-crypto/BUILD/src/libelykseer-crypto_Debug_s.a)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -g -O1 -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -fPIC")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fPIC -g")
# clang also wants its own stdlib
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -g -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -O0 -fPIC")

# OS dependent
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12.0")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# other external tools
include_directories(../ext)


add_subdirectory( src )
add_subdirectory( test )

