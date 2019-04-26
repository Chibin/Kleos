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

exit /b 0

:error_found
exit /b 1
