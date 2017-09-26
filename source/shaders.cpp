#include "shaders.hpp"

GLchar* Shaders::fileToBuff(const char *filename) {

  
    FILE *fptr;
    long length;
    char *buf;

    fptr = fopen(filename, "rb");
    if(!fptr) return NULL;

    fseek(fptr, 0, SEEK_END);

    length = ftell(fptr);
    buf = (GLchar *)malloc(length+1); //length + null terminator

    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);

    buf[length] = 0; //terminate with null

    return buf;
}

GLuint Shaders::programFromFiles(const char *vertFilename, const char *fragFilename) {

  
  GLchar *vsc = fileToBuff(vertFilename);
  GLchar *fsc = fileToBuff(fragFilename);

  GLuint prog = programFromSource(vsc, fsc);

  //allocated in fileToBuff
  free(vsc);
  free(fsc);

  return prog; //can be NULL if it failed
}

GLuint Shaders::shaderFromSource(const GLchar *shaderSrc, GLenum shaderType) {

  
  GLuint shader = glCreateShader(shaderType);

  char *infoLog;
  
  int compiled, maxLength;
  
  // GL detects null terminator, length parameter can be 0
  glShaderSource(shader, 1, (const GLchar**)&shaderSrc, 0);

  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  //dump compile error to console
  if(compiled == false) {
    
     glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

     infoLog = (char *)malloc(maxLength); //maxLength includes null

     glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

     std::cout << infoLog << "\n";
     
     free(infoLog);

     glDeleteShader(shader);

     return 0;
  }

  return shader;
}

//deletes the provided shaders
GLuint Shaders::programFromShaders(GLuint vertShader, GLuint fragShader) {

  
  GLuint shaderProg = glCreateProgram();

  int isLinked, maxLength;

  char *infoLog;

  //Attach compiled shaders
  glAttachShader(shaderProg, vertShader);
  glAttachShader(shaderProg, fragShader);

  //attribute locations must be setup before calling glLinkProgram
  //gotta standardize these, add timer etc, use a map?
  glBindAttribLocation(shaderProg, 0, "aPosition");

  glLinkProgram(shaderProg);

  // If it fails, it would mean either there is a mismatch between the vertex
  // and fragment shaders. It might be that you have surpassed your GPU's abilities.
  // Perhaps too many ALU operations or too many texel fetch instructions
  // or too many interpolators or dynamic loops

  glGetProgramiv(shaderProg, GL_LINK_STATUS, (int *)&isLinked);

  //dump linking error to console
  if(isLinked == false) {
    
    //it's not glGetShaderiv
     glGetProgramiv(shaderProg, GL_INFO_LOG_LENGTH, &maxLength);

     infoLog = (char *)malloc(maxLength);

     //it's not glGetShaderInfoLog
     glGetProgramInfoLog(shaderProg, maxLength, &maxLength, infoLog);

     std::cout << infoLog << "\n";

     free(infoLog);

     return 0;
  }

  return shaderProg;
}

// compiles and links from GLchar* sources
// - must decide on standard attributes to bind (position, color, time, etc)
GLuint Shaders::programFromSource(const GLchar *vertsrc, const GLchar *fragsrc) {
  

  GLuint vertShader, fragShader, shaderProg;

  vertShader = shaderFromSource(vertsrc, GL_VERTEX_SHADER);

  if(vertShader == 0)  return 0;


  fragShader = shaderFromSource(fragsrc, GL_FRAGMENT_SHADER);

  if(fragShader == 0) {
    
     glDeleteShader(vertShader);
     
     return 0;
  }

  shaderProg = programFromShaders(vertShader, fragShader);
  
  if(shaderProg == 0) return 0;

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
  
  return shaderProg;
}
