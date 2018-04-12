cmake_minimum_required (VERSION 2.8.11)
project (jobless)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 14)

set(CMAKE_PREFIX_PATH "D:/Libraries/glew-1.13.0/include")
set(CMAKE_LIBRARY_PATH "D:/Libraries/glew-1.13.0/lib/Release/x64")

find_package (GLEW REQUIRED)

set(CMAKE_PREFIX_PATH "D:/Libraries/SDL2-2.0.7/include")
set(CMAKE_LIBRARY_PATH "D:/Libraries/SDL2-2.0.7/VisualC/x64/Release")
find_package (SDL2 REQUIRED)
find_package (OpenGL REQUIRED)

set(CMAKE_PREFIX_PATH "D:/Libraries/SDL2_ttf-2.0.14")
set(CMAKE_LIBRARY_PATH "D:/Libraries/SDL2_ttf-2.0.14/VisualC/x64/Debug")
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
    win32_platform.cpp
    )

target_link_libraries (main
    ${GLEW_LIBRARY}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2TTF_LIBRARY}
    )

