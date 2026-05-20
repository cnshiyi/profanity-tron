#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
#include <set>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#include <OpenCL/cl_ext.h> // Included to get topology to get an actual unique identifier per device
#else
#include <CL/cl.h>
#include <CL/cl_ext.h> // Included to get topology to get an actual unique identifier per device
#endif

#define CL_DEVICE_PCI_BUS_ID_NV 0x4008
#define CL_DEVICE_PCI_SLOT_ID_NV 0x4009

#include "Dispatcher.hpp"
#include "ArgParser.hpp"
#include "Mode.hpp"
#include "help.hpp"
#include "OpenCLLoader.hpp"
#include "kernel_profanity.hpp"
#include "kernel_sha256.hpp"
#include "kernel_keccak.hpp"

std::string readFile(const char *const szFilename)
{
	std::ifstream in(szFilename, std::ios::in | std::ios::binary);
	std::ostringstream contents;
	contents << in.rdbuf();
	return contents.str();
}

std::vector<cl_device_id> getAllDevices(cl_device_type deviceType = CL_DEVICE_TYPE_GPU)
{
	std::vector<cl_device_id> vDevices;

	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, NULL, &platformIdCount);

	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), NULL);

	for (auto it = platformIds.cbegin(); it != platformIds.cend(); ++it)
	{
		cl_uint countDevice;
		clGetDeviceIDs(*it, deviceType, 0, NULL, &countDevice);

		std::vector<cl_device_id> deviceIds(countDevice);
		clGetDeviceIDs(*it, deviceType, countDevice, deviceIds.data(), &countDevice);

		std::copy(deviceIds.begin(), deviceIds.end(), std::back_inserter(vDevices));
	}

	return vDevices;
}

template <typename T, typename U, typename V, typename W>
T clGetWrapper(U function, V param, W param2)
{
	T t;
	function(param, param2, sizeof(t), &t, NULL);
	return t;
}

template <typename U, typename V, typename W>
std::string clGetWrapperString(U function, V param, W param2)
{
	size_t len;
	function(param, param2, 0, NULL, &len);
	char *const szString = new char[len];
	function(param, param2, len, szString, NULL);
	std::string r(szString);
	delete[] szString;
	return r;
}

template <typename T, typename U, typename V, typename W>
std::vector<T> clGetWrapperVector(U function, V param, W param2)
{
	size_t len;
	function(param, param2, 0, NULL, &len);
	len /= sizeof(T);
	std::vector<T> v;
	if (len > 0)
	{
		T *pArray = new T[len];
		function(param, param2, len * sizeof(T), pArray, NULL);
		for (size_t i = 0; i < len; ++i)
		{
			v.push_back(pArray[i]);
		}
		delete[] pArray;
	}
	return v;
}

unsigned int getUniqueDeviceIdentifier(const cl_device_id &deviceId)
{
#if defined(CL_DEVICE_TOPOLOGY_AMD)
	auto topology = clGetWrapper<cl_device_topology_amd>(clGetDeviceInfo, deviceId, CL_DEVICE_TOPOLOGY_AMD);
	if (topology.raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD)
	{
		return (topology.pcie.bus << 16) + (topology.pcie.device << 8) + topology.pcie.function;
	}
#endif
	cl_int bus_id = clGetWrapper<cl_int>(clGetDeviceInfo, deviceId, CL_DEVICE_PCI_BUS_ID_NV);
	cl_int slot_id = clGetWrapper<cl_int>(clGetDeviceInfo, deviceId, CL_DEVICE_PCI_SLOT_ID_NV);
	return (bus_id << 16) + slot_id;
}

template <typename T>
bool printResult(const T &t, const cl_int &err)
{
	std::cout << ((t == NULL) ? toString(err) : "Done") << std::endl;
	return t == NULL;
}

bool printResult(const cl_int err)
{
	std::cout << ((err != CL_SUCCESS) ? toString(err) : "Done") << std::endl;
	return err != CL_SUCCESS;
}

bool hasCliSwitch(int argc, char **argv, const std::initializer_list<const char *> &names)
{
	for (int i = 1; i < argc; ++i)
	{
		for (const auto &name : names)
		{
			if (std::string(argv[i]) == name)
			{
				return true;
			}
		}
	}

	return false;
}

size_t clampWorkgroupCandidate(size_t candidate, size_t maxWorkGroupSize)
{
	if (candidate == 0 || maxWorkGroupSize == 0)
	{
		return 0;
	}

	if (candidate > maxWorkGroupSize)
	{
		candidate = maxWorkGroupSize;
	}

	while (candidate > 1 && (maxWorkGroupSize % candidate) != 0)
	{
		candidate >>= 1;
	}

	return candidate;
}

