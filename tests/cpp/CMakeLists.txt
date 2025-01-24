cmake_minimum_required(VERSION 3.30)

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
find_package( Boost 1.83.0 REQUIRED COMPONENTS unit_test_framework )

# mine
# include_directories(../src)
include_directories("${CMAKE_HOME_DIRECTORY}/../../ext/rapidcheck/extras/boost_test/include")

# link external libraries
set(testlibs "rapidcheck")

set(TEST_PRIVATE_SRCS
  utFileCtrl.cpp
  utFsUtils.cpp
  utCoqAssembly.cpp
  utCoqAssemblyCache.cpp
  utCoqEnvironment.cpp
  utLiz.cpp
  utMain.cpp
  utOS.cpp
)

set(TEST_SRCS "")


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable ( ut${PROJECT_NAME} )
        target_sources(ut${PROJECT_NAME}
            PRIVATE
              ${TEST_PRIVATE_SRCS}
            PUBLIC
            FILE_SET CXX_MODULES FILES
              ${TEST_SRCS}
        )
        add_test(NAME ut${PROJECT_NAME} COMMAND ut${PROJECT_NAME} --show_progress)

        target_link_libraries(ut${PROJECT_NAME}
          ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
          elykseer-crypto_s
          ${libs} ${testlibs}
          ${Boost_LIBRARIES}
        )
        if(OpenMP_CXX_FOUND)
            target_link_libraries(ut${PROJECT_NAME}
              OpenMP::OpenMP_CXX
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
