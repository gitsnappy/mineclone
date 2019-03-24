#include "glm/ext.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow *window;
#include "PerlinNoise.hpp"
#include "controls.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace glm;

// build-essential
// g++ libx11-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev
// libglm-dev
// libglfw3-dev
// libglew-dev

#define air 0
#define grass 1
#define stone 2
#define cobble 3
#define plank 4
#define logBlock 5
#define sand 6
#define water 7
#define leaf 8
#define dirt 9
#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))

void initCubeTexture(int type, GLfloat *cube, GLfloat cubes[][120]) {
  int blocks[] = {0,  0,  0,  0,  0,  0,  1, //
                  8,  0,  8,  16, 8,  8,  0, //
                  1,  1,  1,  1,  1,  1,  0, //
                  2,  2,  2,  2,  2,  2,  0, //
                  3,  3,  3,  3,  3,  3,  0, //
                  4,  12, 4,  12, 4,  4,  0, //
                  5,  5,  5,  5,  5,  5,  0, //
                  6,  6,  6,  6,  6,  6,  0, //
                  20, 20, 20, 20, 20, 20, 0, //
                  16, 16, 16, 16, 16, 16, 0};
  const int texturesPerRow = 8;
  const GLfloat scale = 1.0f / texturesPerRow;

  GLfloat texCube[48] = {};
  for (int i = 0; i < 6; i++) {
    GLfloat x1 = blocks[type * 7 + i] % texturesPerRow;
    GLfloat y2 = blocks[type * 7 + i] / texturesPerRow;
    GLfloat x2 = x1 + 1;
    GLfloat y1 = y2 + 1;

    y1 = texturesPerRow - y1;
    y2 = texturesPerRow - y2;

    x1 = x1 * scale;
    x2 = x2 * scale;
    y1 = y1 * scale;
    y2 = y2 * scale;

    texCube[i * 8] = x1;
    texCube[i * 8 + 1] = y1;
    texCube[i * 8 + 2] = x2;
    texCube[i * 8 + 3] = y1;
    texCube[i * 8 + 4] = x2;
    texCube[i * 8 + 5] = y2;
    texCube[i * 8 + 6] = x1;
    texCube[i * 8 + 7] = y2;
  }

  for (int i = 0; i < 24; i++) {
    cubes[type][i * 5 + 3] = texCube[i * 2];
    cubes[type][i * 5 + 4] = texCube[i * 2 + 1];
    // std::cout << cube[i*5+3] << ", " <<  cube[i*5+4] << std::endl;
  }
}

void pourWater(int cubeTypes[], int rows, int searchStart, int searchEnd) {

  int lowest = -99;
  for (int x = 0; x < rows; x++) {
    for (int z = 0; z < rows; z++) {
      for (int y = 0; y < searchEnd; y++) {
        int index = y * rows * rows + x * rows + z;
        if (cubeTypes[index] == grass){
          lowest = y ;// +1
        }

      }
    }
  }

  for (int x = 0; x < rows; x++) {
    for (int z = 0; z < rows; z++) {
      for (int y = searchStart; y < searchEnd; y++) {

        int index = y * rows * rows + x * rows + z;
        if (cubeTypes[index] == air && y < lowest) {
          cubeTypes[index] = water;
        }
      }
    }
  }
}

