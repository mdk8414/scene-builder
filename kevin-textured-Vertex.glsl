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
//out vec3 diffs[];
//out vec3 ambs[];
//out vec3 specs[];
//out vec3 poss[];
out vec3 pointDiff;
out vec3 pointAmb;
out vec3 pointSpec;
out vec3 pointPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 inColor;
//uniform vec3 lightsDiff[];
//uniform vec3 lightsAmb[];
//uniform vec3 lightsSpec[];
//uniform vec3 lightsPos[];
uniform vec3 lightDiff;
uniform vec3 lightAmb;
uniform vec3 lightSpec;
uniform vec3 lightPos;

void main() {
   Color = inColor;
   gl_Position = proj * view * model * vec4(position,1.0);
   pos = (view * model * vec4(position,1.0)).xyz;
   lightDir = (view * vec4(inLightDir,0.0)).xyz; //It's a vector!
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);
   vertNormal = normalize(norm4.xyz);
   texcoord = inTexcoord;

   pointDiff = lightDiff;
   pointAmb = lightAmb;
   pointSpec = lightSpec;
   pointPos = lightPos;
}