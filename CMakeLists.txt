cmake_minimum_required(VERSION 3.16)

project(quarkgtkv3 LANGUAGES C)


find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)


add_executable(quarkgtkv3 quarkgtkv3.c)


target_link_libraries(quarkgtkv3 PRIVATE PkgConfig::GTK4)


target_compile_options(quarkgtkv3 PRIVATE ${GTK4_CFLAGS_OTHER})
target_include_directories(quarkgtkv3 PRIVATE ${GTK4_INCLUDE_DIRS})


set_target_properties(quarkgtkv3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)