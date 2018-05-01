#include "stdafx.h"
#include "Vulkan.h"

std::string Vulkan::Title = "Vulkan";
int Vulkan::Width = 800;
int Vulkan::Height = 600;
std::vector<const char*> Vulkan::ValidationLayers = { "VK_LAYER_LUNARG_standard_validation" };

// Initialize everything here.
// Once it's done, we run the main rendering loop.
// When the program closes, we properly close and delete anything initialized.
void Vulkan::Run()
{
	InitWindow(Vulkan::Width, Vulkan::Height, Vulkan::Title.c_str());
	InitVulkan();
	MainLoop();
	Cleanup();
}

// Create a new glfw window. We need to specify glfw for Vulkan instead of OpenGL.
// We reference this window with a member variable.
void Vulkan::InitWindow(int width, int height, const char *title)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We have to specify that we're not using OpenGL.
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Will be adjusted later.
	m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

// Create an instance of Vulkan. 
// Once that's created, we grab any additional extensions and setup callbacks here.
void Vulkan::InitVulkan()
{
	m_Instance = CreateInstance(Vulkan::Title.c_str(), "No Engine");
	m_DebugCallback = SetupDebugCallback(m_Instance); // If we want the debug callback.
	m_Surface = CreateWindowsSurface(m_Instance, m_pWindow);
	m_PhysicalDevice = CreatePhysicalDevice(m_Instance, m_Surface);
	m_LogicalDevice = CreateLogicalDevice(m_PhysicalDevice, m_Surface);
	m_GraphicsQueue = GetDeviceQueue(m_PhysicalDevice, m_Surface, m_LogicalDevice, 0);
	m_PresentQueue = GetDeviceQueue(m_PhysicalDevice, m_Surface, m_LogicalDevice, 1);
}

// We have to define some Vulkan properties and set up the instance.
// Any validation layers and extensions need to be added, then
// verified here.
VkInstance Vulkan::CreateInstance(const char *appName, const char *engineName)
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

	// We gather glfw required extensions.
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

	// Create the actual instance now.
	VkInstance instance;
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance); // TODO: Fill in custom allocator callback later.
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create instance.");
	return instance;
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

VkDebugReportCallbackEXT Vulkan::SetupDebugCallback(VkInstance instance)
{
	if (Vulkan::ValidationLayers.size() <= 0) return VkDebugReportCallbackEXT(); // Validation layers weren't requested.

 	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback; // Pointer to the callback function.
	createInfo.pUserData = nullptr; // We can add pointers to our own data here.

	VkDebugReportCallbackEXT debugCallback;
	VkResult result = Vulkan::CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &debugCallback); // TODO: Fill in custom allocator callback later.
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to set up debug callback.");
	return debugCallback;
}

// The flag parameter specifies the type of message.
// The objType parameter specifies the type of object (the subject of the debug message). 
VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, 
	uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData)
{
	// TODO: Add more stuff for debugging here.
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

VkSurfaceKHR Vulkan::CreateWindowsSurface(VkInstance instance, GLFWwindow *pWindow)
{
	VkSurfaceKHR surface;
	VkResult result = glfwCreateWindowSurface(instance, pWindow, nullptr, &surface);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create surface.");
	return surface;
}

VkPhysicalDevice Vulkan::CreatePhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // We first query the number of devices.
	
	// Verify that we have a GPU that supports Vulkan.
	if (deviceCount == 0) throw std::runtime_error("Failed to find any GPUs with Vulkan support.");
		
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Use an ordered map to automatically sort candidates by rank.
	std::multimap<int, VkPhysicalDevice> physicalDeviceCandidates;
	for (int i = 0; i < (int)deviceCount; i++)
	{
		int rank = RankPhysicalDevice(devices[i], surface);
		physicalDeviceCandidates.insert(std::make_pair(rank, devices[i]));
	}

	// Check the best candidate (highest rank), and if it meets requirements.
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	if (physicalDeviceCandidates.rbegin()->first > 0) 
		physicalDevice = physicalDeviceCandidates.rbegin()->second; // Get the value.

	// Additional check for the physical device.
	if (physicalDevice == VK_NULL_HANDLE) throw std::runtime_error("Failed to find any GPUs that meet standard requirements.");
	return physicalDevice;
}

