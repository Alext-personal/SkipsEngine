#type Vertex

#version 460 core
layout(location = 0) in vec3 position; 
out vec3 v_position;
uniform mat4 modelMatrix;
layout(std140,binding = 0) uniform Matrices{
	mat4 projection;
	mat4 view;
};
void main() 
{ 
	v_position = position;
	gl_Position = projection * view * modelMatrix * vec4(position,1.0f);
}

#type Fragment

#version 460 core
in vec3 v_position;
out vec4 color;
void main()
{
	color = vec4(v_position*0.5 + 0.5,1.0f);
}
