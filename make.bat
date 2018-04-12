cmake -G "Visual Studio 14 2015 Win64" -D CMAKE_PREFIX_PATH="D:/Libraries/glew-1.13.0/include;D:/Libraries/SDL2-2.0.7/include;D:/Libraries/SDL2_ttf-2.0.14" -D CMAKE_LIBRARY_PATH="D:/Libraries/glew-1.13.0/lib/Release/x64;D:/Libraries/SDL2-2.0.7/VisualC/x64/Release;D:/Libraries/SDL2_ttf-2.0.14/VisualC/x64/Debug" .
devenv main.vcxproj /build

if NOT %errorlevel% == 0 goto :error

copy Debug\main.* .
goto :EOF

:error
echo Failed with error code '%errorlevel%.'
exit /b %errorlevel%
