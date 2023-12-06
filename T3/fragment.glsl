#version 410

uniform vec4 lamb;
uniform vec4 ldif;
uniform vec4 lspe;

uniform vec4 mamb;
uniform vec4 mdif;
uniform vec4 mspe;
uniform float mshi;
uniform float mopacity;

in data {
  vec3 normal;
  vec3 view;
  vec3 light;
} f;

out vec4 color;

void main (void) 
{
  vec3 normal = normalize(f.normal);
  vec3 view = normalize(f.view);
  vec3 light = normalize(f.light);
  float ndotl = dot(normal,light);
  color = mamb*lamb + mdif * ldif * max(0,ndotl); 
  if (ndotl > 0) {
    vec3 refl = normalize(reflect(-light,normal));
    color += mspe * lspe * pow(max(0,dot(refl,view)),mshi); 
  }
  color.a = mopacity;
}