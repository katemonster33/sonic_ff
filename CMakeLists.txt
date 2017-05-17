cmake_minimum_required(VERSION 2.6)
project(sonic_ff)

add_executable(sonic_ff SpriteSheet.cpp main.cpp)

install(TARGETS sonic_ff RUNTIME DESTINATION bin)

target_link_libraries(sonic_ff SDL2 SDL2_image)