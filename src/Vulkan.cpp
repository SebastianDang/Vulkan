#include "stdafx.h"
#include "Vulkan.h"

std::string Vulkan::Title = "Vulkan";
int Vulkan::Width = 800;
int Vulkan::Height = 600;
std::vector<const char*> Vulkan::ValidationLayers = { "VK_LAYER_LUNARG_standard_validation" };


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
	SetupDebugCallback();
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

	// For windows, we gather glfw required extensions.
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Verify glfw Vulkan support here.
	bool extensionsSupported = CheckGLFWExtensionSupport(glfwExtensions, glfwExtensionCount);
	if (!extensionsSupported) throw std::runtime_error("Some required GLFW Extensions are not supported by Vulkan.");

	// If we have validation layers that require extensions, we need to add it here.
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if ((int)Vulkan::ValidationLayers.size() > 0)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	// Fill in create info.
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
	createInfo.ppEnabledExtensionNames = extensions.data();

	// If we have validation layers, we can directly verify Vulkan support here.
	bool validationLayersSupported = CheckValidationLayerSupport(Vulkan::ValidationLayers);
	if (!validationLayersSupported) throw std::runtime_error("Some validation layers requested are not available.");

	// Fill in create info.
	createInfo.enabledLayerCount = static_cast<uint32_t>(Vulkan::ValidationLayers.size());
	createInfo.ppEnabledLayerNames = ((int)Vulkan::ValidationLayers.size() > 0) ? Vulkan::ValidationLayers.data() : nullptr;

	// Create the actual instance now. Store it as m_Instance.
	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance); // TODO: Fill in custom allocator callback later.
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create instance.");
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
				std::cout << "Not Supported by Vulkan: " << glfwExtensions[g] << std::endl;
				glfwFullySupported = false;
			}
		}
	}
	return glfwFullySupported;
}

bool Vulkan::CheckValidationLayerSupport(std::vector<const char*> validationLayers)
{
	// Query Vulkan for layer support.
	uint32_t availableLayerCount;
	vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr); // We first query for the number of layers.
	std::vector<VkLayerProperties> availableLayers(availableLayerCount);
	vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

	// Check if all validation layers exist in available layers.
	bool layersFullySupported = true;
	for (int v = 0; v < (int)validationLayers.size(); v++)
	{
		for (int a = 0; a < (int)availableLayerCount; a++)
		{
			if (strcmp(validationLayers[v], availableLayers[a].layerName) == 0)
			{
				std::cout << "Supported by Vulkan: " << validationLayers[v] << std::endl;
				break;
			}
			if (a == ((int)availableLayerCount - 1))
			{
				std::cout << "Not Supported by Vulkan: " << validationLayers[v] << std::endl;
				layersFullySupported = false;
			}
		}
	}
	return layersFullySupported;
}

void Vulkan::SetupDebugCallback()
{
	if (Vulkan::ValidationLayers.size() <= 0) return; // Validation layers weren't requested.

 	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback; // Pointer to the callback function.
	createInfo.pUserData = nullptr; // We can add pointers to our own data here.

	if (Vulkan::CreateDebugReportCallbackEXT(m_Instance, &createInfo, nullptr, &m_DebugCallback) != VK_SUCCESS) // TODO: Fill in custom allocator callback later.
		throw std::runtime_error("Failed to set up debug callback.");
}

// The flag parameter specifies the type of message.
// The objType parameter specifies the type of object (the subject of the debug message). 
VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, 
	uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData)
{
	std::cerr << "DebugCallback: " << flags << objType << obj << location << code << layerPrefix << msg << userData << std::endl;
	return VK_FALSE;
}

VkResult Vulkan::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT * pCreateInfo,
	const VkAllocationCallbacks * pAllocator, VkDebugReportCallbackEXT * pCallback)
{
	auto fn = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (fn != nullptr) return fn(instance, pCreateInfo, pAllocator, pCallback);
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Vulkan::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks * pAllocator)
{
	auto fn = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (fn != nullptr) fn(instance, callback, pAllocator);
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
	if ((int)Vulkan::ValidationLayers.size() > 0) 
		Vulkan::DestroyDebugReportCallbackEXT(m_Instance, m_DebugCallback, nullptr);

	vkDestroyInstance(m_Instance, nullptr); // TODO: Fill in custom allocator callback later.
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}
