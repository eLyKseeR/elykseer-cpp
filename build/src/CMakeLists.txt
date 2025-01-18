cmake_minimum_required(VERSION 3.25)

# search path for headers
include_directories(".")
include_directories("${CMAKE_HOME_DIRECTORY}/../ext/date/include")

# link external libraries
add_library(elykseer-crypto_s STATIC IMPORTED)
SET_TARGET_PROPERTIES(elykseer-crypto_s PROPERTIES IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/../ext/${EXT_PATH}/lib/libelykseer-crypto.a")

IF(${CMAKE_CROSSCOMPILING})
add_library(z STATIC IMPORTED)
SET_TARGET_PROPERTIES(z PROPERTIES IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/../ext/${EXT_PATH}/lib/libzlibstatic.a")
ENDIF(${CMAKE_CROSSCOMPILING})


# list of source files to be compiled
set(GENERATED_SRCS
	appid.cpp
	coqassembly.cpp
	coqassemblycache.cpp
	coqbuffer.cpp
	coqenvironment.cpp
	coqfilesupport.cpp
	coqprocessor.cpp
	coqstore.cpp
	filectrl.cpp
	fsutils.cpp
	liz.cpp
	nchunks.cpp
	os.cpp)
set_source_files_properties(${GENERATED_SRCS} GENERATED)
set(GENERATED_HDRS
	lxr/appid.hpp
	lxr/coqassembly.hpp
	lxr/coqassemblycache.hpp
	lxr/coqbuffer.hpp
	lxr/coqconfiguration.hpp
	lxr/coqenvironment.hpp
	lxr/coqfilesupport.hpp
	lxr/coqprocessor.hpp
	lxr/coqstore.hpp
	lxr/filectrl.hpp
	lxr/fsutils.hpp
	lxr/liz.hpp
	lxr/lxr.hpp
	lxr/nchunks.hpp
	lxr/os.hpp
	lxr/streamio.hpp)
set_source_files_properties(${GENERATED_HDRS} GENERATED)

# generate sources from literate c++ files
set(MAKE_CPP_CMD ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/gitalk/utils/make_cpp.sh)
set(MAKE_HPP_CMD ${CMAKE_COMMAND} -E env HTML2TEXT=${HTML2TEXT} ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/gitalk/utils/make_hpp.sh)

set(LITERATE_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp")

add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f lxr.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/lxr.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/lxr.hpp.md"
  OUTPUT lxr/lxr.hpp)

add_custom_command(
  COMMAND rm -f appid.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/appid.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/appid.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/appid*.md"
  OUTPUT appid.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f appid.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/appid.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/appid.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/appid*.md"
  OUTPUT lxr/appid.hpp)

add_custom_command(
  COMMAND rm -f coqassembly.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/coqassembly.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqassembly.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqassembly*.md"
  OUTPUT coqassembly.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqassembly.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqassembly.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqassembly.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqassembly*.md"
  OUTPUT lxr/coqassembly.hpp)

add_custom_command(
  COMMAND rm -f coqassemblycache.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/coqassemblycache.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqassemblycache.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqassemblycache*.md"
  OUTPUT coqassemblycache.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqassemblycache.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqassemblycache.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqassemblycache.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqassemblycache*.md"
  OUTPUT lxr/coqassemblycache.hpp)

add_custom_command(
  COMMAND rm -f coqbuffer.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/coqbuffer.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqbuffer.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqbuffer*.md"
  OUTPUT coqbuffer.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqbuffer.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqbuffer.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqbuffer.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqbuffer*.md"
  OUTPUT lxr/coqbuffer.hpp)

add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqconfiguration.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqconfiguration.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqconfiguration.hpp.md"
  OUTPUT lxr/coqconfiguration.hpp)

add_custom_command(
  COMMAND rm -f coqenvironment.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/coqenvironment.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqenvironment.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqenvironment*.md"
  OUTPUT coqenvironment.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqenvironment.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqenvironment.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqenvironment.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqenvironment*.md"
  OUTPUT lxr/coqenvironment.hpp)

add_custom_command(
  COMMAND rm -f coqfilesupport.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/coqfilesupport.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqfilesupport.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqfilesupport*.md"
  OUTPUT coqfilesupport.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqfilesupport.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqfilesupport.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqfilesupport.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqfilesupport*.md"
  OUTPUT lxr/coqfilesupport.hpp)

add_custom_command(
  COMMAND rm -f coqprocessor.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/coqprocessor.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqprocessor.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqprocessor*.md"
  OUTPUT coqprocessor.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqprocessor.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqprocessor.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqprocessor.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqprocessor*.md"
  OUTPUT lxr/coqprocessor.hpp)

add_custom_command(
  COMMAND rm -f coqstore.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/coqstore.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqstore.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqstore*.md"
  OUTPUT coqstore.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f coqstore.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/coqstore.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/coqstore.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/coqstore*.md"
  OUTPUT lxr/coqstore.hpp)

add_custom_command(
  COMMAND rm -f filectrl.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/filectrl.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/filectrl.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/filectrl*.md"
  OUTPUT filectrl.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f filectrl.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/filectrl.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/filectrl.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/filectrl*.md"
  OUTPUT lxr/filectrl.hpp)

add_custom_command(
  COMMAND rm -f fsutils.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/fsutils.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/fsutils.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/fsutils*.md"
  OUTPUT fsutils.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f fsutils.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/fsutils.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/fsutils.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/fsutils*.md"
  OUTPUT lxr/fsutils.hpp)

add_custom_command(
  COMMAND rm -f liz.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/liz.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/liz.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/liz*.md"
  OUTPUT liz.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f liz.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/liz.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/liz.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/liz*.md"
  OUTPUT lxr/liz.hpp)

add_custom_command(
  COMMAND rm -f nchunks.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/nchunks.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/nchunks.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/nchunks*.md"
  OUTPUT nchunks.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f nchunks.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/nchunks.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/nchunks.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/nchunks*.md"
  OUTPUT lxr/nchunks.hpp)

add_custom_command(
  COMMAND rm -f os.cpp
  COMMAND ${MAKE_CPP_CMD} "${LITERATE_SRC_PATH}/os.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/os.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/os*.md"
  OUTPUT os.cpp)
add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f os.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/os.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/os.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/os*.md"
  OUTPUT lxr/os.hpp)

