/* Draw GLSL fragment shader to window*/

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

#include <string.h>
#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>

#include "shaders.hpp"

const unsigned int width = 1280;
const unsigned int height = 720;

//GL
GLFWwindow *window;

const char *vertFilename = "default.vert";
const char *fragFilename = "default.frag";

GLchar *vsc, *fsc;
GLuint vertShader, fragShader, defaultProg, VBO, VAO;
GLuint tex0;

const float quad[] = { 1.0f, 1.0f, 0.0f,  1.0f, -1.0f, 0.0f,  -1.0f, 1.0f, 0.0f,
	        1.0f,-1.0f, 0.0f, -1.0f, -1.0f, 0.0f,  -1.0f, 1.0f, 0.0f };

//uniform names and locations, locations need regrabbed after recompile
const char* timeHandle = "time";
GLint timeLoc;

const char* resolutionHandle = "resolution";
GLint resolutionLoc;

//directory watch and compile
DWORD dwWaitStatus;
HANDLE dwChangeHandle;

bool dumpInfo = true;                    //should error info be dumped

std::atomic_bool dirChange(false);       //change in directory was seen

std::atomic_bool cancelWatch(false);     //used to break from watch thread


//change frag source and compile without creating or deleting shader handle
//link and return true on success
bool compileNewFrag(GLuint program) {

  free(fsc);
  fsc = Shaders::fileToBuff(fragFilename);

  char *info;
  int success, maxLength;

  //glDetachShader(program, fragShader);
  //glDeleteShader(fragShader);

  //fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  
  glShaderSource(fragShader, 1, (const GLchar**)&fsc, 0);
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);

  if(!success) {

    if(dumpInfo) {
      
      glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
      info = (char *)malloc(maxLength);
      glGetShaderInfoLog(fragShader, maxLength, &maxLength, info);

      std::cout << info << "\n";

      free(info);

      dumpInfo = false;
    }

    return false;
  }

  //attributes go here
  glBindAttribLocation(program, 0, "aPosition");

  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, (int *)&success);

  if(!success) {

    if(dumpInfo) {
      
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
      info = (char *)malloc(maxLength);
      glGetProgramInfoLog(program, maxLength, &maxLength, info);

      std::cout << info << "\n";

      free(info);

      dumpInfo = false;
    }

    return false;
  }
  
  return true;
}

void watchDirectory(LPSTR watchPath) {

  
  dwChangeHandle = FindFirstChangeNotification(watchPath, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

  if(dwChangeHandle == INVALID_HANDLE_VALUE) {
    
    std::cout << "FindFirstChangeNotification function failed" << "\n";
    ExitProcess(GetLastError());
    
  } else if(dwChangeHandle == NULL) {

    std::cout << "Unexpected NULL from Find FirstChangeNotification" << "\n";
    ExitProcess(GetLastError());
  }

  while(!cancelWatch) {

    dwWaitStatus = WaitForSingleObject(dwChangeHandle, 5000); //wait on file written signal

    switch (dwWaitStatus) {

      case WAIT_OBJECT_0:

	//change witnessed
	dirChange = true;

	if(FindNextChangeNotification(dwChangeHandle) == FALSE) {

	  std::cout << "Error: FindNextChangeNotification function failed" << "\n";
	  ExitProcess(GetLastError());
	}
	
	break;

      case WAIT_TIMEOUT:

	//timeout is necessary to check if cancelWatch was set and thread needs to die
	break;
	
      default:

	std::cout << "Unhandled dwWaitStatus" << "\n";
	ExitProcess(GetLastError());
	break;
    } 
  }
}

int initOGL() {

  
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(width, height, "frag2win", NULL, NULL);
  if(window == NULL) {

    std::cout << "Failed to create GLFW window" << "\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  //pop glew
  GLenum err = glewInit();
  if (GLEW_OK != err) {

    std::cout << "Error: " << glewGetErrorString(err) << "\n";
  }
  
  std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << "\n";

  //GL Start
  glViewport(0, 0, width, height);

  //shaders
  vsc = Shaders::fileToBuff(vertFilename);
  fsc = Shaders::fileToBuff(fragFilename);

  vertShader = Shaders::shaderFromSource(vsc, GL_VERTEX_SHADER);
  fragShader = Shaders::shaderFromSource(fsc, GL_FRAGMENT_SHADER);
  
  defaultProg = Shaders::programFromShaders(vertShader, fragShader);

  //shaders will dump error info, just exit
  if(defaultProg == 0) return -1;

  //cleanup
  free(vsc);

  //prepare geometry
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  
  glUseProgram(defaultProg);

  //prepare textures
  glGenTextures(1, &tex0);

  glBindTexture(GL_TEXTURE_2D, tex0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //load image into texture
  int imageW, imageH;
  unsigned char* image = SOIL_load_image("noise.png", &imageW, &imageH, 0, SOIL_LOAD_RGB);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageW, imageH, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

  SOIL_free_image_data(image);

  return 0;
}

int main() {

  auto t_start = std::chrono::high_resolution_clock::now();
  
  int initStatus = initOGL();

  if(initStatus < 0) {

    std::cout << "OpenGL initialization failed" << "\n";
    return -1;
  }

  char pathBuffer[MAX_PATH];
  char trimmedPath[MAX_PATH];
  char drive[4];
  
  GetModuleFileName(NULL, pathBuffer, MAX_PATH);

  _splitpath_s(pathBuffer, drive, 4, trimmedPath, MAX_PATH, NULL, 0, NULL, 0);

  strcat(drive, trimmedPath);

  std::cout << "Current dir: " << drive << "\n";

  std::thread watch(watchDirectory, drive);  //directory watch logic
  
  //Render Loop
  while(!glfwWindowShouldClose(window)) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {

      glfwSetWindowShouldClose(window, true);
    }

    bool expected = true;
    
    if(dirChange.compare_exchange_strong(expected, false)) {
      
      bool compiled = compileNewFrag(defaultProg);

      if(compiled) {

        compiled = false;
	
        std::cout << "Fragment shader succesfully recompiled" << "\n";
	
        timeLoc = glGetUniformLocation(defaultProg, timeHandle);
        resolutionLoc = glGetUniformLocation(defaultProg, resolutionHandle);
	
        glUseProgram(defaultProg);
        dumpInfo = true;
      }
    }

    //get time
    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
    
    //fill uniforms if they exist
    if(timeLoc != -1) glUniform1f(timeLoc, time);
    
    if(resolutionLoc != -1) glUniform2f(resolutionLoc, (float)width, (float)height);

    //draw
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  cancelWatch = true;
  watch.join();
  
  glDeleteBuffers(1, &VBO);
  
  glfwTerminate();
  return 0;
}
