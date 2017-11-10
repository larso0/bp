#version 450 core

vec2 positions[4] = vec2[](
	vec2(-1.0, 1.0),
	vec2(-1.0, -1.0),
	vec2(1.0, 1.0),
	vec2(1.0, -1.0)
);

vec2 textureCoordinates[4] = vec2[](
	vec2(0.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 1.0),
	vec2(1.0, 0.0)
);

layout (push_constant) uniform _Area
{
	vec2 offset;
	vec2 extent;
} area;

layout (location = 1) out vec2 textureCoordinate;

void main()
{
	gl_Position = vec4(area.extent * positions[gl_VertexIndex] + area.offset, 0.0, 1.0);
	textureCoordinate = textureCoordinates[gl_VertexIndex];
}