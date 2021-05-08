#version 150 core

in vec3 position;
//in vec3 inColor;

//const vec3 inColor = vec3(0.f,0.7f,0.f);
const vec3 inLightDir = normalize(vec3(-1,1,-1));
in vec3 inNormal;
in vec2 inTexcoord;

out vec3 Color;
out vec3 vertNormal;
out vec3 pos;
out vec3 lightDir;
out vec2 texcoord;
out vec3 pointPoss[11];
out float pointScales[11];
out vec3 pointColors[11];

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 inColor;
uniform float lightsScale[11];
uniform vec3 lightsPos[11];
uniform vec3 lightsColor[11];

void main() {
   Color = inColor;
   gl_Position = proj * view * model * vec4(position,1.0);
   pos = (view * model * vec4(position,1.0)).xyz;
   lightDir = (view * vec4(inLightDir,0.0)).xyz; //It's a vector!
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);
   vertNormal = normalize(norm4.xyz);
   texcoord = inTexcoord;

   for (int i = 0; i < 11; i++) {
      pointPoss[i] = (view*vec4(lightsPos[i],1.0)).xyz;
      pointColors[i] = lightsColor[i];
      pointScales[i] = lightsScale[i];
   }
}