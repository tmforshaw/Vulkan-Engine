#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Light
{
protected:
	glm::vec3 m_colour;

public:
	Light() : m_colour( { 1.0f, 1.0f, 1.0f } ) {}

	inline const glm::vec3& GetCol() const { return m_colour; }
};

class DirLight : public Light
{
private:
	// glm::vec3 m_direction;

public:
};

class PointLight : public Light
{
private:
	glm::vec3 m_position;

public:
	PointLight() : m_position( { 0.0f, 0.0f, 0.0f } )
	{
		// Set member variables
		m_colour = { 1.0f, 1.0f, 1.0f };
	}
	PointLight( const glm::vec3& p_colour, const glm::vec3& p_position )
		: m_position( p_position )
	{
		// Set member variables
		m_colour = p_colour;
	}

	inline const glm::vec3& GetPos() const { return m_position; }
};