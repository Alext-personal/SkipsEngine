#type Vertex

#version 330 core
layout(location = 0) in vec3 position; 
out vec3 v_position;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
void main() 
{ 
	v_position = position;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position,1.0f);
}

#type Fragment

#version 330 core
in vec3 v_position;
out vec4 color;
void main()
{
	color = vec4(v_position*0.5 + 0.5,1.0f);
}
