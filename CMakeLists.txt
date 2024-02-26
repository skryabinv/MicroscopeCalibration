cmake_minimum_required(VERSION 3.5)

project(MicroscopeCalibration VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(Eigen3_DIR 3rd/ceres-solver-2.2.0/dependencies/Eigen3/share/eigen3/cmake)
set(gflags_DIR 3rd/ceres-solver-2.2.0/dependencies/gflags/lib/cmake/gflags)
set(glog_DIR   3rd/ceres-solver-2.2.0/dependencies/glog/lib/cmake/glog)
set(ceres_DIR  3rd/ceres-solver-2.2.0/install/lib/cmake/Ceres)
set(OpenCV_DIR 3rd/opencv-4.5.1/)

find_package(Eigen3)
find_package(gflags)
find_package(glog)
find_package(ceres)
find_package(OpenCV)

set(PROJECT_SOURCES
    main.cpp
    MainWidget.h MainWidget.cpp
    Calibration.h Calibration.cpp
    CircleFit.h
    CircleFit.cpp
    CalibrationCostFunction.h
    Graphics.h
    TargetImage.h TargetImage.cpp
    CameraModel.h CameraModel.cpp
    WidgetEditorROI.h WidgetEditorROI.cpp WidgetEditorROI.ui
    OpticalCenterCostFunction.h
    WidgetPixelSizeCalibration.h WidgetPixelSizeCalibration.cpp WidgetPixelSizeCalibration.ui
    WidgetCameraModel.h WidgetCameraModel.cpp WidgetCameraModel.ui
    WidgetOpticalCenterSearch.h WidgetOpticalCenterSearch.cpp WidgetOpticalCenterSearch.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MicroscopeCalibration
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MicroscopeCalibration APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MicroscopeCalibration SHARED
            ${PROJECT_SOURCES}
        )
    # Define properties for Android with Qt 5 after find_package() calls as:
    #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
    add_executable(MicroscopeCalibration
        ${PROJECT_SOURCES}
    )
endif()
endif()

target_link_libraries(MicroscopeCalibration PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(MicroscopeCalibration PRIVATE ceres gflags_static glog::glog ${OpenCV_LIBS})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MicroscopeCalibration)
endif()
set_target_properties(MicroscopeCalibration PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MicroscopeCalibration
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MicroscopeCalibration)
endif()
