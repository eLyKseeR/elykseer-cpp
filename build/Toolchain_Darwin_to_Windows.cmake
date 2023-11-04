# the name of the target operating system
#set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_SYSROOT ${CMAKE_HOME_DIRECTORY}/../ext/w64devkit/x86_64-w64-mingw32/)

# which C and C++ compiler to use
set(CMAKE_C_COMPILER /opt/homebrew/Cellar/mingw-w64/11.0.1/bin/x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER /opt/homebrew/Cellar/mingw-w64/11.0.1/bin/x86_64-w64-mingw32-g++)

# location of the target environment
set(CMAKE_FIND_ROOT_PATH ${CMAKE_HOME_DIRECTORY}/../ext/targets/${CMAKE_SYSTEM_NAME})

# adjust the default behavior of the FIND_XXX() commands:
# search for headers and libraries in the target environment,
# search for programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

