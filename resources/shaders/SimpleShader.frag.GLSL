#version 460
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 fragPos;
layout( location = 1 ) in vec3 fragNormal;
layout( location = 2 ) in vec2 fragTexCoord;
layout( location = 3 ) in flat uint fragSamplerID;
layout( location = 4 ) in vec3 lightDir;

layout( binding = 1 ) uniform sampler2D texSampler1;
layout( binding = 2 ) uniform sampler2D texSampler2;

// // clang-format off
// layout( binding = 1 ) uniform PointLight
// {
// 	vec3 colour;
// 	vec3 position;
// } pointLights[1];
// // clang-format on

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
	vec3 lightColour = vec3( 1.0, 1.0, 1.0 );

	float ambientStrength  = 0.1;
	float specularStrength = 0.5;

	vec3 ambientColour = ambientStrength * lightColour;

	vec3 norm = normalize( fragNormal );
	// vec3 lightDir = normalize( fragPos - pointLights[0].position );
	// vec3 lightDir = normalize( vec3( 0.0f, -1.0f, 0.0f ) );

	float diff			= max( dot( norm, lightDir ), 0.0 ); // Remove negative values
	vec3  diffuseColour = diff * lightColour;

	// vec3 viewDir	= normalize( -fragPos );
	// vec3 reflectDir = reflect( -lightDir, norm );

	// float spec			 = pow( max( dot( viewDir, reflectDir ), 0.0 ), 32 );
	// vec3  specularColour = specularStrength * spec * lightColour;

	oColour = vec4( ( ambientColour + diffuseColour ) * GetColourFromSampler( fragSamplerID ), 1.0 ); //  + specularColour

	// oColour = vec4( lightDir, 1.0 );
}