project(qmlplugin)

set(QMLPLUGIN_SRC
    plugin.cpp
)

add_library(qmenumodel-qml MODULE
    ${QMLPLUGIN_SRC}
)

include_directories(
    ${src_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(qmenumodel-qml
    qmenumodel${QMENUMODEL_LIB_SUFFIX}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
    Qt::Qml Qt::Quick
)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/qmldir"
                                                              "${CMAKE_CURRENT_BINARY_DIR}/qmldir")

set(QT_IMPORTS_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/qml")
set(QMLPLUGIN_INSTALL_PREFIX "${QT_IMPORTS_DIR}/QMenuModel.1")
install(TARGETS qmenumodel-qml DESTINATION ${QMLPLUGIN_INSTALL_PREFIX})
install(FILES qmldir DESTINATION ${QMLPLUGIN_INSTALL_PREFIX})

