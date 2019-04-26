@echo off

call setter.bat
if NOT %errorlevel% == 0 exit /b %errorlevel%

SET ORIGINAL_DIR=%cd%
SET LIBS=kernel32.lib User32.lib Winmm.lib Gdi32.lib OpenGL32.lib glu32.lib 
SET LIBS=%LIBS% glew32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib


SET INCLUDE_PATH=/I %GLEW_INCLUDE% /I %GLM_INCLUDE%
SET INCLUDE_PATH=/I %SDL2_INCLUDE% %INCLUDE_PATH%
SET INCLUDE_PATH=/I %SDL2_TTF_INCLUDE% %INCLUDE_PATH%
SET INCLUDE_PATH=/I %VULKAN_INCLUDE% %INCLUDE_PATH%

SET LIB_PATH=/LIBPATH:%GLEW_LIB%
SET LIB_PATH=/LIBPATH:%SDL2_LIB% %LIB_PATH%
SET LIB_PATH=/LIBPATH:%SDL2_TTF_LIB% %LIB_PATH%
SET LIB_PATH=/LIBPATH:%VULKAN_LIB% %LIB_PATH%

if "%1" == "" (
    goto :top
) ELSE (
    goto :update_and_render
)

:top
SET FILES=src\main.cpp

REM TODO: we'll need to port over glew32.dll and SDL2.dll SDL2_ttf.dll

rem mkdir ..\..\build_game
rem pushd ..\..\build_game

cl /DWIN32#1 /Zi /EHsc -GS -Gs9999999 %INCLUDE_PATH% %FILES% /link-HEAP:4294967296 /SUBSYSTEM:CONSOLE %LIB_PATH% %LIBS%
if NOT %errorlevel% == 0 goto :error


:update_and_render
REM build render as a dll
set COMPILER_FLAGS=-O2 -MTd -GR- -EHa- -Zo -Oi -WX -W4 -FC -Z7 -GS -Gs9999999 /EHsc
set LINKER_FLAGS=-HEAP:4294967296 -incremental:no -opt:ref User32.lib Gdi32.lib winmm.lib Opengl32.lib glew32.lib glu32.lib SDL2main.lib SDL2_ttf.lib SDL2.lib
SET INCLUDE_PATH=/I %GLEW_INCLUDE% /I %GLM_INCLUDE% /I .\include
SET INCLUDE_PATH=/I %SDL2_INCLUDE% %INCLUDE_PATH%
SET INCLUDE_PATH=/I %SDL2_TTF_INCLUDE% /I %VULKAN_INCLUDE% %INCLUDE_PATH%

cl /DWIN32#1 %COMPILER_FLAGS% -I..\iaca-win64\ %INCLUDE_PATH% src/render.cpp -Fmrender.map /LD /link %LIB_PATH% %LINKER_FLAGS% -incremental:no -opt:ref -PDB:kleos_%random%.pdb -EXPORT:Render -EXPORT:UpdateAndRender -OUT:render1.dll

copy render1.dll render.dll

rem copy %ORIGINAL_DIR%\*.glsl .
rem copy %ORIGINAL_DIR%\*.vs .
rem copy %ORIGINAL_DIR%\*.fs .
rem copy %ORIGINAL_DIR%\*.DDS .
rem popd
goto :EOF

:error
echo Failed with error code '%errorlevel%.'
exit /b %errorlevel%
