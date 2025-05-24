cmake_minimum_required(VERSION 3.16)

project(trading_charts_simple VERSION ${VERSION_SHORT}) 

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.5 COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCES
    main.cpp
    MainWindow.cpp
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_executable(SimpleExample WIN32 
    ${SOURCES}
)


find_package(qt${QT_VERSION_MAJOR}-edtradingcharts CONFIG QUIET)
if(qt${QT_VERSION_MAJOR}-edtradingcharts_FOUND)
    get_target_property(QTRADINGCHARTS_INCLUDES  ed::qt${QT_VERSION_MAJOR}-edtradingcharts INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(QTRADINGCHARTS_LIBRARIES ed::qt${QT_VERSION_MAJOR}-edtradingcharts INTERFACE_LINK_LIBRARIES)
    message(STATUS "ETradingCharts Found: ${QTRADINGCHARTS_LIBRARIES}, ${QTRADINGCHARTS_INCLUDES}")
    
    target_link_libraries(SimpleExample PUBLIC ed::qt${QT_VERSION_MAJOR}-edtradingcharts)

    target_include_directories(SimpleExample PUBLIC
        ${QTRADINGCHARTS_INCLUDES}
    )
else()
    target_link_libraries(SimpleExample PRIVATE ed::qt${QT_VERSION_MAJOR}-edtradingcharts)
    message(STATUS "ETradingCharts Not Found: Use private instead")
endif()

find_package(qt${QT_VERSION_MAJOR}-qcustomplot CONFIG QUIET)
if(qt${QT_VERSION_MAJOR}-qcustomplot_FOUND)
    get_target_property(QCUSTOMEPLOT_INCLUDES  qcp::qt${QT_VERSION_MAJOR}-qcustomplot INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(QCUSTOMEPLOT_LIBRARIES qcp::qt${QT_VERSION_MAJOR}-qcustomplot INTERFACE_LINK_LIBRARIES)
    message(STATUS "QCustomPlot Found: ${QCUSTOMEPLOT_LIBRARIES}, ${QCUSTOMEPLOT_INCLUDES}")
    
    target_link_libraries(SimpleExample PUBLIC qcp::qt${QT_VERSION_MAJOR}-qcustomplot)

    target_include_directories(SimpleExample PUBLIC
        ${QCUSTOMEPLOT_INCLUDES}
    )
else()
    target_link_libraries(SimpleExample PRIVATE qcp::qt${QT_VERSION_MAJOR}-qcustomplot)
endif()

target_link_libraries(SimpleExample PUBLIC 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Gui 
    Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(SimpleExample PROPERTIES 
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VERSION ${VERSION_SHORT}
    EXPORT_NAME "Qt Dock Menu Simple Example"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
