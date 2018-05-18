cmake_minimum_required (VERSION 2.8.11)

project (jobless)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 14)

if (${USE_OPENGL_ES})
    message("Setting USE_OPENGL_ES: " ${USE_OPENGL_ES})
    add_definitions(-DUSE_OPENGL_ES=${USE_OPENGL_ES})
endif()

find_package (GLEW REQUIRED)
find_package (SDL2 REQUIRED)
find_package (OpenGL REQUIRED)
find_package (SDL2TTF REQUIRED)

include_directories (${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} "D:/Libraries/SDL2_ttf-2.0.14" "D:/Libraries/glm" "${CMAKE_SOURCE_DIR}/include")

message("test inglibs: " ${GLEW_LIBRARY})
message("test inglibs: " ${SDL2_LIBRARIES})

subdirs(src)
