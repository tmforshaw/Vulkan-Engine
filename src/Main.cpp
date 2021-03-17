#include "Application.hpp"

int main()
{
	Application app;

	try // Run the applicaton and catch errors
	{
		app.Run();
	}
	catch ( const std::exception& e )
	{
		std::cerr << e.what() << std::endl; // Output error to console
		return 1;							// Error
	}

	return 0; // No errors
}