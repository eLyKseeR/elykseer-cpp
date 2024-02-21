cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0074 NEW)

IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING 
        "Choose the type of build, options are: 
        None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) 
        Debug Release RelWithDebInfo MinSizeRel." )
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING 
        "Choose the type of build, options are: 
        None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) 
        Debug Release RelWithDebInfo MinSizeRel." )
ENDIF()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

project(elykseer-cpp)

# The version number.
set (${PROJECT_NAME}_VERSION_MAJOR 2)
set (${PROJECT_NAME}_VERSION_MINOR 0)
set (${PROJECT_NAME}_VERSION_PATCH 0)
set (${PROJECT_NAME}_VERSION_STRING ${elykseer-cpp_VERSION_MAJOR}.${elykseer-cpp_VERSION_MINOR}.${elykseer-cpp_VERSION_PATCH})

# set to '-DDEBUG' to turn on debugging output
# set to '-UDEBUG' to disable debugging output
add_definitions( -UDEBUG )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

IF(${CMAKE_CROSSCOMPILING})
  message(STATUS "make sure dependencies have been cross compiled and are available.")
ELSE()
  # boost
  #add_definitions( -DBOOST_ALL_DYN_LINK )
  set(Boost_DEBUG OFF)
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_STATIC_RUNTIME OFF)
  set(Boost_USE_MULTITHREADED ON)
  find_package( Boost 1.83.0 REQUIRED COMPONENTS system date_time contract )
  LINK_DIRECTORIES ( ${Boost_LIBRARY_DIRS} )
  INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS})

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

  find_package(ZLIB REQUIRED)
  LINK_DIRECTORIES (${ZLIB_LIBRARY_DIRS})
  INCLUDE_DIRECTORIES (${ZLIB_INCLUDE_DIRS})
ENDIF(${CMAKE_CROSSCOMPILING})

set(PROCESSOR_TYPE ${CMAKE_SYSTEM_PROCESSOR})
set(SELECTED_AR ${CMAKE_AR})
set(LIB_PREFIX "")

# OS dependent
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.6")
  set(SELECTED_AR "/opt/homebrew/opt/binutils/bin/ar")
  if(CMAKE_C_COMPILER_ID MATCHES "Clang\$")
      set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_C_LIB_NAMES "omp")
      set(OpenMP_omp_LIBRARY omp)
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang\$")
      set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_CXX_LIB_NAMES "omp")
      set(OpenMP_omp_LIBRARY omp)
  endif()
  set(LIB_PREFIX "lib")
  set(libs "dl" ${ZLIB_LIBRARIES} "intl")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
          set(PROCESSOR_TYPE "x86_64")
  ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
  set(LIB_PREFIX "lib")
  set(libs ${ZLIB_LIBRARIES} "ws2_32")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set(SELECTED_AR "/usr/local/bin/ar")
  set(LIB_PREFIX "lib")
  set(libs "dl" ${ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LIB_PREFIX "lib")
  set(libs "dl" ${ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_CROSSCOMPILING})
    set(CROSS_PREFIX "x")
ELSE()
    set(CROSS_PREFIX "")
ENDIF()
set(EXT_PATH ${CROSS_PREFIX}${CMAKE_SYSTEM_NAME}_${PROCESSOR_TYPE})

find_program(CPPCHECK NAMES cppcheck)
if(CPPCHECK)
    message(STATUS "Found: cppcheck")
    add_custom_target(cppcheck
        ${CPPCHECK}
        -Uassert --std=c++20 --enable=all
        ${CMAKE_SOURCE_DIR}/src/lxr/*.hpp
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/test/*.cpp
        -I ${CMAKE_SOURCE_DIR}/../ext
        -I /opt/homebrew/include
    )
else()
    message(STATUS "Cppcheck - not found")
    message(STATUS "  Build target 'cppcheck' not available.")
endif()

IF(${CMAKE_CROSSCOMPILING})
  message(STATUS "OpenMP not available yet in cross compilation.")
ELSE()
  find_package(OpenMP REQUIRED)
  if(OPENMP_FOUND)
    add_definitions(-DOpenMP_available)
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      link_directories("/opt/homebrew/opt/libomp/lib")
      include_directories("/opt/homebrew/opt/libomp/include")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  endif(OPENMP_FOUND)
ENDIF(${CMAKE_CROSSCOMPILING})

# other external tools
include_directories(../ext)
include_directories(../ext/${EXT_PATH}/include)
link_directories("../ext/${EXT_PATH}/lib")

IF(${CMAKE_CROSSCOMPILING})
    set(HTML2TEXT ${CMAKE_HOME_DIRECTORY}/../ext/${CMAKE_HOST_SYSTEM_NAME}_${CMAKE_HOST_SYSTEM_PROCESSOR}/bin/html2text)
    message(STATUS "setting html2text to host version. " ${HTML2TEXT})
    set(ENV{HTML2TEXT} ${HTML2TEXT})
ELSE()
  find_program (HTML2TEXT NAMES html2text html2text.exe PATHS ../ext/${EXT_PATH}/bin NO_DEFAULT_PATH REQUIRED)
  IF(HTML2TEXT)
      message(STATUS "Found: html2text in " ${HTML2TEXT})
      set(ENV{HTML2TEXT} ${HTML2TEXT})
  ENDIF()
ENDIF()


add_subdirectory( src )

if(NOT CMAKE_CROSSCOMPILING)
enable_testing()
add_subdirectory( test )
endif()
