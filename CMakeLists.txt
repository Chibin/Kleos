cmake_minimum_required (VERSION 2.8.11)
project (jobless)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 14)

find_package (GLEW REQUIRED)
find_package (SDL2 REQUIRED)
find_package (OpenGL REQUIRED)
find_package (SDL2TTF REQUIRED)

include_directories (${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} "D:/Libraries/SDL2_ttf-2.0.14" "D:/Libraries/glm")

message("test inglibs: " ${GLEW_LIBRARY})
message("test inglibs: " ${SDL2_LIBRARIES})

add_executable (main
    main.cpp
    game.cpp
    camera.cpp
    collision.cpp
    entity.cpp
    entity_manager.cpp
    font.cpp
    game_time.cpp
    input.cpp
    opengl.cpp
    rect_manager.cpp
    rectangle.cpp
    shaders.cpp
    )

if (UNIX)
    target_sources (main PUBLIC linux_platform.cpp)
elseif (WIN32)
    add_definitions(-DWIN32=1)
    target_sources(main PUBLIC win32_platform.cpp)
endif()

target_link_libraries (main
    ${GLEW_LIBRARY}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2TTF_LIBRARY}
    )

if (UNIX)
    target_link_libraries (main
        dl
        m
        )
endif()
