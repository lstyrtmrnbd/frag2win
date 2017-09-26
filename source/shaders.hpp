#ifndef _SHADERS_HPP_
#define _SHADERS_HPP_

//Adapted from khronos.org opengl wiki tut2

#include <iostream>

#include <GL/glew.h> // necessary??
#include <GL/gl.h>


namespace Shaders {

  GLchar* fileToBuff(const char *filename);
  GLuint programFromFiles(const char *vertFilename, const char *fragFilename);
  GLuint shaderFromSource(const GLchar *shaderSrc, GLenum shaderType);
  GLuint programFromShaders(GLuint vertShader, GLuint fragShader);
  GLuint programFromSource(const GLchar *vertsrc, const GLchar *fragsrc);
}

#endif
