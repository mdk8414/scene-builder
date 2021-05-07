#version 150 core

in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
in vec3 lightDir;
in vec2 texcoord;
in vec3 pointDiff;
in vec3 pointAmb;
in vec3 pointSpec;
in vec3 pointPos;

out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;

uniform int texID;

const float ambient = .3;
void main() {
  vec3 color;
  if (texID == -1)
    color = Color;
  else if (texID == 0)
    color = texture(tex0, texcoord).rgb;
  else if (texID == 1)
    color = texture(tex1, texcoord).rgb;  
  else{
    outColor = vec4(1,0,0,1);
    return; //This was an error, stop lighting!
  }
  vec3 normal = normalize(vertNormal);
  vec3 viewDir = normalize(-pos);
  vec3 diffuseC = color*max(dot(-lightDir,normal),0.0);

  float d = distance(pos, pointPos);
  float intensity = 1.0 / (1.0 + 0.09*d + 0.032*d*d);
  vec3 pointLightDir = normalize(pointPos-pos);
  vec3 pointReflectDir = reflect(viewDir, normal);
  float pSpec = max(dot(pointReflectDir,pointLightDir),0.0);
  if (dot(-pointLightDir, normal) <= 0.0) pSpec = 0;
  vec3 pAmbC = color*ambient;
  vec3 pSpecC = .8*vec3(1.0,1.0,1.0)*pow(pSpec,4);
  vec3 pDiffC = color*max(dot(-pointLightDir,normal),0.0);
  vec3 pColor = (pAmbC+pDiffC+pSpecC)*intensity;


  vec3 ambC = color*ambient;
  //vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)! (Do you know why?)
  vec3 reflectDir = reflect(viewDir,normal);
  float spec = max(dot(reflectDir,lightDir),0.0);
  if (dot(-lightDir,normal) <= 0.0) spec = 0; //No highlight if we are not facing the light
  vec3 specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4);
  vec3 oColor = ambC+diffuseC+specC+pColor;
  outColor = vec4(oColor,1);
}