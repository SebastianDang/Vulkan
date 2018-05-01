#pragma once

#ifndef VULKAN_H
#define VULKAN_H

class Vulkan
{
public:

	// Window size properties
	static std::string Title;
	static int Width;
	static int Height;
	static std::vector<const char*> ValidationLayers; 

	void Run();

private:

	GLFWwindow * m_pWindow = nullptr;
	VkInstance m_Instance;
	VkDebugReportCallbackEXT m_DebugCallback;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;



	void InitWindow(int width, int height, const char *title);
	void InitVulkan();

	// Functions for setting up the instance and verifying extensions and layers.
	void CreateInstance(const char *appName, const char *engineName);
	bool CheckGLFWExtensionSupport(const char ** glfwExtensions, int glfwExtensionCount);
	bool CheckValidationLayerSupport(std::vector<const char*> validationLayers);

	// Functions for setting up the debug callback.
	void SetupDebugCallback();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
		uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
	static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, 
		const VkAllocationCallbacks* pAllocator);

	// Functions for initializing physical devices.
	void InitPhysicalDevice();
	int RankPhysicalDevice(VkPhysicalDevice device);
	bool CheckPhysicalDevice(VkPhysicalDevice device);




	void MainLoop();
	void Cleanup();

};

#endif