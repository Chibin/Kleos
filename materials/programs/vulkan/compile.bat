glslangValidator.exe -v test.vert
glslangValidator.exe -v test.frag

glslc.exe test.vert -o test-vert.spv

glslc.exe test.frag -o test-frag.spv
