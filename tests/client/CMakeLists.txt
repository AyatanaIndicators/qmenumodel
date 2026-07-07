macro(declare_test testname)
    add_executable(${testname} ${testname}.cpp)
    target_link_libraries(${testname}
                          qmenumodel${QMENUMODEL_LIB_SUFFIX}
                          dbusmenuscript
                          ${GLIB_LDFLAGS}
                          ${GIO_LDFLAGS}
                          Qt::Core
                          Qt::DBus
                          Qt::Widgets
                          Qt::Test
                          Qt::Qml
                          Qt::Quick
    )

    if(TEST_XML_OUTPUT)
        set(TEST_ARGS -p -xunitxml -p -o -p test_${testname}.xml)
    else()
        set(TEST_ARGS "")
    endif()

    add_test(${testname}
             ${DBUS_RUNNER}
             --task ${CMAKE_CURRENT_SOURCE_DIR}/script_${testname}.py --task-name Server
             --ignore-return
             --task ${CMAKE_CURRENT_BINARY_DIR}/${testname} ${TEST_ARGS} --task-name Client
             --wait-for org.ayatana.test)
    set_tests_properties(${testname} PROPERTIES
                         TIMEOUT ${CTEST_TESTING_TIMEOUT}
                         ENVIRONMENT "PYTHONPATH=${TEST_PYTHONPATH};QT_QPA_PLATFORM=minimal")

    set(TESTS ${TESTS} ${testname})
endmacro(declare_test testname)

macro(declare_simple_test testname)
    add_executable(${testname} ${testname}.cpp)
    target_link_libraries(${testname}
                          qmenumodel${QMENUMODEL_LIB_SUFFIX}
                          ${GLIB_LDFLAGS}
                          ${GIO_LDFLAGS}
                          Qt::Core
                          Qt::Test
    )

    add_test(${testname}
             ${CMAKE_CURRENT_BINARY_DIR}/${testname})

    set_tests_properties(${testname} PROPERTIES
                         TIMEOUT ${CTEST_TESTING_TIMEOUT}
                         ENVIRONMENT "QT_QPA_PLATFORM=minimal")
endmacro(declare_simple_test testname)

include_directories(${src_SOURCE_DIR}
                    ${dbusmenuscript_SOURCE_DIR}
                    ${GLIB_INCLUDE_DIRS}
)

add_definitions(-DTEST_SUITE)
set(TEST_PYTHONPATH ${dbusmenuscript_SOURCE_DIR})
if(NOT CTEST_TESTING_TIMEOUT)
    set(CTEST_TESTING_TIMEOUT 60)
endif()

declare_test(servicetest)
declare_test(menuchangestest)
declare_test(modeltest)
declare_test(actiongrouptest)
declare_test(qmltest)
declare_simple_test(convertertest)
declare_simple_test(modelsignalstest)
declare_simple_test(treetest)
declare_simple_test(ayatanamenuactiontest)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qmlfiles.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/qmlfiles.h)

if (ENABLE_COVERAGE)
    find_package(CoverageReport)
    ENABLE_COVERAGE_REPORT(
      TARGETS
        qmenumodel${QMENUMODEL_LIB_SUFFIX}
      FILTER
        ${CMAKE_SOURCE_DIR}/tests/*
        ${CMAKE_BINARY_DIR}/*
      TESTS
        ${TESTS}
    )
endif()
