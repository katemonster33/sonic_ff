cmake_minimum_required(VERSION 2.6)
project(sonic_ff)

add_executable(sonic_ff main.cpp)

install(TARGETS sonic_ff RUNTIME DESTINATION bin)
