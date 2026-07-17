cmake_minimum_required(VERSION 3.28)

project(jlibc VERSION 0.1 LANGUAGES C DESCRIPTION "nolibc minimal C library")

# jlibc
add_library(jlibc OBJECT libs/jlibc.c libs/jlibc.h)
target_include_directories(jlibc PUBLIC libs)
target_compile_options(jlibc PUBLIC -ffreestanding -nostdlib)

# nolibc linktime flags
add_library(nolibc_exe_flags INTERFACE)
target_link_options(nolibc_exe_flags INTERFACE -nostdlib -static -no-pie -Wl,-e,_start)

# jthreads
add_library(jthreads OBJECT libs/jthreads.c libs/jthreads.h)
target_link_libraries(jthreads PUBLIC jlibc)

# jmem
add_library(jmem OBJECT libs/jmem.c libs/jmem.h)
target_link_libraries(jmem PUBLIC jlibc)

# testing futex
add_executable(jtest utils/threads/futex.c)
target_link_libraries(jtest PRIVATE jlibc nolibc_exe_flags)