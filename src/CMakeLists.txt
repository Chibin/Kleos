add_executable (main
    main.cpp
    game.cpp
    opengl.cpp
    )

add_library(render SHARED
    render.cpp
    )

set_target_properties(
	render PROPERTIES
	ENABLE_EXPORTS 1
	WINDOWS_EXPORT_ALL_SYMBOLS 1
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

target_link_libraries (render
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

file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

file(COPY "${CMAKE_SOURCE_DIR}/materials" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
