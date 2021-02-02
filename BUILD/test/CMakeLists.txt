cmake_minimum_required(VERSION 3.5)

IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release DebugProfile RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release DebugProfile RelWithDebInfo MinSizeRel.")
ENDIF()

# boost
set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.67.0 REQUIRED COMPONENTS system thread date_time chrono unit_test_framework contract)
include_directories(${Boost_INCLUDE_DIRS})

# mine
include_directories(../src)
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/prngsharp/src")
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/elykseer-crypto/BUILD/src")
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/rapidcheck/include")
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/rapidcheck/extras/boost_test/include")

# link external libraries
link_directories("${CMAKE_HOME_DIRECTORY}/../ext/prngsharp")
link_directories("${CMAKE_HOME_DIRECTORY}/../ext/elykseer-crypto/BUILD/src")
link_directories("${CMAKE_HOME_DIRECTORY}/../ext/rapidcheck/BUILD/")
set(libs "dl" "z" "rapidcheck")

set(GENERATED_SRCS
  utAssembly.cpp
  utBackupCtrl.cpp
  utDbBackupJob.cpp
  utDbFp.cpp
  utDbKey.cpp
  utFileCtrl.cpp
  utFsUtils.cpp
  utLiz.cpp
  utMain.cpp
  utOptions.cpp
  utOS.cpp
  utRestoreCtrl.cpp)
set_source_files_properties(${GENERATED_SRCS} GENERATED)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")

        add_executable ( ut${PROJECT_NAME}
          ${GENERATED_SRCS}
        )

        target_link_libraries ( ut${PROJECT_NAME}
          elykseer-cpp_${CMAKE_BUILD_TYPE}
          ${libs}
          ${Boost_LIBRARIES}
        )
endif()

if (CMAKE_BUILD_TYPE STREQUAL "DebugProfile")
        add_executable ( utp${PROJECT_NAME}
          ${GENERATED_SRCS}
        )
        set_target_properties(utp${PROJECT_NAME} PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage ")
        target_link_libraries ( utp${PROJECT_NAME}
                elykseer-cpp_p${CMAKE_BUILD_TYPE}
                #gcov
                ${libs}
                ${Boost_LIBRARIES}
        )
endif()

# generate sources from literate c++ files
set(EXTRACT_CPP_CMD "../../ext/gitalk/utils/extract_cpp.sh")

set(LITERATE_SRC_PATH "../../tests/cpp")

add_custom_command(
  COMMAND rm -f utAssembly.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utAssembly.cpp.md"  utAssembly.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utAssembly.cpp.md"
  OUTPUT utAssembly.cpp)
add_custom_command(
  COMMAND rm -f utBackupCtrl.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utBackupCtrl.cpp.md"  utBackupCtrl.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utBackupCtrl.cpp.md"
  OUTPUT utBackupCtrl.cpp)
add_custom_command(
  COMMAND rm -f utDbBackupJob.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utDbBackupJob.cpp.md"  utDbBackupJob.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utDbBackupJob.cpp.md"
  OUTPUT utDbBackupJob.cpp)
add_custom_command(
  COMMAND rm -f utDbFp.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utDbFp.cpp.md"  utDbFp.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utDbFp.cpp.md"
  OUTPUT utDbFp.cpp)
add_custom_command(
  COMMAND rm -f utDbKey.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utDbKey.cpp.md"  utDbKey.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utDbKey.cpp.md"
  OUTPUT utDbKey.cpp)
add_custom_command(
  COMMAND rm -f utFileCtrl.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utFileCtrl.cpp.md"  utFileCtrl.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utFileCtrl.cpp.md"
  OUTPUT utFileCtrl.cpp)
add_custom_command(
  COMMAND rm -f utFsUtils.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utFsUtils.cpp.md"  utFsUtils.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utFsUtils.cpp.md"
  OUTPUT utFsUtils.cpp)
add_custom_command(
  COMMAND rm -f utLiz.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utLiz.cpp.md"  utLiz.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utLiz.cpp.md"
  OUTPUT utLiz.cpp)
add_custom_command(
  COMMAND rm -f utMain.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utMain.cpp.md"  utMain.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utMain.cpp.md"
  OUTPUT utMain.cpp)
add_custom_command(
  COMMAND rm -f utOptions.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utOptions.cpp.md"  utOptions.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utOptions.cpp.md"
  OUTPUT utOptions.cpp)
add_custom_command(
  COMMAND rm -f utOS.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utOS.cpp.md"  utOS.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utOS.cpp.md"
  OUTPUT utOS.cpp
  )
add_custom_command(
  COMMAND rm -f utRestoreCtrl.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utRestoreCtrl.cpp.md"  utRestoreCtrl.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utRestoreCtrl.cpp.md"
  OUTPUT utRestoreCtrl.cpp
  )


