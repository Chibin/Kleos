#include "shaders.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

std::string FileToString(const char *filePath)
{
    std::string shaderCode;
    std::ifstream shaderStream(filePath, std::ios::in);
    if (shaderStream.is_open())
    {
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderCode = sstr.str();
        shaderStream.close();
    }

    return shaderCode;
}

/* return the programID of the linked vertex and fragment shaders */
GLuint CreateProgram(const char *vertex_file_path,
                     const char *fragment_file_path)
{

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    VertexShaderCode = FileToString(vertex_file_path);
    if (VertexShaderCode.empty())
    {
        printf("Impossible to open %s. "
               "Are you in the right directory? "
               "Don't forget to read the FAQ !\n",
               vertex_file_path);
        getchar();
        return 0;
    }

#if USE_OPENGL_ES
    VertexShaderCode = std::regex_replace(VertexShaderCode,
		    std::regex("#version 330 core"),
		    "#version 300 es");
#endif

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    FragmentShaderCode = FileToString(fragment_file_path);
    if (FragmentShaderCode.empty())
    {
        printf("Impossible to open %s. "
               "Are you in the right directory? "
               "Don't forget to read the FAQ !\n",
               fragment_file_path);
        getchar();
        return 0;
    }

#if USE_OPENGL_ES
    FragmentShaderCode = std::regex_replace(FragmentShaderCode,
		    std::regex("#version 330 core"),
		    "#version 300 es");
#endif

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr,
                           &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr,
                           &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    GLuint ProgramID = glCreateProgram();

    // Link the program
    printf("Linking program\n");
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr,
                            &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    /* clean up
     * Note: calling deleteshader without detaching will not actually delete
     * it.
     */
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
