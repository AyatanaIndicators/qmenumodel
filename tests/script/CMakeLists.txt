project(dbusmenuscript)

add_library(dbusmenuscript STATIC dbusmenuscript.cpp)

set_target_properties(dbusmenuscript PROPERTIES COMPILE_FLAGS -fPIC)

target_link_libraries(dbusmenuscript Qt5::Core Qt5::DBus Qt5::Test)

