#ifndef HPP_OPENCLLOADER
#define HPP_OPENCLLOADER

#ifdef _WIN32

#include <windows.h>
#include <stdexcept>
#include <string>

class OpenCLLoader {
	public:
		static void ensureLoaded() {
			static OpenCLLoader loader;
			(void)loader;
		}

	private:
		OpenCLLoader() {
			HMODULE module = LoadLibraryA("OpenCL.dll");
			if (module == NULL) {
				throw std::runtime_error("failed to load OpenCL.dll");
			}
		}
};

#else

class OpenCLLoader {
	public:
		static void ensureLoaded() {}
};

#endif

#endif /* HPP_OPENCLLOADER */
