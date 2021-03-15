#version 460
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) out vec3 oFragColour;

float pi = 3.1415926;

vec2 positions[] = vec2[](
	vec2( 0.0, -sin( pi / 3 ) / 2 ),
	vec2( 0.5, sin( pi / 3 ) / 2 ),
	vec2( -0.5, sin( pi / 3 ) / 2 ) );

vec3 colours[] = vec3[](
	vec3( 1.0, 0.0, 0.0 ),
	vec3( 0.0, 1.0, 0.0 ),
	vec3( 0.0, 0.0, 1.0 ) );

void main()
{
	gl_Position = vec4( positions[gl_VertexIndex], 0.0, 1.0 );
	oFragColour = colours[gl_VertexIndex];
}