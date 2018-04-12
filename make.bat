devenv main.vcxproj /build

if NOT %errorlevel% == 0 goto :error

copy Debug\main.* .
goto :EOF

:error
echo Failed with error code '%errorlevel%.'
exit /b %errorlevel%
