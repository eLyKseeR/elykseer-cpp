cmake_minimum_required(VERSION 3.5)

set(GENERATED_SRCS
  lxrbackup.hpp lxrbackup.cpp
  lxrrestore.hpp lxrrestore.cpp
  lxrverify.hpp lxrverify.cpp)
set_source_files_properties(${GENERATED_SRCS} GENERATED)

# generate sources from literate c++ files
set(MAKE_CODE_CMD "${CMAKE_CURRENT_SOURCE_DIR}/../../../ext/gitalk/utils/make_all_code.sh")

set(LITERATE_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/cpp/cli")

## executable LXRbackup

add_custom_command(
  COMMAND rm -f lxrbackup.cpp lxrbackup.hpp
  COMMAND ${MAKE_CODE_CMD} "${LITERATE_SRC_PATH}/lxrbackup.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/lxrbackup.md"
  DEPENDS "${LITERATE_SRC_PATH}/lxrbackup.*.md"
  OUTPUT lxrbackup.cpp lxrbackup.hpp)

add_executable ( LXRbackup
  lxrbackup.cpp
)

target_link_libraries (LXRbackup PUBLIC
  elykseer-cpp_${CMAKE_BUILD_TYPE}
  ${libs}
  ${Boost_LIBRARIES}
)

## executable LXRrestore

add_custom_command(
  COMMAND rm -f lxrrestore.cpp lxrrestore.hpp
  COMMAND ${MAKE_CODE_CMD} "${LITERATE_SRC_PATH}/lxrrestore.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/lxrrestore.md"
  DEPENDS "${LITERATE_SRC_PATH}/lxrrestore.*.md"
  OUTPUT lxrrestore.cpp lxrrestore.hpp)

add_executable ( LXRrestore
  lxrrestore.cpp
)

target_link_libraries (LXRrestore PUBLIC
  elykseer-cpp_${CMAKE_BUILD_TYPE}
  ${libs}
  ${Boost_LIBRARIES}
)

## executable LXRverify

add_custom_command(
  COMMAND rm -f lxrverify.cpp lxrverify.hpp
  COMMAND ${MAKE_CODE_CMD} "${LITERATE_SRC_PATH}/lxrverify.md"
  MAIN_DEPENDENCY "${LITERATE_SRC_PATH}/lxrverify.md"
  DEPENDS "${LITERATE_SRC_PATH}/lxrverify.*.md"
  OUTPUT lxrverify.cpp lxrverify.hpp)

add_executable ( LXRverify
  lxrverify.cpp
)

target_link_libraries (LXRverify PUBLIC
  elykseer-cpp_${CMAKE_BUILD_TYPE}
  ${libs}
  ${Boost_LIBRARIES}
)
