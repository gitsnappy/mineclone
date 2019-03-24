#version 330 core


// layout(location = 2) in bool transparent;
layout(location = 1) in vec2 uv;
layout(location = 0) in vec3 pos;
vec2 mid;
out vec2 UV;
// out vec3 fragmentColor;
uniform mat4 MVP;
void main() {

  gl_Position = MVP * vec4(pos, 1.0);

  // fragmentColor = vertexColor;
  mid = uv;
  mid.y = 1.0 - mid.y;


  UV = vec2(mid);


}