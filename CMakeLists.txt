# rva: experiments in computing in the space of Brevais Lattices
# Copyright (C) 2014  University of Chicago
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any
# damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any
# purpose, including commercial applications, and to alter it and
# redistribute it freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must
#    not claim that you wrote the original software. If you use this
#    software in a product, an acknowledgment in the product
#    documentation would be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must
#    not be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source distribution.


# HEY: CMake novice here; feel free to fix ...
cmake_minimum_required(VERSION 2.6)

project(reva)

find_package(Teem REQUIRED)
include(${Teem_USE_FILE})

# foo_reva.c implements "reva foo"
add_library(rva rva.h rvaPrivate.h util.c spec.c grid.c
            about_reva.c fold_reva.c dia_reva.c conv_reva.c grid_reva.c)
add_executable(reva reva.c)
target_link_libraries(reva rva teem)

install(TARGETS reva RUNTIME DESTINATION bin)
install(TARGETS rva
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install (FILES rva.h DESTINATION include)
