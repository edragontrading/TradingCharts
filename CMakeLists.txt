cmake_minimum_required(VERSION 3.17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)

set(VERSION_SHORT "1.0.0")

project(QtTradingCharts LANGUAGES CXX VERSION ${VERSION_SHORT})

option(BUILD_STATIC     "Build the static library"  ON)
option(BUILD_EXAMPLES   "Build the examples"        OFF)

find_package(qt${QT_VERSION_MAJOR}-qcustomplot CONFIG QUIET)
if(NOT qt${QT_VERSION_MAJOR}-qcustomplot_FOUND)
    message(STATUS "QCustomPlot Not Found: Use private instead")
    add_subdirectory(3rd-party/qcustomplot)
endif()

add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

