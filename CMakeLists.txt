cmake_minimum_required(VERSION 3.5.0)
project(hex-magic VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(SDL2 REQUIRED)

add_compile_options(-Wall -Werror)
add_executable(hex-magic src/main.cpp)

target_include_directories(hex-magic PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(hex-magic ${SDL2_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
