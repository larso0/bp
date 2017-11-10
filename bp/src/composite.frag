#version 450 core

layout (location = 0) out vec3 color;

layout (location = 1) in vec2 textureCoordinate;

layout (set = 0, binding = 0) uniform sampler2D imageSampler;

void main()
{
	color = texture(imageSampler, textureCoordinate).rgb;
}