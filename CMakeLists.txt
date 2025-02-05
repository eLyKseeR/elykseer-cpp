cmake_minimum_required(VERSION 3.30)

cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0167 NEW)

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

project(elykseer-cpp LANGUAGES CXX)

# The version number.
set (${PROJECT_NAME}_VERSION_MAJOR 4)
set (${PROJECT_NAME}_VERSION_MINOR 0)
set (${PROJECT_NAME}_VERSION_PATCH 1)
set (${PROJECT_NAME}_VERSION_STRING ${elykseer-cpp_VERSION_MAJOR}.${elykseer-cpp_VERSION_MINOR}.${elykseer-cpp_VERSION_PATCH})

# set to '-DDEBUG' to turn on debugging output
# set to '-UDEBUG' to disable debugging output
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions( -DDEBUG )
  add_compile_options(-glldb)
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
  set(LLDB_EXPORT_ALL_SYMBOLS 1)
ELSE()
  add_definitions( -UDEBUG )
ENDIF()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#    link_libraries(-static-libgcc -static-libstdc++)
#ENDIF()

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
  IF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    # only check preconditions and class invariants (on entry)
    ##add_definitions(-DBOOST_CONTRACT_NO_PRECONDITIONS)
    add_definitions(-DBOOST_CONTRACT_NO_POSTCONDITIONS)
    add_definitions(-DBOOST_CONTRACT_NO_EXCEPTS)
    ##add_definitions(-DBOOST_CONTRACT_NO_ENTRY_INVARIANTS)
    add_definitions(-DBOOST_CONTRACT_NO_EXIT_INVARIANTS)
    add_definitions(-DBOOST_CONTRACT_NO_CHECKS)
  ENDIF()

  find_package(ZLIB REQUIRED)
  LINK_DIRECTORIES (${ZLIB_LIBRARY_DIRS})
  INCLUDE_DIRECTORIES (${ZLIB_INCLUDE_DIRS})
ENDIF(${CMAKE_CROSSCOMPILING})

set(PLATFORM "unk")
set(PROCESSOR_TYPE ${CMAKE_SYSTEM_PROCESSOR})
set(SELECTED_AR ${CMAKE_AR})
set(LIB_PREFIX "")

# OS dependent
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(PLATFORM "darwin")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "15.1")
  set(SELECTED_AR "/opt/homebrew/opt/binutils/bin/ar")
  set(LIB_PREFIX "lib")
  set(libs "dl" ${ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(PLATFORM "win64")
  IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
          set(PROCESSOR_TYPE "x86_64")
  ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
  set(LIB_PREFIX "lib")
  set(libs ${ZLIB_LIBRARIES} "ws2_32")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set(PLATFORM "freebsd")
  set(SELECTED_AR "/usr/local/bin/ar")
  set(LIB_PREFIX "lib")
  set(libs "dl" ${ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(PLATFORM "linux")
  set(LIB_PREFIX "lib")
  set(libs "dl" ${ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_definitions( -DPLATFORM_${PLATFORM} )

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
  # find_package(OpenMP REQUIRED)
  # if(OPENMP_FOUND)
  #   # add_definitions(-DOpenMP_available)
  #   IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  #     link_directories("/opt/homebrew/opt/libomp/lib")
  #     include_directories("/opt/homebrew/opt/libomp/include")
  #   ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # endif(OPENMP_FOUND)
ENDIF(${CMAKE_CROSSCOMPILING})

# other external tools
include_directories(ext)
include_directories(ext/${EXT_PATH}/include)
link_directories("ext/${EXT_PATH}/lib")


# link external library elykseer-crypto
file(GLOB MODULE_INTERFACE_FILES "${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/include/lxr/*.ixx")

set(LIB_ELYKSEER_CRYPTO "libelykseer-crypto.a")
# IF(CMAKE_BUILD_TYPE STREQUAL "Release")
#   set(LIB_ELYKSEER_CRYPTO "libelykseer-crypto.a")
# ELSE()
#   set(LIB_ELYKSEER_CRYPTO "libelykseer-crypto_d.a")
# ENDIF()

add_library(elykseer-crypto_s STATIC IMPORTED)
set_target_properties(elykseer-crypto_s PROPERTIES
  IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/lib/${LIB_ELYKSEER_CRYPTO}"
)
target_sources(elykseer-crypto_s
    INTERFACE
      FILE_SET HEADERS
      BASE_DIRS "${CMAKE_HOME_DIRECTORY}/ext"
      FILES
      ${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/include/lxr/lxr-cbindings.hpp
      ${CMAKE_HOME_DIRECTORY}/ext/sizebounded/sizebounded.hpp
      FILE_SET CXX_MODULES
      BASE_DIRS "${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/include/lxr/"
      FILES
      ${MODULE_INTERFACE_FILES}
)
target_include_directories(elykseer-crypto_s INTERFACE "${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/include/lxr/")
target_compile_features(elykseer-crypto_s INTERFACE cxx_std_20)


add_subdirectory( src/cpp )


if(NOT CMAKE_CROSSCOMPILING)
enable_testing()
add_subdirectory( tests/cpp )
endif()
