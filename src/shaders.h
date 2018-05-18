#pragma once
#include <GL/glew.h>
#include <string>

std::string FileToString(const char *filePath);
GLuint CreateProgram(const char *vertex_file_path,
                     const char *fragment_file_path);
