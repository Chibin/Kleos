call setter.bat
if NOT %errorlevel% == 0 exit /b %errorlevel%

cmake -G "Visual Studio 15 2017 Win64" ^
	-D CMAKE_PREFIX_PATH="%GLEW_INCLUDE%;%SDL2_INCLUDE%;%SDL2_TTF_INCLUDE%;%VULKAN_INCLUDE%" ^
	-D CMAKE_LIBRARY_PATH="%GLEW_LIB%;%SDL2_LIB%;%SDL2_TTF_LIB%;%VULKAN_LIB%" .
devenv src/main.vcxproj /build

if NOT %errorlevel% == 0 goto :error

copy src\Debug\main.* .
copy src\Debug\update_and_render.* .
goto :EOF

:error
echo Failed with error code '%errorlevel%.'
exit /b %errorlevel%

:file_not_found
echo ERROR:
echo env.bat file not found.
echo env.bat is used to set the INCLUDE_PATH and LIB_PATH
exit /b 1
