@echo off

SET ORIGINAL_DIR=%cd%
SET LIBRARIES=kernel32.lib User32.lib Winmm.lib Gdi32.lib OpenGL32.lib glu32.lib glew32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib
SET INCLUDE_PATH=/I D:\Libraries\glew-1.13.0\include /I D:\Libraries\glm /I D:\Libraries\SDL2-2.0.7\include /I D:\Libraries\SDL2_ttf-2.0.14
SET LIBRARY_PATH=/LIBPATH:D:\Libraries\glew-1.13.0\lib\Release\x64 /LIBPATH:D:\Libraries\SDL2-2.0.7\VisualC\x64\Release /LIBPATH:D:\Libraries\SDL2-2.0.7\VisualC\x64\Release /LIBPATH:D:\Libraries\SDL2_ttf-2.0.14\VisualC\x64\Debug

SET FILES=..\game\main.cpp

REM TODO: we'll need to port over glew32.dll and SDL2.dll SDL2_ttf.dll

rem mkdir ..\..\build_game
rem pushd ..\..\build_game
cl /Zi /EHsc %INCLUDE_PATH% %FILES% /link /SUBSYSTEM:CONSOLE %LIBRARY_PATH% %LIBRARIES% 
rem copy %ORIGINAL_DIR%\*.glsl .
rem copy %ORIGINAL_DIR%\*.vs .
rem copy %ORIGINAL_DIR%\*.fs .
rem copy %ORIGINAL_DIR%\*.DDS .
rem popd
