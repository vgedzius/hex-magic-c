cmake_minimum_required(VERSION 3.5.0)
project(hex-magic VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/sdl2)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_gfx REQUIRED)

add_compile_options(-Wall -Werror -Wno-unused-but-set-variable)
add_compile_definitions(OLD_RENDER=1)

add_executable(hex-magic ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)

target_sources(hex-magic 
    PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/src/hex_platform.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hex_platform.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hex.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hex.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/math.h
)

add_custom_target(copy-assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)

add_dependencies(hex-magic copy-assets)

target_include_directories(hex-magic PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_GFX_INCLUDE_DIRS})
target_link_libraries(hex-magic ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_GFX_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