void autoTuneForDevices(
	const std::vector<cl_device_id> &devices,
	const bool userSetWorksizeLocal,
	const bool userSetInverseMultiple,
	size_t &worksizeLocal,
	size_t &inverseMultiple)
{
	if (devices.empty() || (userSetWorksizeLocal && userSetInverseMultiple))
	{
		return;
	}

	size_t tunedWorksizeLocal = 0;
	size_t tunedInverseMultiple = inverseMultiple;

	for (const auto &deviceId : devices)
	{
		const auto vendor = clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_VENDOR);
		const auto name = clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_NAME);
		const auto maxWorkGroupSize = clGetWrapper<size_t>(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE);
		const auto globalMemSize = clGetWrapper<cl_ulong>(clGetDeviceInfo, deviceId, CL_DEVICE_GLOBAL_MEM_SIZE);
		const bool isNvidia = vendor.find("NVIDIA") != std::string::npos || name.find("NVIDIA") != std::string::npos;
		const bool isAmd = vendor.find("Advanced Micro Devices") != std::string::npos || vendor.find("AMD") != std::string::npos || name.find("AMD") != std::string::npos;
		const bool isIntel = vendor.find("Intel") != std::string::npos || name.find("Intel") != std::string::npos;
		const bool hasEnoughMemory = globalMemSize >= (cl_ulong)6 * 1024 * 1024 * 1024;
		size_t candidateWorksizeLocal = 64;

		if (isNvidia)
		{
			candidateWorksizeLocal = hasEnoughMemory ? 512 : 256;
		}
		else if (isAmd)
		{
			candidateWorksizeLocal = hasEnoughMemory ? 256 : 128;
		}
		else if (isIntel)
		{
			candidateWorksizeLocal = 64;
		}
		else
		{
			candidateWorksizeLocal = 64;
		}

		candidateWorksizeLocal = clampWorkgroupCandidate(candidateWorksizeLocal, maxWorkGroupSize);

		if (!userSetWorksizeLocal)
		{
			if (tunedWorksizeLocal == 0)
			{
				tunedWorksizeLocal = candidateWorksizeLocal;
			}
			else
			{
				tunedWorksizeLocal = (std::min)(tunedWorksizeLocal, candidateWorksizeLocal);
			}
		}

		if (!userSetInverseMultiple && isNvidia && hasEnoughMemory)
		{
			tunedInverseMultiple = (std::max)(tunedInverseMultiple, (size_t)32768);
		}
		else if (!userSetInverseMultiple && isAmd && hasEnoughMemory)
		{
			tunedInverseMultiple = (std::max)(tunedInverseMultiple, (size_t)24576);
		}
		else if (!userSetInverseMultiple && isIntel)
		{
			tunedInverseMultiple = (std::max)(tunedInverseMultiple, (size_t)8192);
		}
	}

	if (!userSetWorksizeLocal)
	{
		worksizeLocal = tunedWorksizeLocal == 0 ? worksizeLocal : tunedWorksizeLocal;
	}

	if (!userSetInverseMultiple)
	{
		inverseMultiple = tunedInverseMultiple;
	}
}

