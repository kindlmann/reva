cmake_minimum_required(VERSION 2.6)

project(reva)

find_package(Teem REQUIRED)
include(${Teem_USE_FILE})

add_library(rva util.c flotsam.c reva.c about.c fold.c)
add_executable(reva reva.c)
target_link_libraries(reva rva teem)
