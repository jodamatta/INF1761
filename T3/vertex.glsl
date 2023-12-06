#version 410

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;

uniform vec4 lpos;  // light pos in lighting space
uniform vec4 cpos;  // camera pos in lighting space
uniform mat4 Mv; 
uniform mat4 Mn; 
uniform mat4 Mvp;

out data {
  vec3 normal;
  vec3 view;
  vec3 light;
} v;

void main (void) 
{
  vec3 p = vec3(Mv*pos);
  if (lpos.w == 0) 
    v.light = normalize(vec3(lpos));
  else 
    v.light = vec3(3.0f,3.0f,3.0f); 
  v.view = normalize(vec3(cpos)-p);
  v.normal = normalize(vec3(Mn*vec4(normal,0.0f)));
  gl_Position = Mvp*pos; 
}

