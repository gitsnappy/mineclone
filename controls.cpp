#include <GLFW/glfw3.h>
extern GLFWwindow *window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "controls.hpp"
#include <iostream>

glm::mat4 ViewMatrix;
// glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() { return ViewMatrix; }
glm::mat4 getProjectionMatrix() { 
  float fov = 45.0f;
  return glm::perspective(glm::radians(fov), 4.0f / 3.0f, 0.1f, 100.0f);
  // return glm::perspective(glm::radians(fov), 4.0f / 3.0f, 0.1f, 50.0f);
// ProjectionMatrix = 
}


glm::vec3 position = glm::vec3(-10, 25.0, 15.0);
float horizontalAngle = 3.14f; //3.14f
float verticalAngle = -0.7f;

float speed = 5.5f; 
float mouseSpeed = 0.003f;

void computeMatricesFromInputs() {
  static double lastTime = glfwGetTime();

  double currentTime = glfwGetTime();
  float deltaTime = float(currentTime - lastTime);

  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  glfwSetCursorPos(window, 1024 / 2, 768 / 2);

  // Compute new orientation
  horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
  verticalAngle -= mouseSpeed * float(768 / 2 - ypos);

  // spherical to cartesian
  glm::vec3 direction(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle));
  glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.14f / 2.0f), 0, cos(horizontalAngle - 3.14f / 2.0f));
  // glm::vec3 up = glm::cross(right, direction);
  glm::vec3 up = glm::vec3(0,1,0);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    position += direction * deltaTime * speed;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    position -= direction * deltaTime * speed;
  }
  if (glfwGetKey(window,  GLFW_KEY_D) == GLFW_PRESS) {
    position -= right * deltaTime * speed;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    position += right * deltaTime * speed;
  }
  ViewMatrix = glm::lookAt(position,             
                           position + direction, 
                           up                    
  );

  lastTime = currentTime;
}