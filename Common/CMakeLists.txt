message(STATUS "Configuring DriverLib")

wdk_add_library(DriverLib
    STATIC
    KMDF
        1.15

    ../Common/Log.cpp
    ../Common/Log.hpp
    ../Common/Utils.cpp
    ../Common/Utils.hpp
)

target_include_directories(DriverLib INTERFACE ../Common)

