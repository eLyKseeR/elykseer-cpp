cmake_minimum_required(VERSION 3.30)

# find_package(elykseer-cpp 4.0 REQUIRED)

# add_library(elykseer-cpp_s STATIC IMPORTED)
# SET_TARGET_PROPERTIES(elykseer-cpp_s PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/../${LIBRARY_NAME}")

# add_library(elykseer-cpp_s STATIC IMPORTED)
# if (CMAKE_BUILD_TYPE STREQUAL "Release")
# SET_TARGET_PROPERTIES(elykseer-cpp_s PROPERTIES IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/src/libelykseer-cpp.a")
# else()
# SET_TARGET_PROPERTIES(elykseer-cpp_s PROPERTIES IMPORTED_LOCATION "${CMAKE_HOME_DIRECTORY}/src/libelykseer-cpp_d.a")
# endif()

set(STATIC_BUILD "")
IF(${CMAKE_CROSSCOMPILING})
set(STATIC_BUILD "-static")
ENDIF(${CMAKE_CROSSCOMPILING})

add_executable (LXRbackup
  lxrbackup.cpp
)
target_link_libraries (LXRbackup PRIVATE
  ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
  elykseer-crypto_s
  ${libs}
  ${Boost_LIBRARIES}
  ${STATIC_BUILD}
)
if(OpenMP_CXX_FOUND)
  target_link_libraries (LXRbackup PUBLIC OpenMP::OpenMP_CXX)
endif()


add_executable (LXRrestore
  lxrrestore.cpp
)
target_link_libraries (LXRrestore PUBLIC
  ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
  elykseer-crypto_s
  ${libs}
  ${Boost_LIBRARIES}
  ${STATIC_BUILD}
)
if(OpenMP_CXX_FOUND)
  target_link_libraries (LXRrestore PUBLIC OpenMP::OpenMP_CXX)
endif()

add_executable (LXRverify
  lxrverify.cpp
)
target_link_libraries (LXRverify PUBLIC
  ${PROJECT_NAME}_${CMAKE_BUILD_TYPE}_s
  elykseer-crypto_s
  ${libs}
  ${Boost_LIBRARIES}
  ${STATIC_BUILD}
)
if(OpenMP_CXX_FOUND)
  target_link_libraries (LXRverify PUBLIC OpenMP::OpenMP_CXX)
endif()

install(TARGETS LXRbackup RUNTIME DESTINATION bin)
install(TARGETS LXRrestore RUNTIME DESTINATION bin)
install(TARGETS LXRverify RUNTIME DESTINATION bin)