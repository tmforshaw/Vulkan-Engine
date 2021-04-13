#pragma once

#include "../VulkanUtil/Window.hpp"
#include "Keyboard.hpp"

#include <GLFW/glfw3.h>

// Mouse variables
static float prevMouseX = WINDOW_WIDTH / 2, prevMouseY = WINDOW_HEIGHT / 2;
static bool	 firstMouse = true;

static bool KeyboardCallbackBool = false;
static bool MouseCallbackBool	 = false;
static bool ScrollCallbackBool	 = false;

static double xPosMouse;
static double yPosMouse;

static double yOffScroll;

static void KeyboardCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	KeyboardCallbackBool = true;

	// Set the correct key
	KeyboardHandler::Set( (char)key, (bool)action );
}

void MouseCallback( GLFWwindow* window, double xPos, double yPos )
{
	MouseCallbackBool = true;

	xPosMouse = xPos;
	yPosMouse = yPos;
}

void ScrollCallback( GLFWwindow* window, double xOff, double yOff )
{
	ScrollCallbackBool = true;

	yOffScroll = yOff;
}

static void ProcessCallbacks( Camera* p_camera )
{
	if ( MouseCallbackBool )
	{
		if ( firstMouse )
		{
			prevMouseX = xPosMouse;
			prevMouseY = yPosMouse;
			firstMouse = false;
		}

		p_camera->ProcessMouse( xPosMouse - prevMouseX, prevMouseY - yPosMouse ); // Reverse because y-axis is from bottom to top

		prevMouseX = xPosMouse;
		prevMouseY = yPosMouse;

		MouseCallbackBool = false;
	}

	if ( ScrollCallbackBool )
	{
		p_camera->ProcessMouseScroll( (float)yOffScroll );

		ScrollCallbackBool = false;
	}
}