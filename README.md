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

# Troubleshooting

## Vulkan
The new steam chat update broke GetSwapchainImagesKHR.
You'll get a crash `Exception thrown at 0x* (SteamOverlayVulkanLayer64.dll)`
You can prevent the crash by disabling the layer.
Add 'DISABLE_VK_LAYER_VALVE_steam_overlay_1=1' as an environment variable.

# Misc Links
https://www.mkyong.com/linux/grep-for-windows-findstr-example/


# Misc ENV

aliased:
`DOSKEY gvim="C:\Program Files (x86)\Vim\vim74\gvim.exe" -- $1`

Troubleshooting:
nvim looks for python2 and python binaries. I created the
respected python2 and python3 files by copying the python.exe.
https://www.reddit.com/r/neovim/comments/7l0757/neovim_installation_in_windows_10/

To easily generate compile_commands.json on Windows, use ninja for the cmake generator.
`cmake -G "Ninja" ..."

# Debian
apt-get install libsdl2-dev 
apt-get install libglm-dev

I manually built sdl2, sdl2-ttf, and glm as well.

```
clang -std=c++14  main.cpp /usr/lib64/libGLEW.a -lSDL2 -lSDL2_ttf -lm -ldl -lGL -lstdc++

or

clang++ -std=c++14 main.cpp -lGLEW -lSDL2 -lSDL2_ttf -lm -ldl -lGL -lstdc++ -lGLU
```

building render.cpp <needs to be renamed>:

```
clang -shared -fPIC -std=c++14 render.cpp -lGLEW  -lSDL2 -lSDL2_ttf -lm -ldl -lGL -lstdc++ -lGLU -o render.so
```

# Fedora
dnf install mesa-libGL-devel
dnf install glew-devel
dnf install SDL2_ttf-devel
dnf install SDL_ttf-devel
dnf install glm-devel

``` clang -std=c++14  main.cpp /usr/lib64/libGLEW.a -lSDL2 -lSDL2_ttf -lm -ldl -lGL -lstdc++ ```


# Docker build
```
docker build -t jobless:testing .
./run.sh
```

