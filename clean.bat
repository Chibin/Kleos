DEL *.pdb
DEL CMakeCache.txt
DEL main.vcxproj* main.pdb main.lib main.exp main.ilk main.exe
DEL render.vcxproj* render.pdb render.lib render.exp render.ilk render.map render.dll render.obj
RMDIR /s /Q main.dir render.dir
cd src
DEL main.vcxproj* main.pdb main.lib main.exp main.ilk main.exe
DEL render.vcxproj* render.pdb render.lib render.exp render.ilk render.map render.dll render.obj
RMDIR /s /Q main.dir render.dir
cd ..
