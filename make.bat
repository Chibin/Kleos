@echo off
if NOT exist env.bat goto :file_not_found

set INCLUDE_VARS=GLEW_INCLUDE SDL2_INCLUDE SDL2_TTF_INCLUDE VULKAN_INCLUDE
set LIB_VARS=GLEW_LIB SDL2_LIB SDL2_TTF_LIB VULKAN_LIB
set ERROR_FOUND=0

rem "cleaning the variables"
for %%a in (%INCLUDE_VARS% %LIB_VARS%) do (
    set "%%a="
)

call env.bat

setlocal enabledelayedexpansion
for %%a in (%INCLUDE_VARS% %LIB_VARS%) do (
    if "!%%a!" == "" (
        echo ERROR:
        echo variable %%a not set.
        set ERROR_FOUND=1
    )
)
if %ERROR_FOUND% == 1 goto :error_found

cmake -G "Visual Studio 15 2017 Win64" ^
	-D CMAKE_PREFIX_PATH="%GLEW_INCLUDE%;%SDL2_INCLUDE%;%SDL2_TTF_INCLUDE%;%VULKAN_INCLUDE%" ^
	-D CMAKE_LIBRARY_PATH="%GLEW_LIB%;%SDL2_LIB%;%SDL2_TTF_LIB%;%VULKAN_LIB%" .
devenv src/main.vcxproj /build

if NOT %errorlevel% == 0 goto :error

copy src\Debug\main.* .
copy src\Debug\render.* .
goto :EOF

:error
echo Failed with error code '%errorlevel%.'
exit /b %errorlevel%

:error_found
exit /b 1

:file_not_found
echo ERROR:
echo env.bat file not found.
echo env.bat is used to set the INCLUDE_PATH and LIB_PATH
exit /b 1