int main(int argc, char **argv)
{
	try
	{
		OpenCLLoader::ensureLoaded();
		ArgParser argp(argc, argv);
		bool bHelp = false;

		std::string matchingInput;
		std::string postUrl;
		std::vector<size_t> vDeviceSkipIndex;
		size_t worksizeLocal = 64;
		size_t worksizeMax = 0;
		size_t inverseSize = 255;
		size_t inverseMultiple = 16384;
		size_t prefixCount = 0;
		size_t suffixCount = 6;
		size_t quitCount = 0;
		const bool userSetWorksizeLocal = hasCliSwitch(argc, argv, {"-w", "--work"});
		const bool userSetWorksizeMax = hasCliSwitch(argc, argv, {"-W", "--work-max"});
		const bool userSetInverseMultiple = hasCliSwitch(argc, argv, {"-I", "--inverse-multiple"});

		argp.addSwitch('h', "help", bHelp);
		argp.addSwitch('m', "matching", matchingInput);
		argp.addSwitch('w', "work", worksizeLocal);
		argp.addSwitch('W', "work-max", worksizeMax);
		argp.addSwitch('p', "post", postUrl);
		argp.addSwitch('i', "inverse-size", inverseSize);
		argp.addSwitch('I', "inverse-multiple", inverseMultiple);
		argp.addSwitch('b', "prefix-count", prefixCount);
		argp.addSwitch('e', "suffix-count", suffixCount);
		argp.addSwitch('q', "quit-count", quitCount);
		argp.addMultiSwitch('s', "skip", vDeviceSkipIndex);

		if (!argp.parse())
		{
			std::cout << "error: bad arguments, try again :<" << std::endl;
			return 1;
		}

		if (bHelp)
		{
			std::cout << g_strHelp << std::endl;
			return 0;
		}

		if (matchingInput.empty())
		{
			std::cout << "error: matching file must be specified :<" << std::endl;
			return 1;
		}

		if (!postUrl.empty())
		{
			std::cout << "error: --post has been disabled for security reasons because it can leak private keys :<" << std::endl;
			return 1;
		}

		if (prefixCount < 0)
		{
			prefixCount = 0;
		}

		if (prefixCount > 10)
		{
			std::cout << "error: the number of prefix matches cannot be greater than 10 :<" << std::endl;
			return 1;
		}

		if (suffixCount < 0)
		{
			suffixCount = 6;
		}

		if (suffixCount > 10)
		{
			std::cout << "error: the number of suffix matches cannot be greater than 10 :<" << std::endl;
			return 1;
		}

		Mode mode = Mode::matching(matchingInput);

		if (mode.matchingCount <= 0)
		{
			std::cout << "error: please check your matching file to make sure the path and format are correct :<" << std::endl;
			return 1;
		}

		mode.prefixCount = prefixCount;
		mode.suffixCount = suffixCount;

		std::vector<cl_device_id> vFoundDevices = getAllDevices();
		std::vector<cl_device_id> vDevices;
		std::map<cl_device_id, size_t> mDeviceIndex;

		cl_int errorCode;

		std::cout << "Devices:" << std::endl;
		for (size_t i = 0; i < vFoundDevices.size(); ++i)
		{
			if (std::find(vDeviceSkipIndex.begin(), vDeviceSkipIndex.end(), i) != vDeviceSkipIndex.end())
			{
				continue;
			}
			cl_device_id &deviceId = vFoundDevices[i];
			const auto strName = clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_NAME);
			const auto computeUnits = clGetWrapper<cl_uint>(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_COMPUTE_UNITS);
			const auto globalMemSize = clGetWrapper<cl_ulong>(clGetDeviceInfo, deviceId, CL_DEVICE_GLOBAL_MEM_SIZE);
			std::cout << "  GPU-" << i << ": " << strName << ", " << globalMemSize << " bytes available, " << computeUnits << " compute units" << std::endl;
			vDevices.push_back(vFoundDevices[i]);
			mDeviceIndex[vFoundDevices[i]] = i;
		}

		if (vDevices.empty())
		{
			return 1;
		}

		autoTuneForDevices(vDevices, userSetWorksizeLocal, userSetInverseMultiple, worksizeLocal, inverseMultiple);

		if (!userSetWorksizeLocal || !userSetInverseMultiple || !userSetWorksizeMax)
		{
			std::cout << std::endl;
			std::cout << "Tuning:" << std::endl;
			std::cout << "  work = " << worksizeLocal << std::endl;
			std::cout << "  inverse-multiple = " << inverseMultiple << std::endl;
			std::cout << "  work-max = " << (worksizeMax == 0 ? inverseSize * inverseMultiple : worksizeMax) << std::endl;
		}

		std::cout << std::endl;
		std::cout << "OpenCL:" << std::endl;
		std::cout << "  Context creating ..." << std::flush;
		auto clContext = clCreateContext(NULL, vDevices.size(), vDevices.data(), NULL, NULL, &errorCode);
		if (printResult(clContext, errorCode))
		{
			return 1;
		}

		cl_program clProgram;
		std::cout << "  Kernel compiling ..." << std::flush;
		const char *szKernels[] = { kernel_keccak.c_str(), kernel_sha256.c_str(), kernel_profanity.c_str() };
		clProgram = clCreateProgramWithSource(clContext, sizeof(szKernels) / sizeof(char *), szKernels, NULL, &errorCode);
		if (printResult(clProgram, errorCode))
		{
			return 1;
		}

		// Build the program
		std::cout << "  Program building ..." << std::flush;
		const std::string strBuildOptions = "-D PROFANITY_INVERSE_SIZE=" + toString(inverseSize) + " -D PROFANITY_MAX_SCORE=" + toString(PROFANITY_MAX_SCORE);
		if (printResult(clBuildProgram(clProgram, vDevices.size(), vDevices.data(), strBuildOptions.c_str(), NULL, NULL)))
		{
			return 1;
		}

		std::cout << std::endl;

		Dispatcher d(clContext, clProgram, mode, worksizeMax == 0 ? inverseSize * inverseMultiple : worksizeMax, inverseSize, inverseMultiple, quitCount, postUrl);

		for (auto &i : vDevices)
		{
			d.addDevice(i, worksizeLocal, mDeviceIndex[i]);
		}

		d.run();
		clReleaseContext(clContext);
		return 0;
	}
	catch (std::runtime_error &e)
	{
		std::cout << "std::runtime_error - " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception occured" << std::endl;
	}

	return 1;
}
