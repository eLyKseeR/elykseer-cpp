cmake_minimum_required(VERSION 3.5)

# search path for headers
include_directories(".")
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/date/include")
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/elykseer-crypto/BUILD/src")

# link external libraries
link_directories("${CMAKE_HOME_DIRECTORY}/../ext/elykseer-crypto/BUILD/src/")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X specific code
   set(libs "dl" "z" "elykseer-crypto_${CMAKE_BUILD_TYPE}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  # specific code
  set(libs "dl" "z" "elykseer-crypto_${CMAKE_BUILD_TYPE}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # Linux specific code
  set(libs "dl" "z" "elykseer-crypto_${CMAKE_BUILD_TYPE}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# list of source files to be compiled
#file(GLOB SRCS *.cpp)
set(GENERATED_SRCS
	appid.cpp
	assembly.cpp
	backupctrl.cpp
	chunk.cpp
	dbbackupjob.cpp
	dbfp.cpp
	dbkey.cpp
	filectrl.cpp
	fsutils.cpp
	liz.cpp
	options.cpp
	os.cpp
	restorectrl.cpp)
set_source_files_properties(${GENERATED_SRCS} GENERATED)
set(GENERATED_HDRS
	lxr/appid.hpp
	lxr/assembly.hpp
	lxr/backupctrl.hpp
	lxr/chunk.hpp
	lxr/dbbackupjob.hpp
	lxr/dbctrl.hpp
	lxr/dbfp.hpp
	lxr/dbkey.hpp
	lxr/filectrl.hpp
	lxr/fsutils.hpp
	lxr/liz.hpp
	lxr/lxr.hpp
	lxr/options.hpp
	lxr/os.hpp
	lxr/restorectrl.hpp
	lxr/streamio.hpp)
set_source_files_properties(${GENERATED_HDRS} GENERATED)

# generate sources from literate c++ files
set(MAKE_CPP_CMD "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/gitalk/utils/make_all_cpp.sh")
set(MAKE_HPP_CMD "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/gitalk/utils/make_all_hpp.sh")

set(LITERATE_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp")

add_custom_command(
	WORKING_DIRECTORY "lxr"
  	#COMMAND rm -f *.hpp
	COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/elykseer-cpp.md"
	MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/elykseer-cpp.md"
    DEPENDS "${LITERATE_SRC_PATH}/*.hpp.md"
	OUTPUT ${GENERATED_HDRS})

add_custom_command(
	#COMMAND rm -f *.cpp
	COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/elykseer-cpp.md"
	MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/elykseer-cpp.md"
    DEPENDS "${LITERATE_SRC_PATH}/*.cpp.md"
	OUTPUT ${GENERATED_SRCS})


add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} SHARED
	${GENERATED_SRCS}
	${GENERATED_HDRS}
)

add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s STATIC
	${GENERATED_SRCS}
	${GENERATED_HDRS}
)

set_target_properties(${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PROPERTIES
                      VERSION ${elykseer-cpp_VERSION_STRING}
                      SOVERSION ${elykseer-cpp_VERSION_MAJOR})

if (CMAKE_BUILD_TYPE STREQUAL "DebugProfile")
	add_library (${PROJECT_NAME}_p${CMAKE_BUILD_TYPE} STATIC
		${GENERATED_SRCS}
	)
	set_target_properties(${PROJECT_NAME}_p${CMAKE_BUILD_TYPE} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
	#set(libs, "${libs} gcov")
	target_link_libraries (${PROJECT_NAME}_p${CMAKE_BUILD_TYPE}
		${libs}
		${Boost_LIBRARIES}
	)
endif()


if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PUBLIC
		OpenMP::OpenMP_CXX
		${libs}
		${Boost_LIBRARIES}
	)
    target_link_libraries(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s PUBLIC
		OpenMP::OpenMP_CXX
		${libs}
		${Boost_LIBRARIES}
	)
else()
	target_link_libraries(${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PUBLIC
		${libs}
		${Boost_LIBRARIES}
	)
	target_link_libraries(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s PUBLIC
		${libs}
		${Boost_LIBRARIES}
	)
endif()

add_subdirectory( cli )
