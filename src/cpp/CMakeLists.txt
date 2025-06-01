cmake_minimum_required(VERSION 3.30)

# search path for headers
# include_directories(".")
include_directories("${CMAKE_HOME_DIRECTORY}/ext/date/include")

# IF(${CMAKE_CROSSCOMPILING})
# add_library(z STATIC IMPORTED)
# SET_TARGET_PROPERTIES(z PROPERTIES IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/ext/${EXT_PATH}/lib/libzlibstatic.a")
# ENDIF(${CMAKE_CROSSCOMPILING})

set(LIB_PRIVATE_SRCS
    appid.cpp
    filectrl.cpp
    fsutils.cpp
    liz.cpp
    nchunks.cpp
    os.cpp
    coqfilesupport.cpp
    coqbuffer.cpp
    coqassembly.cpp
    coqassemblycache.cpp
    coqenvironment.cpp
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
    # generator.hpp
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

# IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
# 	message("not building dynamic library on ${CMAKE_SYSTEM_NAME}")
# ELSE()
#   add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} SHARED)
#   target_sources(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}
#       PRIVATE
#         ${LIB_PRIVATE_SRCS}
#       PUBLIC
#       FILE_SET CXX_MODULES FILES
#         ${LIB_HDRS}
#         ${LIB_SRCS}
#   )
#
#   set_target_properties(${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PROPERTIES
#                         VERSION ${elykseer-cpp_VERSION_STRING}
#                         SOVERSION ${elykseer-cpp_VERSION_MAJOR})
#
#   target_link_libraries (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PUBLIC
#       ${libs}
# 	    elykseer-crypto_s
#   )
#   if(OpenMP_CXX_FOUND)
#     target_link_libraries (${PROJECT_NAME}_${CMAKE_BUILD_TYPE} PUBLIC
# 	    OpenMP::OpenMP_CXX)
#   endif()
#
# ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


add_library (${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s STATIC)
target_sources(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
      PRIVATE
        ${LIB_PRIVATE_SRCS}
      PUBLIC
      FILE_SET CXX_MODULES FILES
        ${LIB_HDRS}
        ${LIB_SRCS}
)
target_link_libraries(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
      PRIVATE
        elykseer-crypto_s
      PUBLIC
        ${libs}
)
# target_compile_options(${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s PRIVATE -fmodules)
if(OpenMP_CXX_FOUND)
  target_link_libraries (${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s PUBLIC
	    OpenMP::OpenMP_CXX)
endif()

# combine all static libs into one
set(LIBRARY_NAME "lib${PROJECT_NAME}.a")
# IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
#   set(LIBRARY_NAME "lib${PROJECT_NAME}_d.a")
# ELSE()
#   set(LIBRARY_NAME "lib${PROJECT_NAME}.a")
# ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_custom_command(OUTPUT ${LIBRARY_NAME}
        COMMAND libtool -static -o ${LIBRARY_NAME} ${LIB_PREFIX}${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s.a ../../../ext/${EXT_PATH}/lib/${LIB_ELYKSEER_CRYPTO}
        DEPENDS ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s 
        COMMENT "combining static libraries into ${LIBRARY_NAME}"
  )
  set(CUSTOM_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}")
ELSE()
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/lib_${CMAKE_BUILD_TYPE}.mri"
"create ${LIBRARY_NAME}\n\
addlib ${LIB_PREFIX}${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s.a\n\
addlib ../../../ext/${EXT_PATH}/lib/${LIB_ELYKSEER_CRYPTO}\n\
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
  ../../ext/${EXT_PATH}/include/lxr/aes.ixx
  ../../ext/${EXT_PATH}/include/lxr/base64.ixx
  ../../ext/${EXT_PATH}/include/lxr/gpg.ixx
  ../../ext/${EXT_PATH}/include/lxr/hmac.ixx
  ../../ext/${EXT_PATH}/include/lxr/key.ixx
  ../../ext/${EXT_PATH}/include/lxr/key128.ixx
  ../../ext/${EXT_PATH}/include/lxr/key160.ixx
  ../../ext/${EXT_PATH}/include/lxr/key256.ixx
  ../../ext/${EXT_PATH}/include/lxr/md5.ixx
  ../../ext/${EXT_PATH}/include/lxr/randlist.ixx
  ../../ext/${EXT_PATH}/include/lxr/random.ixx
  # ../../ext/${EXT_PATH}/include/lxr/sha256.ixx
  ../../ext/${EXT_PATH}/include/lxr/sha3.ixx
  ../../ext/${EXT_PATH}/include/lxr/lxr-cbindings.hpp
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
