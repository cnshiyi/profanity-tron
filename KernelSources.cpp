#include "KernelSources.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::string loadKernelSource(const std::string &rootPath, const std::string &fileName)
{
	const std::string path = rootPath + "\\kernels\\" + fileName;
	std::ifstream in(path, std::ios::in | std::ios::binary);
	if (!in.is_open())
	{
		throw std::runtime_error("failed to open kernel source: " + path);
	}

	std::ostringstream contents;
	contents << in.rdbuf();
	return contents.str();
}
