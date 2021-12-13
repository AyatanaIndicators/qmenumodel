project(dbusmenuscript)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS Test REQUIRED)

add_library(dbusmenuscript STATIC dbusmenuscript.cpp)

set_target_properties(dbusmenuscript PROPERTIES COMPILE_FLAGS -fPIC)

target_link_libraries(dbusmenuscript Qt5::Core Qt5::DBus Qt5::Test)

