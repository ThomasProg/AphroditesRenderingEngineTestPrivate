//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 worldNormal;

//output write
layout (location = 0) out vec4 outFragColor;

void main() 
{	
	outFragColor = vec4((worldNormal.xyz + vec3(1,1,1)) / 2.0, 1.0f);
}