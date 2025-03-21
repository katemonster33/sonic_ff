# CMakeList.txt : CMake project for sonic_ff, include source and define
# project specific logic here.
#

#add_subdirectory(tmxparser/thirdparty/tinyxml2)
#add_subdirectory(tmxparser)

cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("sonic_ff")

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(cjson REQUIRED)

add_subdirectory(tmxlite-json/tmxlite)

# Add source to this project's executable.
add_executable (sonic_ff "main.cpp" "Actor.cpp" "GameWindow.cpp" "Texture.cpp" "MapLayer.cpp" "Geometry.h" "Geometry.cpp" "SpriteProvider.cpp" "TilesetConfig.h" "TilesetConfig.cpp")
target_include_directories(sonic_ff PUBLIC tileson/include)
target_link_libraries(sonic_ff PRIVATE cJson SDL2::SDL2 SDL2::SDL2main $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
if(LINUX)
  target_link_libraries(sonic_ff PRIVATE zstd)
endif (LINUX)
target_compile_definitions(sonic_ff PUBLIC -D_CRT_SECURE_NO_WARNINGS)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/)
     
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sonic_ff PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
