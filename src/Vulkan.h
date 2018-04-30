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

	void Run();


private:

	GLFWwindow * m_pWindow = nullptr;
	VkInstance m_Instance;

	void InitWindow(int width, int height, const char *title);
	void InitVulkan();
	void CreateInstance(const char *appName, const char *engineName);
	bool CheckGLFWExtensionSupport(const char ** glfwExtensions, int glfwExtensionCount);

	void MainLoop();
	void Cleanup();

};

#endif