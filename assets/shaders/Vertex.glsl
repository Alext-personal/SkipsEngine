#version 330 core
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 a_Color;
uniform mat4 modelMatrix;
out vec3 v_Color;
void main() 
{ 
	gl_Position = modelMatrix * vec4(position,1.0f);
	v_Color = a_Color;
}