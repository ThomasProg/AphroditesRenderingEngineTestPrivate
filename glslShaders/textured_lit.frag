//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 1) uniform SceneData
{   
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} sceneData;

layout(set = 2, binding = 0) uniform sampler2D tex1;


layout(std140, set = 2, binding = 1) uniform TestData
{ 
	vec4 multValue;
} testData;



float Convert_sRGB_FromLinear (float theLinearValue) 
{
  return theLinearValue <= 0.0031308f
       ? theLinearValue * 12.92f
       : pow (theLinearValue, 1.0f/2.4f) * 1.055f - 0.055f;
}

vec3 Convert_sRGB_FromLinear (vec3 theLinearValue) 
{
  return vec3(Convert_sRGB_FromLinear(theLinearValue.x), Convert_sRGB_FromLinear(theLinearValue.y), Convert_sRGB_FromLinear(theLinearValue.z));
}

float Convert_sRGB_ToLinear (float thesRGBValue) 
{
  return thesRGBValue <= 0.04045f
       ? thesRGBValue / 12.92f
       : pow ((thesRGBValue + 0.055f) / 1.055f, 2.4f);
}


void main() 
{
	vec3 color = texture(tex1,texCoord).xyz;
	// outFragColor = vec4(color * testData.multValue.xyz,1.0f);

	outFragColor = vec4(Convert_sRGB_FromLinear(color * testData.multValue.xyz),1.0f); // gamma correction
}
