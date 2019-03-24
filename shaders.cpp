// #include "glm/ext.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/transform.hpp>

#include <fstream>
#include <sstream>
#include <vector>

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path) {
  GLuint vsid = glCreateShader(GL_VERTEX_SHADER);
  GLuint fsid = glCreateShader(GL_FRAGMENT_SHADER);

  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if (VertexShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << VertexShaderStream.rdbuf();
    VertexShaderCode = sstr.str();
    VertexShaderStream.close();
  } else {
    printf("Failed to open %s. Are you in the right directory ? ", vertex_file_path);
    return 0;
  }

  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if (FragmentShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << FragmentShaderStream.rdbuf();
    FragmentShaderCode = sstr.str();
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  char const *VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(vsid, 1, &VertexSourcePointer, NULL);
  glCompileShader(vsid);

  glGetShaderiv(vsid, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(vsid, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(vsid, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", &VertexShaderErrorMessage[0]);
  }

  char const *FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(fsid, 1, &FragmentSourcePointer, NULL);
  glCompileShader(fsid);

  glGetShaderiv(fsid, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(fsid, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(fsid, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    printf("%s\n", &FragmentShaderErrorMessage[0]);
  }

  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, vsid);
  glAttachShader(ProgramID, fsid);
  glLinkProgram(ProgramID);

  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    printf("%s\n", &ProgramErrorMessage[0]);
  }

  glDetachShader(ProgramID, vsid);
  glDetachShader(ProgramID, fsid);
  glDeleteShader(vsid);
  glDeleteShader(fsid);
  return ProgramID;
}