cmake_minimum_required(VERSION 3.30)

# search path for headers
# include_directories(".")
include_directories("${CMAKE_HOME_DIRECTORY}/ext/date/include")

# link external libraries
add_library(elykseer-crypto_s STATIC IMPORTED)
SET_TARGET_PROPERTIES(elykseer-crypto_s PROPERTIES IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/lib/libelykseer-crypto.a")

IF(${CMAKE_CROSSCOMPILING})
add_library(z STATIC IMPORTED)
SET_TARGET_PROPERTIES(z PROPERTIES IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/lib/libzlibstatic.a")
ENDIF(${CMAKE_CROSSCOMPILING})

set(LIB_PRIVATE_SRCS
    appid.cpp
    filectrl.cpp
    fsutils.cpp
    liz.cpp
    nchunks.cpp
    os.cpp
    coqbuffer.cpp
    coqassembly.cpp
    coqassemblycache.cpp
    coqenvironment.cpp
    coqfilesupport.cpp
    coqstore.cpp
    coqprocessor.cpp
)
# set(LIB_SRCS
#     anything.cppm
# )
set(LIB_HDRS
    lxr.ixx
    appid.ixx
    filectrl.ixx
    fsutils.ixx
    liz.ixx
    nchunks.ixx
    os.ixx
    streamio.ixx
    coqconfiguration.ixx
    coqbuffer.ixx
    coqassembly.ixx
    coqassemblycache.ixx
    coqenvironment.ixx
    coqfilesupport.ixx
    coqstore.ixx
    coqprocessor.ixx
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	message("not building dynamic library on ${CMAKE_SYSTEM_NAME}")
ELSE()
  add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} SHARED)
  target_sources(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}
      PRIVATE
        ${LIB_PRIVATE_SRCS}
      PUBLIC
      FILE_SET CXX_MODULES FILES
        ${LIB_HDRS}
        ${LIB_SRCS}
  )

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


add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s STATIC)
target_sources(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
      PRIVATE
        ${LIB_PRIVATE_SRCS}
      PUBLIC
      FILE_SET CXX_MODULES FILES
        ${LIB_HDRS}
        ${LIB_SRCS}
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
        COMMAND libtool -static -o ${LIBRARY_NAME} ${LIB_PREFIX}${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s.a ../../../ext/${EXT_PATH}/lib/libelykseer-crypto.a
        DEPENDS ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s 
        COMMENT "combining static libraries into ${LIBRARY_NAME}"
  )
  set(CUSTOM_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}")
ELSE()
  file(WRITE "lib_${CMAKE_BUILD_TYPE}.mri"
"create ${LIBRARY_NAME}\n\
addlib ${LIB_PREFIX}${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s.a\n\
addlib ../../../ext/${EXT_PATH}/lib/libelykseer-crypto.a\n\
save\n\
end")

  add_custom_command(OUTPUT ${LIBRARY_NAME}
	COMMAND ${SELECTED_AR} -M <"lib_${CMAKE_BUILD_TYPE}.mri"
	DEPENDS ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s 
	COMMENT "combining static libraries into ${LIBRARY_NAME}"
  )
  set(CUSTOM_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_custom_target(static_lib ALL
	DEPENDS ${LIBRARY_NAME}
)

# profiling
if (CMAKE_BUILD_TYPE STREQUAL "DebugProfile")
	add_library (${PROJECT_NAME}_p${CMAKE_BUILD_TYPE} STATIC)
  target_sources(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
      PRIVATE
        ${LIB_PRIVATE_SRCS}
      PUBLIC
      FILE_SET CXX_MODULES FILES
        ${LIB_HDRS}
        ${LIB_SRCS}
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
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME} DESTINATION lib)
install(FILES ${LIB_HDRS} DESTINATION include/lxr)
install(FILES ${ELYKSEER_CRYPTO_HDRS} DESTINATION include/lxr)

include(CPack)

set(CPACK_BINARY_STGZ "OFF")
set(CPACK_BINARY_TGZ "ON")
