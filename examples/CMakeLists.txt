# Standalone CMakeLists.txt to generate the info-menumodel executable.
# This is not built as part of libqmenumodel, it has to be built separately
# and it assumes libqmenumodel-dev is installed on the system.

cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)

include(FindPkgConfig)
pkg_check_modules(QMENUMODEL REQUIRED qmenumodel)

add_executable(info-menumodel info-menumodel.cpp)

include_directories(${QMENUMODEL_INCLUDE_DIRS})
target_link_libraries(info-menumodel ${QMENUMODEL_LDFLAGS} Qt5::Core)

