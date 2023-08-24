cmake_minimum_required (VERSION 3.13)
project (qmenumodel VERSION 0.9.2)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Options
option(ENABLE_TESTS "Enable all tests and checks" OFF)
option(ENABLE_COVERAGE "Enable coverage reports (includes enabling all tests and checks)" OFF)
option(ENABLE_WERROR "Treat all build warnings as errors" OFF)
option(TEST_XML_OUTPUT "Print test results to xml files" OFF)
option(GENERATE_DOC "Enable qdoc generation" OFF)

if(ENABLE_COVERAGE)
    set(ENABLE_TESTS ON)
    set(CMAKE_BUILD_TYPE "Coverage")
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(ENABLE_TESTS)
    set(TEST_XML_OUTPUT ON)
endif()

if(ENABLE_WERROR)
    add_definitions("-Werror")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions("-Weverything")
else()
    add_definitions("-Wall")
endif()

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
add_subdirectory(libqmenumodel)

# Tests
if (ENABLE_TESTS)
    find_program(DBUS_RUNNER dbus-test-runner REQUIRED)
    enable_testing()
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
        message(STATUS "Current version of qemu crashes during the tests. We will skip it for now.")
        set(ENABLE_TESTS OFF)
        set(ENABLE_COVERAGE OFF)
        set(TEST_XML_OUTPUT OFF)
    else()
        add_subdirectory(tests)
    endif()
endif()

# Doc
if(GENERATE_DOC)
    find_program(QDOC_BIN qdoc REQUIRED)
    add_subdirectory(doc)
endif()

# Display config info
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Unit tests: ${ENABLE_TESTS}")
message(STATUS "Build with -Werror: ${ENABLE_WERROR}")
message(STATUS "Print test results to xml files: ${TEST_XML_OUTPUT}")
message(STATUS "QDoc generation: ${GENERATE_DOC}")
