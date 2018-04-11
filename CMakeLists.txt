cmake_minimum_required (VERSION 2.8.11)
project (jobless)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 14)

find_package (GLEW REQUIRED)
find_package (SDL2 REQUIRED)
find_package (OpenGL REQUIRED)
find_package (SDL2TTF REQUIRED)

include_directories (${SDL2_INCLUDE_DIRS})

add_executable (main
    main.cpp
    camera.cpp
    collision.cpp
    entity.cpp
    entity_manager.cpp
    font.cpp
    game.cpp
    game_time.cpp
    input.cpp
    linux_platform.cpp
    opengl.cpp
    rect_manager.cpp
    rectangle.cpp
    shaders.cpp
    )

target_link_libraries (main
    ${GLEW_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2TTF_LIBRARY}
    dl
    m
    )
