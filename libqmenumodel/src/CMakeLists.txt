project(src)

set(QMENUMODEL_SRC
    actionstateparser.cpp
    converter.cpp
    dbus-enums.h
    menunode.cpp
    qmenumodel.cpp
    qdbusobject.cpp
    qdbusmenumodel.cpp
    qdbusactiongroup.cpp
    qmenumodelevents.cpp
    qstateaction.cpp
    ayatanamenuaction.cpp
    ayatanamenuactionevents.cpp
    ayatanamenumodel.cpp
    ayatanamenumodelevents.cpp
    gtk/gtkactionmuxer.c
    gtk/gtkactionmuxer.h
    gtk/gtkactionobservable.c
    gtk/gtkactionobservable.h
    gtk/gtkactionobserver.c
    gtk/gtkactionobserver.h
    gtk/gtksimpleactionobserver.c
    gtk/gtksimpleactionobserver.h
    gtk/gtkmenutracker.c
    gtk/gtkmenutracker.h
    gtk/gtkmenutrackeritem.c
    gtk/gtkmenutrackeritem.h
)

set(SHAREDLIBNAME qmenumodel)
add_library(${SHAREDLIBNAME} SHARED
    ${QMENUMODEL_SRC}
)

set_target_properties(${SHAREDLIBNAME} PROPERTIES
    COMPILE_FLAGS -fPIC
    SOVERSION 1
    VERSION 1.0.0
)

include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)

find_package(Qt5Quick REQUIRED)
target_link_libraries(${SHAREDLIBNAME}
    ${GLIB_LDFLAGS}
    ${GIO_LDFLAGS}
    Qt5::Core Qt5::Qml Qt5::Quick
)

install(TARGETS ${SHAREDLIBNAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(QMENUMODEL_HEADERS
    actionstateparser.h
    dbus-enums.h
    qdbusactiongroup.h
    qdbusmenumodel.h
    qdbusobject.h
    qmenumodel.h
    qstateaction.h
    ayatanamenuaction.h
    ayatanamenumodel.h
)

set(INCLUDEDIR qmenumodel)
install(FILES ${QMENUMODEL_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${INCLUDEDIR}
)

set(PCFILE qmenumodel.pc)
configure_file(${PCFILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${PCFILE} @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PCFILE}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

