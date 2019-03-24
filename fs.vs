#version 330 core
out vec4 color;
// in vec3 fragmentColor;

in vec2 UV;
uniform sampler2D textureSampler;

void main(){
  // color =  fragmentColor;
  color = texture(textureSampler, UV).rgba;
  // vec4 col = texture(textureSampler, UV).rgba;

  // color = vec3(1,0,0);
}