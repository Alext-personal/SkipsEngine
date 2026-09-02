#type Vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texcoords;
out vec3 v_position;
out vec2 v_texcoords;
uniform mat4 modelMatrix;
layout(std140,binding = 0) uniform Matrices{
	mat4 projection;
	mat4 view;
};
void main() 
{ 
	v_position = position;
	gl_Position = projection * view * modelMatrix * vec4(position,1.0f);
	v_texcoords = texcoords;
}

#type Fragment
#version 460 core

in vec3 v_position;
in vec2 v_texcoords;
out vec4 color;
uniform sampler2D AlbedoTexture;
layout(std140,binding = 1) uniform material{
	vec4 tint;
};
void main()
{
	color = tint * texture(AlbedoTexture,v_texcoords);
}
