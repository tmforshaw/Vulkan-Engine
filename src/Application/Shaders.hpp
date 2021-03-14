#pragma once
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

static std::vector<char> ReadFile( const std::string& filepath )
{
	// Create a filestream that starts reading from the end, and treats it as binary
	std::ifstream fs( filepath, std::ios::ate | std::ios::binary );

	// Create a buffer
	std::vector<char> buffer;

	if ( fs.is_open() )
	{
		// Get the position (starting at the end gets the size)
		size_t fileSize = (size_t)fs.tellg();

		// Set the buffer size
		buffer = std::vector<char>( fileSize );

		// Go to the beginning of the files
		fs.seekg( 0 );
		fs.read( buffer.data(), fileSize );

		// Close the filestream
		fs.close();
	}
	else
		throw std::runtime_error( "Failed to open file" );

	return buffer;
}