void setCubeTypes(int strataStart, int strataEnd, int cubeTypes[], int rows, int type) {
  const int square = rows * rows;

  // for rows below this, if empty, fill with this type
  for (int i = 0; i < square * strataStart; i++) {
    if ((cubeTypes[i] == air) && (type != grass)) { // final layers
      cubeTypes[i] = type;
    }
  }

  int seed = 237;
  PerlinNoise pn(seed);
  int strata = strataEnd - strataStart;
  for (int x = 0; x < rows; x++) {
    for (int z = 0; z < rows; z++) {
      GLfloat f = 1.0f;
      // cubePositions[x * rows + z] = glm::vec3(f * x * 2, 0.0f, f * z * 2);
      if (type == grass) {
        for (int y = 0; y < strataEnd; y++) {
          int index = y * square + x * rows + z;
          if (cubeTypes[index] == air) {
            cubeTypes[index] = type;
            break;
          }
        }

      } else {

        double relX = x * type * 1.04 / (double)rows; // 1.04 random numbers for variety
        double relZ = z * type * 1.04 / (double)rows;
        double relY = pn.noise(relX, relZ, 0);

        int pnRange = 1 - 0;
        int heightRange = strataEnd - strataStart;
        int height = (int)ceil(((relY - 0) * heightRange / pnRange) + strataStart);

        for (int y = strataStart; y < height; y++) {
          // fill in all blocks for this XZ coordinate in cubeTypes;
          int index = y * square + x * rows + z;
          if (cubeTypes[index] == air) {
            cubeTypes[index] = type;
          }
          // std::cout << cubeTypes[y * square + x * rows + z] << std::endl;
        }
      }
    }
  }
}

void drawType(int type, int rows, glm::vec3 *cubePositions, int strata, int cubeTypes[], GLuint matrixId, glm::mat4 projView, GLfloat cubes[][120]) {

  glBufferData(GL_ARRAY_BUFFER, sizeof(cubes[type]), cubes[type], GL_STATIC_DRAW);
  int square = rows * rows;
  for (int x = 0; x < rows; x++) {
    for (int z = 0; z < rows; z++) {
      for (int y = 0; y < strata; y++) {
        int index = y * square + x * rows + z;
        bool optimize = (x == 0) || (z == 0) || (x == rows-1) || (z == rows-1) || 
                      cubeTypes[index-1] == air || cubeTypes[index+1] == air || cubeTypes[index -rows] == air ||
                      cubeTypes[index+rows] == air ||
                      cubeTypes[index + rows*rows] == air ||
                      cubeTypes[index - rows*rows] == air ;

        if (cubeTypes[index] == type && optimize) {
          glm::mat4 model = glm::mat4(1.0f);
          model = glm::translate(model, cubePositions[index]);
          glm::mat4 mvp = projView * model;
          glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mvp[0][0]);
          glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
      }
    }
  }
}

