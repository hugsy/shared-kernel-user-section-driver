cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(
    shared-kernel-user-section-driver
    LANGUAGES CXX
    VERSION 0.1.0
    DESCRIPTION "shared-kernel-user-section-driver"
    HOMEPAGE_URL https://github.com/hugsy/shared-kernel-user-section-driver
)

set(PROJECT_AUTHOR hugsy)
set(PROJECT_LICENSE MIT)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(DEBUG "Enable Debug" ON)
option(BUILD_MINIFILTER "Compile the minifilter driver" ON)

message(STATUS "Locating Windows Driver Kit")
set(_WIN32_WINNT_WIN10 0x0A00)
set(WDK_WINVER ${_WIN32_WINNT_WIN10})
find_package(WDK REQUIRED)

#
# Build DriverLib
#
add_subdirectory(Common)

#
# Build Minifilter
#
if(BUILD_MINIFILTER)
    add_subdirectory(MiniFilter)
endif()
