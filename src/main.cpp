#include "stdafx.h"
#include "Vulkan.h"

int main() 
{
	// Initialize the Vulkan app.
	Vulkan app;

	try { app.Run(); }
	catch (const std::runtime_error& e) // Throw this error from Vulkan if anything occurs during 'Run'.
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
