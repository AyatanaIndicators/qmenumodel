# Standalone CMakeLists.txt to generate the info-menumodel executable.
# This is not built as part of libqmenumodel, it has to be built separately
# and it assumes libqmenumodel-dev is installed on the system.

cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

option(ENABLE_QT6 "Enable building with Qt6" OFF)
set(QT_VERSION_MAJOR 5)
set(QMENUMODEL_PKGCONFIG_SUFFIX)
if(ENABLE_QT6)
    set(QT_VERSION_MAJOR 6)
    string(APPEND QMENUMODEL_PKGCONFIG_SUFFIX "-qt6")
endif()

find_package(QT NAMES Qt${QT_VERSION_MAJOR})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

include(FindPkgConfig)
pkg_check_modules(QMENUMODEL REQUIRED qmenumodel${QMENUMODEL_PKGCONFIG_SUFFIX})

add_executable(info-menumodel info-menumodel.cpp)

include_directories(${QMENUMODEL_INCLUDE_DIRS})
target_link_libraries(info-menumodel ${QMENUMODEL_LDFLAGS} Qt::Core)

