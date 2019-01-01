set SDL2=C:/Libraries/SDL2-2.0.8
set GLEW=C:/Libraries/glew-2.1.0
set SDL2_TTF=C:/Libraries/SDL2_ttf-2.0.14
set VULKAN=C:/VulkanSDK/1.1.92.1
REM set VULKAN=C:/VulkanSDK/1.1.77.0

set SDL2_INCLUDE=%SDL2%/include
set GLEW_INCLUDE=%GLEW%/include
set SDL2_TTF_INCLUDE=%SDL2_TTF%/include
set VULKAN_INCLUDE=%VULKAN%/include
set GLM_INCLUDE=C:/Libraries/glm

set SDL2_LIB=%SDL2%/VisualC//x64/Debug
set GLEW_LIB=%GLEW%/lib/Debug
set SDL2_TTF_LIB=%SDL2_TTF%/lib/x64
set VULKAN_LIB=%VULKAN%/Lib

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
