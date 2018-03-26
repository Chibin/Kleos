@echo off
if "%1" == "" (
    goto :top
) ELSE (
    goto :update_and_render
)

:top
SET ORIGINAL_DIR=%cd%
SET LIBS=kernel32.lib User32.lib Winmm.lib Gdi32.lib OpenGL32.lib glu32.lib 
SET LIBS=%LIBS% glew32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib

SET INCLUDE_PATH=/I D:\Libraries\glew-1.13.0\include /I D:\Libraries\glm
SET INCLUDE_PATH=/I D:\Libraries\SDL2-2.0.7\include %INCLUDE_PATH%
SET INCLUDE_PATH=/I D:\Libraries\SDL2_ttf-2.0.14 %INCLUDE_PATH%

SET LIB_PATH=/LIBPATH:D:\Libraries\glew-1.13.0\lib\Release\x64
SET LIB_PATH=/LIBPATH:D:\Libraries\SDL2-2.0.7\VisualC\x64\Release %LIB_PATH%
SET LIB_PATH=/LIBPATH:D:\Libraries\SDL2-2.0.7\VisualC\x64\Release %LIB_PATH%
SET LIB_PATH=/LIBPATH:D:\Libraries\SDL2_ttf-2.0.14\VisualC\x64\Debug %LIB_PATH%

SET FILES=..\game\main.cpp

REM TODO: we'll need to port over glew32.dll and SDL2.dll SDL2_ttf.dll

rem mkdir ..\..\build_game
rem pushd ..\..\build_game

cl /Zi /EHsc -Gm -GS -Gs9999999 %INCLUDE_PATH% %FILES% /link -HEAP:4294967296 /SUBSYSTEM:CONSOLE %LIB_PATH% %LIBS%
if NOT %errorlevel% == 0 goto :error


:update_and_render
REM build render as a dll
set COMPILER_FLAGS=-O2 -MTd -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -FC -Z7 -GS -Gs9999999 /EHsc
set LINKER_FLAGS=-HEAP:4294967296 -incremental:no -opt:ref User32.lib Gdi32.lib winmm.lib Opengl32.lib glew32.lib glu32.lib SDL2main.lib SDL2_ttf.lib SDL2.lib
SET INCLUDE_PATH=/I D:\Libraries\glew-1.13.0\include /I D:\Libraries\glm
SET INCLUDE_PATH=/I D:\Libraries\SDL2-2.0.7\include %INCLUDE_PATH%
SET INCLUDE_PATH=/I D:\Libraries\SDL2_ttf-2.0.14 %INCLUDE_PATH%

cl %COMPILER_FLAGS% -I..\iaca-win64\ %INCLUDE_PATH% render.cpp -Fmrender.map -LD /link %LIB_PATH% %LINKER_FLAGS% -incremental:no -opt:ref -PDB:game_%random%.pdb -EXPORT:Render -EXPORT:UpdateAndRender -OUT:render_new.dll 

copy render_new.dll render.dll


rem copy %ORIGINAL_DIR%\*.glsl .
rem copy %ORIGINAL_DIR%\*.vs .
rem copy %ORIGINAL_DIR%\*.fs .
rem copy %ORIGINAL_DIR%\*.DDS .
rem popd
goto :EOF

:error
echo Failed with error code '%errorlevel%.'
exit /b %errorlevel%