add_custom_command(
  WORKING_DIRECTORY "lxr"
  COMMAND rm -f streamio.hpp
  COMMAND ${MAKE_HPP_CMD} "${LITERATE_SRC_PATH}/streamio.hpp.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/streamio.hpp.md"
  DEPENDS "${LITERATE_SRC_PATH}/streamio*.md"
  OUTPUT lxr/streamio.hpp)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	message("not building dynamic library on ${CMAKE_SYSTEM_NAME}")
ELSE()
  add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} SHARED
               ${GENERATED_SRCS}
               ${GENERATED_HDRS})
  set_target_properties(${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PROPERTIES
                        VERSION ${elykseer-cpp_VERSION_STRING}
                        SOVERSION ${elykseer-cpp_VERSION_MAJOR})

  target_link_libraries (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PUBLIC
      ${libs}
	    elykseer-crypto_s
  )
  if(OpenMP_CXX_FOUND)
    target_link_libraries (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PUBLIC
	    OpenMP::OpenMP_CXX)
  endif()

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s STATIC
	${GENERATED_SRCS}
	${GENERATED_HDRS}
)
target_link_libraries(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s PUBLIC
  ${libs}
)
if(OpenMP_CXX_FOUND)
  target_link_libraries (${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s PUBLIC
	    OpenMP::OpenMP_CXX)
endif()

# combine all static libs into one
IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  set(LIBRARY_NAME "lib${PROJECT_NAME}_d.a")
ELSE()
  set(LIBRARY_NAME "lib${PROJECT_NAME}.a")
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_custom_command(OUTPUT ${LIBRARY_NAME}
        COMMAND libtool -static -o ${LIBRARY_NAME} ${LIB_PREFIX}${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s.a ../../ext/${EXT_PATH}/lib/libelykseer-crypto.a
        DEPENDS ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s 
        COMMENT "combining static libraries into ${LIBRARY_NAME}"
  )
ELSE()
  file(WRITE "lib_${CMAKE_BUILD_TYPE}.mri"
"create ${LIBRARY_NAME}\n\
addlib ${LIB_PREFIX}${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s.a\n\
addlib ../../ext/${EXT_PATH}/lib/libelykseer-crypto.a\n\
save\n\
end")

  add_custom_command(OUTPUT ${LIBRARY_NAME}
	COMMAND ${SELECTED_AR} -M <"lib_${CMAKE_BUILD_TYPE}.mri"
	DEPENDS ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s 
	COMMENT "combining static libraries into ${LIBRARY_NAME}"
  )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_custom_target(static_lib ALL
	DEPENDS ${LIBRARY_NAME}
)

# profiling
if (CMAKE_BUILD_TYPE STREQUAL "DebugProfile")
	add_library (${PROJECT_NAME}_p${CMAKE_BUILD_TYPE} STATIC
		${GENERATED_SRCS}
	)
	set_target_properties(${PROJECT_NAME}_p${CMAKE_BUILD_TYPE} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
	#set(libs, "${libs} gcov")
	target_link_libraries (${PROJECT_NAME}_p${CMAKE_BUILD_TYPE}
		${libs}
	)
endif()

add_subdirectory( cli )

set(ELYKSEER_CRYPTO_HDRS
  ../../ext/${EXT_PATH}/include/lxr/aes.hpp
  ../../ext/${EXT_PATH}/include/lxr/base64.hpp
  ../../ext/${EXT_PATH}/include/lxr/gpg.hpp
  ../../ext/${EXT_PATH}/include/lxr/hmac.hpp
  ../../ext/${EXT_PATH}/include/lxr/key.hpp
  ../../ext/${EXT_PATH}/include/lxr/key128.hpp
  ../../ext/${EXT_PATH}/include/lxr/key160.hpp
  ../../ext/${EXT_PATH}/include/lxr/key256.hpp
  ../../ext/${EXT_PATH}/include/lxr/lxr-cbindings.hpp
  ../../ext/${EXT_PATH}/include/lxr/md5.hpp
  ../../ext/${EXT_PATH}/include/lxr/randlist.hpp
  ../../ext/${EXT_PATH}/include/lxr/random.hpp
  ../../ext/${EXT_PATH}/include/lxr/sha256.hpp
  ../../ext/${EXT_PATH}/include/lxr/sha3.hpp
)
set(CPACK_PACKAGE_VERSION ${${PROJECT_NAME}_VERSION_STRING})
set(CPACK_PACKAGE_VERSION_MAJOR ${${PROJECT_NAME}_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${${PROJECT_NAME}_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${${PROJECT_NAME}_VERSION_PATCH})
install(FILES ${LIBRARY_NAME} DESTINATION lib)
install(FILES ${GENERATED_HDRS} DESTINATION include/lxr)
install(FILES ${ELYKSEER_CRYPTO_HDRS} DESTINATION include/lxr)
include(CPack)

set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TGZ "ON")
