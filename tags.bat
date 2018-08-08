@echo off
REM build everything if we have an argument
if "%1" == "" (
    SET command=ctags -R .
) ELSE (
SET command=ctags -R . D:\Libraries\glew-1.13.0\src D:\Libraries\glew-1.13.0\include D:\Libraries\SDL2-2.0.7\src D:\Libraries\SDL2-2.0.7\include D:\VulkanSDK\1.1.77.0
)

ECHO running: %command%
%command%
