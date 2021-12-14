project(qmenumodel)

set (PROJECT_VERSION "0.9.0")

cmake_minimum_required(VERSION 3.13)

# Standard install paths
include(GNUInstallDirs)

find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Gui REQUIRED)
include(FindPkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32)
pkg_check_modules(GIO REQUIRED gio-2.0>=2.32)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions(-DQT_NO_KEYWORDS)

find_program(DBUS_RUNNER dbus-test-runner)

add_subdirectory(libqmenumodel)

# Tests Tools
OPTION(TEST_XML_OUTPUT "Print test results on xml files" ON)
if(NOT DBUS_RUNNER)
    message(STATUS "dbus-test-runner not found tests disabled.")
else()
    # We need to enable test to create the 'make check' target mandatory on jenkins
    enable_testing()
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
        message(STATUS "Current version of qemu crashes during the tests. We will skip it for now.")
    else()
        message(STATUS "Tests enabled for arch: ${CMAKE_SYSTEM_PROCESSOR}")
        add_subdirectory(tests)
    endif()
endif()

# Doc
OPTION(GENERATE_DOC "Enable qdoc generation" OFF)
if(GENERATE_DOC)
    message(STATUS "QDoc enabled.")
    find_program(QDOC_BIN qdoc)
    if(NOT QDOC_BIN)
        message(FATAL_ERROR "qdoc command not found")
    else()
        add_subdirectory(doc)
    endif()
endif()

# Coverage
#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage (or ninja if you use that backend)
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
find_package(CoverageReport)
