cmake_minimum_required(VERSION 3.16)
project(QtTradingCharts LANGUAGES CXX VERSION ${VERSION_SHORT})

include(GNUInstallDirs)

if (${QT_VERSION_MAJOR})
  message(STATUS "Forced to use Qt version ${QT_VERSION_MAJOR}")
  find_package(QT NAMES Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
else()
  find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
endif()

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)
if (UNIX AND NOT APPLE)
    include_directories(${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS})
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(BUILD_STATIC)
    set(CMAKE_STATIC_LIBRARY_SUFFIX "_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()


set(TRADING_CHARTS_SRCS
    ed/tradingcharts/TradingPlot.cpp
    ed/tradingcharts/ResizeHandle.cpp
    ed/tradingcharts/TradingRect.cpp
)

set(TRADING_CHARTS_HEADERS
    ed/tradingcharts/ed_globals.h
    ed/tradingcharts/TradingPlot.h
    ed/tradingcharts/ResizeHandle.h
    ed/tradingcharts/TradingPlotable.h
    ed/tradingcharts/TradingRect.h
)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(LIBRARY_NAME "qt${QT_VERSION_MAJOR}-edtradingcharts")

if(ED_BUILD_STATIC)
    add_library(${LIBRARY_NAME} STATIC ${RESOURCES_RCC} ${TRADING_CHARTS_SRCS} ${TRADING_CHARTS_HEADERS})
    target_compile_definitions( ${LIBRARY_NAME} PUBLIC ED_TRADINGCHARTS_STATIC)
else()
    add_library( ${LIBRARY_NAME} SHARED ${RESOURCES_RCC} ${TRADING_CHARTS_SRCS} ${TRADING_CHARTS_HEADERS})
    target_compile_definitions( ${LIBRARY_NAME} PRIVATE ED_TRADINGCHARTS_SHARED_EXPORT)
endif()

add_library(ed::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})


find_package(qt${QT_VERSION_MAJOR}-qcustomplot CONFIG QUIET)
if(qt${QT_VERSION_MAJOR}-qcustomplot_FOUND)
    get_target_property(QCUSTOMEPLOT_INCLUDES  qcp::qt${QT_VERSION_MAJOR}-qcustomplot INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(QCUSTOMEPLOT_LIBRARIES qcp::qt${QT_VERSION_MAJOR}-qcustomplot INTERFACE_LINK_LIBRARIES)
    message(STATUS "QCustomPlot Found: ${QCUSTOMEPLOT_LIBRARIES}, ${QCUSTOMEPLOT_INCLUDES}")
    
    target_link_libraries(${LIBRARY_NAME} PUBLIC qcp::qt${QT_VERSION_MAJOR}-qcustomplot)

    target_include_directories(${LIBRARY_NAME} PUBLIC
        ${QCUSTOMEPLOT_INCLUDES}
    )
else()
    target_link_libraries(${LIBRARY_NAME} PRIVATE qcp::qt${QT_VERSION_MAJOR}-qcustomplot)
endif()

target_link_libraries(${LIBRARY_NAME} PUBLIC 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Gui 
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(${LIBRARY_NAME} PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if (UNIX AND NOT APPLE)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        find_package(X11 REQUIRED)
        target_link_libraries(${LIBRARY_NAME} PUBLIC X11::xcb)
    else()
        target_link_libraries(${LIBRARY_NAME} PUBLIC xcb)
    endif()
endif()

set_target_properties(${LIBRARY_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    CXX_EXTENSIONS OFF
    VERSION ${VERSION_SHORT}
    EXPORT_NAME ${LIBRARY_NAME}
    DEBUG_POSTFIX "d"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(QT_VERSION_MAJOR STREQUAL "5")
    set_target_properties(${LIBRARY_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON)
elseif(QT_VERSION_MAJOR STREQUAL "6")
    set_target_properties(${LIBRARY_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
endif()

install(FILES ${TRADING_CHARTS_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME}/ed/tradingcharts
    COMPONENT headers
)
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
    DESTINATION license/ed
    COMPONENT license
)

# Export targets for find_package
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${LIBRARY_NAME}ConfigVersion.cmake"
    VERSION ${VERSION_SHORT}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS ${LIBRARY_NAME}
    EXPORT edtradingchartsTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME}
)

install(EXPORT edtradingchartsTargets
    FILE edtradingchartsTargets.cmake
    NAMESPACE ed::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
)

install(FILES cmake/edtradingchartsConfig.cmake RENAME ${LIBRARY_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
)