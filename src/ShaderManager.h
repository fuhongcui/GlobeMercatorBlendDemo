#pragma once
#include "glad/glad.h"

class ShaderManager {
public:
    static const char* getVertexShaderSource();
    static const char* getFragmentShaderSource();
    static GLuint compileShader(GLenum type, const char* source);
    static GLuint createProgram();
};
