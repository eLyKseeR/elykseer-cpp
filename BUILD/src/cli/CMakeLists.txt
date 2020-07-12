cmake_minimum_required(VERSION 2.8)

add_executable ( LXRbackup
  lxrbackup.cpp
)

target_link_libraries (LXRbackup PUBLIC
  elykseer-cpp_${CMAKE_BUILD_TYPE}
  ${libs}
  ${Boost_LIBRARIES}
)

add_executable ( LXRrestore
  lxrrestore.cpp
)

target_link_libraries (LXRrestore PUBLIC
  elykseer-cpp_${CMAKE_BUILD_TYPE}
  ${libs}
  ${Boost_LIBRARIES}
)
