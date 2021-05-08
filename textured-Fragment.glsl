#version 150 core

in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
in vec3 lightDir;
in vec2 texcoord;
in vec3 pointColors[11];
in float pointScales[11];
in vec3 pointPoss[11];

out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;

uniform int texID;

uniform float ambient = .1;
uniform float specularity = 1;
uniform float diffuse = 1;
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
  vec3 diffuseC = color*max(dot(-lightDir,normal),0.0) * diffuse;

  vec3 pointContr = vec3(0.0,0.0,0.0);
  for (int i = 0; i < 11; i++) {
    float d = length(pointPoss[i]-pos);
    float intensity = 1.0 / (1.0 + 0.09*d + 0.032*d*d);
    vec3 pointDir = normalize(pointPoss[i]-pos);
    vec3 pointReflectDir = reflect(-pointDir, normal);
    float pSpec = max(dot(pointReflectDir,viewDir),0.0);
    if (dot(-pointDir, normal) <= 0.0) pSpec = 0;
    vec3 pAmbC = color*pointColors[i];
    vec3 pSpecC = .8*vec3(1.0,1.0,1.0)*pow(pSpec,4);
    vec3 pDiffC = pointColors[i]*color*max(dot(pointDir,normal),0.0);
    pointContr += intensity*(pAmbC+pDiffC+pSpecC)*pointScales[i];
  }

  vec3 ambC = color*ambient;
  //vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)! (Do you know why?)
  vec3 reflectDir = reflect(viewDir,normal);
  float spec = max(dot(reflectDir,lightDir),0.0);
  if (dot(-lightDir,normal) <= 0.0) spec = 0; //No highlight if we are not facing the light
  vec3 specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4) * specularity;
  vec3 oColor = ambC+diffuseC+specC+pointContr;
  if (oColor.x > 1.0) {
    oColor.x = 1.0;
  } if (oColor.y > 1.0) {
    oColor.y = 1.0;
  } if (oColor.z > 1.0) {
    oColor.z = 1.0;
  }
  outColor = vec4(oColor,1);
}