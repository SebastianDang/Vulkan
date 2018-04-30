#pragma once

class Vulkan
{
public:
	void Run();

private:

	void InitVulkan();
	void MainLoop();
	void Cleanup();

};