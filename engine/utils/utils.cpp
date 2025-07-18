#include <cstdarg>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <chrono>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

#include "utils.h"

void Utils::printShaderData(unsigned int ProgramID){
    int count = 0;
	glGetProgramiv(ProgramID, GL_ACTIVE_UNIFORMS, &count);
	printf("Active Uniforms: %d\n", count);

	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	const GLsizei bufSize = 16; // maximum name length
	GLchar name[bufSize]; // variable name in GLSL
	GLsizei length; // name length

    GLint loc = 0;

	for (int i = 0; i < count; i++)
	{
		glGetActiveUniform(ProgramID, (GLuint)i, bufSize, &length, &size, &type, name);

        loc = glGetUniformLocation(ProgramID, name);

		printf("Uniform #%d   Location: %d Type: %u Name: %s\n", i, loc, type, name);
	}
}

bool Utils::replaceInString(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}