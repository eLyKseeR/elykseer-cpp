cmake_minimum_required(VERSION 3.25)

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
find_package( Boost 1.83.0 REQUIRED COMPONENTS system thread date_time chrono unit_test_framework )

# mine
include_directories(../src)
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/rapidcheck/extras/boost_test/include")

# link external libraries
set(testlibs "rapidcheck")

set(GENERATED_SRCS
  utFileCtrl.cpp
  utFsUtils.cpp
  utCoqAssembly.cpp
  utCoqAssemblyCache.cpp
  utCoqEnvironment.cpp
  utLiz.cpp
  utMain.cpp
  utOS.cpp)
set_source_files_properties(${GENERATED_SRCS} GENERATED)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable ( ut${PROJECT_NAME}
          ${GENERATED_SRCS}
        )
        add_test(NAME ut${PROJECT_NAME} COMMAND ut${PROJECT_NAME} --show_progress)

        if(OpenMP_CXX_FOUND)
        target_link_libraries(ut${PROJECT_NAME}
          ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}
          OpenMP::OpenMP_CXX
          ${libs} ${testlibs}
          ${Boost_LIBRARIES}
        )
        else()
        target_link_libraries(ut${PROJECT_NAME}
          ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}
          ${libs} ${testlibs}
          ${Boost_LIBRARIES}
        )
        endif()    
endif()

if (CMAKE_BUILD_TYPE STREQUAL "DebugProfile")
        add_executable ( utp${PROJECT_NAME}
          ${GENERATED_SRCS}
        )
        set_target_properties(utp${PROJECT_NAME} PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage ")
        target_link_libraries ( utp${PROJECT_NAME}
                ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}
                #gcov
                ${libs} ${testlibs}
                ${Boost_LIBRARIES}
        )
endif()

# generate sources from literate c++ files
set(EXTRACT_CPP_CMD "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/gitalk/utils/extract_cpp.sh")

set(LITERATE_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/cpp")

add_custom_command(
  COMMAND rm -f utCoqAssembly.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utCoqAssembly.cpp.md"  utCoqAssembly.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utCoqAssembly.cpp.md"
  OUTPUT utCoqAssembly.cpp)
add_custom_command(
  COMMAND rm -f utCoqAssemblyCache.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utCoqAssemblyCache.cpp.md"  utCoqAssemblyCache.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utCoqAssemblyCache.cpp.md"
  OUTPUT utCoqAssemblyCache.cpp)
add_custom_command(
  COMMAND rm -f utCoqEnvironment.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utCoqEnvironment.cpp.md"  utCoqEnvironment.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utCoqEnvironment.cpp.md"
  OUTPUT utCoqEnvironment.cpp)
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
  COMMAND rm -f utOS.cpp
  COMMAND ${EXTRACT_CPP_CMD} "${LITERATE_SRC_PATH}/utOS.cpp.md"  utOS.cpp
  DEPENDS "${LITERATE_SRC_PATH}/utOS.cpp.md"
  OUTPUT utOS.cpp)