int main() {
  glewExperimental = true; // Needed for core profile
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }
  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1024, 768, "Mineclone", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to open GLFW window. Intel GPU is not 3.3 compatible. Try 2.1 \n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  GLuint programID = LoadShaders("src/vs.vs", "src/fs.vs");
  glEnable(GL_CULL_FACE);
  glEnable(GL_MULTISAMPLE);
  // glEnable(GL_LINE_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glDepthFunc(GL_LESS);

  // load 2d texture
  // unsigned char *data = new unsigned char [imageSize];
  GLuint blockTexId = loadImage("src/assets/blocks.png");

  glUseProgram(programID);
  GLuint matrixId = glGetUniformLocation(programID, "MVP");

  // front top back bottom left right, transparency

  const int numTextures = 10;

  GLfloat cubes[numTextures][120];
  GLfloat cube[] = {
      // front
      -1.0, -1.0, 1.0, 0.0f, 0.875f,  //
      1.0, -1.0, 1.0, 0.125f, 0.875f, //
      1.0, 1.0, 1.0, 0.125f, 1.0f,    //
      -1.0, 1.0, 1.0, 0.0f, 1.0f,     //
      // top
      -1.0, 1.0, 1.0, 0.0f, 0.0f,  //
      1.0, 1.0, 1.0, 0.0f, 0.0f,   //
      1.0, 1.0, -1.0, 0.0f, 0.0f,  //
      -1.0, 1.0, -1.0, 0.0f, 0.0f, // 0.0f,1.0f,//
      // back
      1.0, -1.0, -1.0, 0.0f, 0.0f,  //
      -1.0, -1.0, -1.0, 0.0f, 0.0f, //
      -1.0, 1.0, -1.0, 0.0f, 0.0f,  //
      1.0, 1.0, -1.0, 0.0f, 0.0f,   //
      // bottom
      -1.0, -1.0, -1.0, 0.0f, 0.0f, //
      1.0, -1.0, -1.0, 0.0f, 0.0f,  //
      1.0, -1.0, 1.0, 0.0f, 0.0f,   //
      -1.0, -1.0, 1.0, 0.0f, 0.0f,  //
      // left
      -1.0, -1.0, -1.0, 0.0f, 0.0f, ////
      -1.0, -1.0, 1.0, 0.0f, 0.0f,  //
      -1.0, 1.0, 1.0, 0.0f, 0.0f,   //
      -1.0, 1.0, -1.0, 0.0f, 0.0f,  //
      // right
      1.0, -1.0, 1.0, 0.0f, 0.0f,  //
      1.0, -1.0, -1.0, 0.0f, 0.0f, //
      1.0, 1.0, -1.0, 0.0f, 0.0f,  //
      1.0, 1.0, 1.0, 0.0f, 0.0f,   //
  };

  GLint indices[] = {
      // front
      0, 1, 2,   //
      2, 3, 0,   //
                 //  top
      4, 5, 6,   //
      6, 7, 4,   //
                 // back
      8, 9, 10,  //
      10, 11, 8, //
      // bottom
      12, 13, 14, //
      14, 15, 12, //
      // left
      16, 17, 18, //
      18, 19, 16, //
      // right
      20, 21, 22, //
      22, 23, 20, //
  };


  // for (int i =0; i< numTextures; i++){
  for (int i = 0; i < numTextures; i++) {
    memcpy(cubes[i], cube, sizeof(cube));
    initCubeTexture(i, cube, cubes);
    // for (int y = 0; y < 4; y++) {
    //   std::cout << cubes[i][3 + y * 5] << "," << cubes[i][4 + y * 5] << std::endl;
    // }
  }

  // for (int x = 0; x < rows; x++) {
  //   for (int z = 0; z < rows; z++) {
  //     for (int y = 0; y < 2; y++) {
  //       cubePositions[x * rows + y + z * rows] = glm::vec3(x * 2, z * 2, y * 2);
  //     }
  //   }
  // }

  const int rows = 32;
  const int strata = 24;
  glm::vec3 cubePositions[strata * rows * rows]; // height rows^2
  int cubeTypes[strata * rows * rows];
  std::fill(std::begin(cubeTypes), std::end(cubeTypes), air);
  for (int x = 0; x < rows; x++) {
    for (int z = 0; z < rows; z++) {
      for (int y = 0; y < strata; y++) {
        cubePositions[x * rows + z + y * rows * rows] = glm::vec3(x * 2, y * 2, z * 2);
      }
    }
  }

  const int count = rows * rows * strata;

  // todo,
  //sky
  //lines fix

  glm::mat4 projectionMatrix = getProjectionMatrix();

  GLuint vao, ebo;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &ebo);

  const int stoneHeight = 2;
  const int sandHeight = 4;
  const int dirtHeight = 12;
  const int grassHeight = 13;
  const int square = rows * rows;

  setCubeTypes(0, stoneHeight, cubeTypes, rows, stone);
  setCubeTypes(stoneHeight, sandHeight, cubeTypes, rows, sand);
  setCubeTypes(sandHeight, dirtHeight, cubeTypes, rows, dirt);
  setCubeTypes(dirtHeight, grassHeight, cubeTypes, rows, grass);
  pourWater(cubeTypes, rows, sandHeight, strata);


  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glUniform1i(blockTexId, 0);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    computeMatricesFromInputs();

    glm::mat4 projView = projectionMatrix * getViewMatrix();

    drawType(stone, rows, cubePositions, strata, cubeTypes, matrixId, projView, cubes);
    drawType(sand, rows, cubePositions, strata, cubeTypes, matrixId, projView, cubes);
    drawType(dirt, rows, cubePositions, strata, cubeTypes, matrixId, projView, cubes);
    drawType(grass, rows, cubePositions, strata, cubeTypes, matrixId, projView, cubes);
    drawType(water, rows, cubePositions, strata, cubeTypes, matrixId, projView, cubes);


    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glfwSwapBuffers(window);
    glfwPollEvents();

  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

  glfwTerminate();

  return 0;
}
