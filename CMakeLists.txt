# CMakeList.txt : CMake project for sonic_ff, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("sonic_ff")

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image REQUIRED)

# Add source to this project's executable.
add_executable (sonic_ff "main.cpp" "SpriteSheet.cpp" "Actor.cpp" "Actor.h")
target_link_libraries(sonic_ff PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sprites/sonic3.png
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sprites/)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sonic_ff PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
