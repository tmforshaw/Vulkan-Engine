#pragma once

#include "../Buffers/UniformBuffers.hpp"

// Default values
#define MOVE_SPEED		10.0f
#define MOVE_SPEED_FAST 25.0f
#define MOUSE_SENS		0.005f
#define ZOOM_SENS		5.0f
#define FOV_DEFAULT		45.0f

#define MIN_FOV 1.0f
#define MAX_FOV 75.0f

// Define the ways the camera can move
enum class CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;

	UniformBufferObject m_MVP;

	// Axis
	glm::vec3 m_xDirection;
	glm::vec3 m_zDirection;
	glm::vec3 m_yDirection;

	glm::vec3 m_worldUp;

public:
	// Properties
	float m_moveSpeed;
	float m_moveSpeedFast;
	float m_mouseSensitivity;
	float m_zoomSensitivity;
	float m_fov;

	bool m_movingFast;

	void Init( const glm::vec3& p_position, const glm::vec3& p_target, const float& p_aspectRatio )
	{
		// Set the member variables
		m_position		   = p_position;
		m_moveSpeed		   = MOVE_SPEED;
		m_moveSpeedFast	   = MOVE_SPEED_FAST;
		m_mouseSensitivity = MOUSE_SENS;
		m_zoomSensitivity  = ZOOM_SENS;
		m_fov			   = FOV_DEFAULT;
		m_worldUp		   = glm::vec3( 0.0f, 0.0f, 1.0f );

		// Set the MVP matrix
		m_MVP.model = glm::mat4( 1.0f );
		m_MVP.view	= glm::lookAt( m_position, p_target, m_worldUp );
		m_MVP.proj	= glm::perspective( glm::radians( m_fov ), p_aspectRatio, 0.1f, 100.0f );

		// Flip the y axis of the projection matrix
		m_MVP.proj[1][1] *= -1;

		// Update the vectors
		UpdateVectors();
	}

	void ProcessKeyboard( const CameraMovement& dir, const float& deltaT )
	{
		// Define the velocity the camera is moving at
		float velocity = ( m_movingFast ? m_moveSpeedFast : m_moveSpeed ) * deltaT;

		// Move in the correct direction based upon the input direction
		switch ( dir )
		{
		case CameraMovement::FORWARD:
			m_position += m_zDirection * velocity;
			break;
		case CameraMovement::BACKWARD:
			m_position -= m_zDirection * velocity;
			break;
		case CameraMovement::RIGHT:
			m_position += m_xDirection * velocity;
			break;
		case CameraMovement::LEFT:
			m_position -= m_xDirection * velocity;
			break;
		case CameraMovement::UP:
			m_position += m_worldUp * velocity;
			break;
		case CameraMovement::DOWN:
			m_position -= m_worldUp * velocity;
			break;
		default:
			break;
		}
	}

	void ProcessMouse( const float& xOff, const float& yOff )
	{
		// Dampen or strengthen the mouse input to a reasonable level and add the adjusted offsets to the rotation
		m_rotation.x -= xOff * m_mouseSensitivity;
		m_rotation.y += yOff * m_mouseSensitivity;

		// Update the axis of the camera (Also ensures that the rotation is clamped)
		UpdateVectors();
	}

	void ProcessMouseScroll( const float& yOff )
	{
		// Reduce the field of view when the user scrolls (adjusted to make it easier/harder)
		m_fov -= yOff * m_zoomSensitivity;

		// Clamp the FOV so that the camera is still usable
		if ( m_fov < MIN_FOV )
			m_fov = MIN_FOV;
		else if ( m_fov > MAX_FOV )
			m_fov = MAX_FOV;
	}

	void UpdateVectors()
	{
		// Fix yaw by removing the remainder (modulo 2 pi)
		m_rotation.x = glm::mod( m_rotation.x, glm::pi<float>() * 2.0f );

		// Constrain pitch
		if ( m_rotation.y > glm::radians( 89.0f ) )
			m_rotation.y = glm::radians( 89.0f );
		if ( m_rotation.y < glm::radians( -89.0f ) )
			m_rotation.y = glm::radians( -89.0f );

		// Set new forward direction
		m_zDirection = glm::normalize( glm::vec3(
			cos( m_rotation.x ) * cos( m_rotation.y ),
			sin( m_rotation.x ) * cos( m_rotation.y ),
			sin( m_rotation.y ) ) );

		// Recalculate x and y axis
		m_xDirection = glm::normalize( glm::cross( m_zDirection, m_worldUp ) );
		m_yDirection = glm::normalize( glm::cross( m_xDirection, m_zDirection ) );
	}

	inline const UniformBufferObject& GetMVP()
	{
		// Update view matrix
		m_MVP.view = glm::lookAt( m_position, m_position + m_zDirection, m_worldUp );
		return m_MVP;
	}
};