// If there are multiple GPUs and we want to use the 'best' one, we rank them here.
// This can also serve as a check for the physical device.
// If it doesn't meet any of these requirements of base requirements, returns 0.
int Vulkan::RankPhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	int score = 0; // If it's not suitable, we just return 0.

	// Get properties and features of this device.
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders, return 0 if not supported.
	if (!deviceFeatures.geometryShader) return 0;

	// We check the queue family to ensure that it can handle the operations we need.
	bool queueComplete = CheckQueueFamilies(device, surface).isComplete();
	if (!queueComplete) return 0; // return 0 if not supported.

	// Return weighted score, if it meets the general requirements.
	return score;
}

// Checks the queue families for our device, then returns the index that supports it.
// If it doesn't meet requirements, return -1.
// Extend this to search for specific families, so we have handles for each one.
S_QueueFamilies Vulkan::CheckQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	// Fill out this struct with a list of the families we need.
	S_QueueFamilies queueFamilyResults;

	// Query for queue families.
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// We set up a way to check for each property, and by the end choose the ones we need.
	for (int i = 0; i < (int)queueFamilyCount; i++)
	{
		bool graphics = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
		bool compute = queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
		bool transfer = queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
		bool sparse = queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT; // Related to sparse memory.
		bool protect = queueFamilies[i].queueFlags & VK_QUEUE_PROTECTED_BIT; // Related to protected memory.

		// We make sure it supports the graphics operations queue family.
		if (graphics) queueFamilyResults.graphicsFamily = i;
		else if (compute | transfer | sparse | protect) {} // Change this if we need it later.
		
		// We check if it has presentation support for the surface.
		VkBool32 presentation = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentation);
		if (presentation) queueFamilyResults.presentFamily = i;

		// If it's done filling out the queuefamily, we break.
		if (queueFamilyResults.isComplete()) break;
	}

	return queueFamilyResults;
}

VkDevice Vulkan::CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	// We check the queue family again of the chosen device, to get the index.
	S_QueueFamilies queueFamilyResults = CheckQueueFamilies(physicalDevice, surface);

	// Setup the queues first.
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { queueFamilyResults.graphicsFamily, queueFamilyResults.presentFamily };

	// For each queue, we fill in the createinfos.
	float queuePriority = 1.0f; // Value from 0.0 - 1.0. Required!
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily; // Queue family index.
		queueCreateInfo.queueCount = 1; // Right now, we're using 1 queue per family.
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Setup the logical device features.
	VkPhysicalDeviceFeatures deviceFeatures = {}; // We'll add features here later.

	// Fill in the device create info.
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); 
	createInfo.pQueueCreateInfos = queueCreateInfos.data(); // Point to the queue create info.
	createInfo.pEnabledFeatures = &deviceFeatures; // Link the device features we specify above.

	// We'll add in extensions later.
	createInfo.enabledExtensionCount = 0;

	// If we have validation layers,, we already verified them, so we add them in here now.
	if ((int)Vulkan::ValidationLayers.size() > 0)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(Vulkan::ValidationLayers.size());
		createInfo.ppEnabledLayerNames = Vulkan::ValidationLayers.data();
	}
	else createInfo.enabledLayerCount = 0;

	// Create the actual device now.
	VkDevice logicalDevice;
	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice); // TODO: Fill in custom allocator callback later.
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create logical device.");

	return logicalDevice;
}

VkQueue Vulkan::GetDeviceQueue(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice logicalDevice, int familyIndex)
{
	// We check the queue family again of the chosen device, to get the index.
	S_QueueFamilies queueFamily = CheckQueueFamilies(physicalDevice, surface);
	int family = queueFamily.family(familyIndex);
	
	// Get the device queue reference from the logical device.
	// Since queues are automatically created with the logical device, we can directly grab it.
	VkQueue deviceQueue;
	vkGetDeviceQueue(logicalDevice, family, 0, &deviceQueue); // Since we only added one queue per family, we just take the first.
	return deviceQueue;
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
	vkDestroyDevice(m_LogicalDevice, nullptr); // Destroy the device first.

	if ((int)Vulkan::ValidationLayers.size() > 0) 
		Vulkan::DestroyDebugReportCallbackEXT(m_Instance, m_DebugCallback, nullptr);

	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr); // Destroyed BEFORE instance.
	vkDestroyInstance(m_Instance, nullptr); // TODO: Fill in custom allocator callback later.

	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}
