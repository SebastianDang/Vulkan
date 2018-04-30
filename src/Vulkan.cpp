#include "stdafx.h"
#include "Vulkan.h"

std::string Vulkan::Title = "Vulkan";
int Vulkan::Width = 800;
int Vulkan::Height = 600;

void Vulkan::Run()
{
	InitWindow(Vulkan::Width, Vulkan::Height, Vulkan::Title.c_str());
	InitVulkan();
	MainLoop();
	Cleanup();
}

void Vulkan::InitWindow(int width, int height, const char *title)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We have to specify that we're not using OpenGL.
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Will be adjusted later.
	m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void Vulkan::InitVulkan()
{
	CreateInstance(Vulkan::Title.c_str(), "No Engine");
}

void Vulkan::CreateInstance(const char *appName, const char *engineName)
{
	// Define the application layer. 
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = engineName;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pNext = nullptr; // TODO: pNext for extension types.

	// Define the create layer.
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// For windows, link it to glfw extensions.
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Fill in the rest of the create info.
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	// Create the actual instance now. Store it as m_Instance.
	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance); // TODO: Fill in custom allocator callbacks later.
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Error: Failed to create instance!");
	}

	// Check for extension support.
	bool extensionsSupported = CheckGLFWExtensionSupport(glfwExtensions, glfwExtensionCount);
	if (!extensionsSupported)
	{
		std::cerr << "Some GLFW Extensions are not supported by Vulkan." << std::endl;
	}
}

bool Vulkan::CheckGLFWExtensionSupport(const char ** glfwExtensions, int glfwExtensionCount)
{
	// Query Vulkan for extension support.
	uint32_t vkExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr); // We first query for the number of extensions.
	std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data()); // Next we fill a vector with it's properties.

	// Check if GLFW extensions are supported in Vulkan.
	bool glfwFullySupported = true;
	for (int g = 0; g < glfwExtensionCount; g++)
	{
		for (int v = 0; v < (int)vkExtensionCount; v++)
		{
			if (strcmp(glfwExtensions[g], vkExtensions[v].extensionName) == 0)
			{
				std::cout << "Supported by Vulkan: " << glfwExtensions[g] << std::endl;
				break;
			}
			if (v == ((int)vkExtensionCount - 1))
			{
				std::cout << "Not supported by Vulkan: " << glfwExtensions[g] << std::endl;
				glfwFullySupported = false;
			}
		}
	}
	return glfwFullySupported;
}

void Vulkan::MainLoop()
{
	while (!glfwWindowShouldClose(m_pWindow)) 
	{
		glfwPollEvents();
	}
}

void Vulkan::Cleanup()
{
	vkDestroyInstance(m_Instance, nullptr); // TODO: Fill in custom allocator callbacks later.
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

