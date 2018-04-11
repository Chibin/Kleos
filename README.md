# Prerequisites
SDL2
SDL2 ttf
glm
GLEW

## Debugging OpenGL
Currently using RenderDoc https://renderdoc.org

## Using cl

My build is a simple bat file that uses `cl`

your call might slightly be different depending on your visual studio version:
`call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64`

# ENV
Not really part of the build.
Just things that helps me with the work flow.

`https://github.com/jesse23/with`


# Misc Links
https://www.mkyong.com/linux/grep-for-windows-findstr-example/


# Misc ENV

aliased:
`DOSKEY gvim="C:\Program Files (x86)\Vim\vim74\gvim.exe" -- $1`

# Debian
apt-get install libsdl2-dev
apt-get install glm-devel
apt-get install glm
apt-get install libsdl2-dev

I mnaully built sdl2-ttf and glm

# Fedora
dnf install mesa-libGL-devel
dnf install glew-devel
dnf install SDL2_ttf-devel
dnf install SDL_ttf-devel
dnf install glm-devel

``` clang -std=c++14  main.cpp /usr/lib64/libGLEW.a -lSDL2 -lSDL2_ttf -lm -ldl -lGL -lstdc++ ```
