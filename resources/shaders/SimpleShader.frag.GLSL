#version 460
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 fragColour;
layout( location = 1 ) in vec2 fragTexCoord;
layout( location = 2 ) in flat uint fragSamplerID;

layout( binding = 1 ) uniform sampler2D texSampler1;
layout( binding = 2 ) uniform sampler2D texSampler2;

layout( location = 0 ) out vec4 oColour;

vec3 GetColourFromSampler( uint p_ID )
{
	switch ( fragSamplerID )
	{
	case 0: return texture( texSampler1, fragTexCoord ).rgb;
	case 1: return texture( texSampler2, fragTexCoord ).rgb;
	default: return texture( texSampler1, fragTexCoord ).rgb;
	}
}

void main()
{
	oColour = vec4( GetColourFromSampler( fragSamplerID ), 1.0 );
}