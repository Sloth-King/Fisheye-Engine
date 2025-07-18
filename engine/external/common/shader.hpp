#ifndef SHADER_HPP
#define SHADER_HPP

GLuint loadShadersFromFileGLSL(const char * vertex_file_path,const char * fragment_file_path);

GLuint loadShaders(std::string VertexShaderCode, std::string FragmentShaderCode);

#endif
