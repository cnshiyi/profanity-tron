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
#include <iomanip>

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
#include "KernelSources.hpp"
#include "Mode.hpp"
#include "help.hpp"
#include "OpenCLLoader.hpp"

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

static std::string sanitizeFileComponent(const std::string &input)
{
	std::string output;
	output.reserve(input.size());
	for (const unsigned char c : input)
	{
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		{
			output.push_back((char)c);
		}
		else
		{
			output.push_back('_');
		}
	}
	return output;
}

static std::string makeProgramCacheKey(
	const std::vector<cl_device_id> &devices,
	const std::string &buildOptions,
	const std::string &kernelKeccak,
	const std::string &kernelSha256,
	const std::string &kernelProfanity)
{
	std::ostringstream ss;
	ss << buildOptions << '\n' << kernelKeccak << '\n' << kernelSha256 << '\n' << kernelProfanity;
	for (const auto &deviceId : devices)
	{
		ss << '\n' << clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_VENDOR);
		ss << '\n' << clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_NAME);
		ss << '\n' << clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_VERSION);
		ss << '\n' << clGetWrapperString(clGetDeviceInfo, deviceId, CL_DRIVER_VERSION);
	}

	const auto text = ss.str();
	unsigned long long hash = 1469598103934665603ull;
	for (const unsigned char c : text)
	{
		hash ^= c;
		hash *= 1099511628211ull;
	}

	std::ostringstream hex;
	hex << std::hex << std::setfill('0') << std::setw(16) << hash;
	return hex.str();
}

static std::string makeProgramCachePath(const std::vector<cl_device_id> &devices, const std::string &cacheKey)
{
	std::string deviceName = devices.empty() ? "unknown" : clGetWrapperString(clGetDeviceInfo, devices[0], CL_DEVICE_NAME);
	return "cache-opencl." + sanitizeFileComponent(deviceName) + "." + cacheKey + ".bin";
}

static cl_program tryLoadProgramBinary(
	cl_context context,
	const std::vector<cl_device_id> &devices,
	const std::string &cachePath)
{
	if (devices.empty())
	{
		return NULL;
	}

	std::ifstream in(cachePath, std::ios::in | std::ios::binary);
	if (!in.is_open())
	{
		return NULL;
	}

	std::vector<unsigned char> binary((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	if (binary.empty())
	{
		return NULL;
	}

	size_t binarySize = binary.size();
	const unsigned char *binaryPtr = binary.data();
	cl_int binaryStatus = CL_SUCCESS;
	cl_int errorCode = CL_SUCCESS;
	cl_program program = clCreateProgramWithBinary(context, (cl_uint)devices.size(), devices.data(), &binarySize, &binaryPtr, &binaryStatus, &errorCode);
	if (errorCode != CL_SUCCESS || binaryStatus != CL_SUCCESS)
	{
		if (program != NULL)
		{
			clReleaseProgram(program);
		}
		return NULL;
	}

	return program;
}

static void saveProgramBinary(cl_program program, const std::string &cachePath)
{
	size_t sizeCount = 0;
	clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(sizeCount), &sizeCount, NULL);
	if (sizeCount == 0)
	{
		return;
	}

	std::vector<size_t> binarySizes(sizeCount, 0);
	clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * binarySizes.size(), binarySizes.data(), NULL);
	if (binarySizes[0] == 0)
	{
		return;
	}

	std::vector<std::vector<unsigned char>> binaries(binarySizes.size());
	std::vector<unsigned char *> binaryPtrs(binarySizes.size(), NULL);
	for (size_t i = 0; i < binarySizes.size(); ++i)
	{
		if (binarySizes[i] == 0)
		{
			continue;
		}
		binaries[i].resize(binarySizes[i]);
		binaryPtrs[i] = binaries[i].data();
	}
	clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(unsigned char *) * binaryPtrs.size(), binaryPtrs.data(), NULL);

	std::ofstream out(cachePath, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out.is_open())
	{
		return;
	}

	out.write(reinterpret_cast<const char *>(binaries[0].data()), (std::streamsize)binaries[0].size());
}

void printBuildLogs(cl_program program, const std::vector<cl_device_id> &devices)
{
	for (const auto &deviceId : devices)
	{
		size_t logSize = 0;
		clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
		if (logSize == 0)
		{
			continue;
		}

		std::vector<char> log(logSize + 1, 0);
		clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, logSize, log.data(), NULL);
		std::cout << log.data() << std::endl;
	}
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
		const bool isIntel = vendor.find("Intel") != std::string::npos || name.find("Intel") != std::string::npos;
		const bool hasEnoughMemory = globalMemSize >= (cl_ulong)6 * 1024 * 1024 * 1024;
		const bool hasLargeBatchMemory = globalMemSize >= (cl_ulong)15 * 1024 * 1024 * 1024 / 2;
		size_t candidateWorksizeLocal = 64;

		if (isNvidia)
		{
			candidateWorksizeLocal = hasLargeBatchMemory ? 128 : (hasEnoughMemory ? 512 : 256);
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

		if (!userSetInverseMultiple && isNvidia && hasLargeBatchMemory)
		{
			tunedInverseMultiple = (std::max)(tunedInverseMultiple, (size_t)237568);
		}
		else if (!userSetInverseMultiple && isNvidia && hasEnoughMemory)
		{
			tunedInverseMultiple = (std::max)(tunedInverseMultiple, (size_t)32768);
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
		std::string resultsPath;
		std::vector<size_t> vDeviceSkipIndex;
		size_t worksizeLocal = 64;
		size_t worksizeMax = 0;
		size_t inverseSize = 255;
		size_t inverseMultiple = 16384;
		size_t prefixCount = 0;
		size_t suffixCount = 6;
		size_t quitCount = 0;
		size_t benchmarkSeconds = 0;
		bool noCache = false;
		const bool userSetWorksizeLocal = hasCliSwitch(argc, argv, {"-w", "--work"});
		const bool userSetWorksizeMax = hasCliSwitch(argc, argv, {"-W", "--work-max"});
		const bool userSetInverseMultiple = hasCliSwitch(argc, argv, {"-I", "--inverse-multiple"});

		argp.addSwitch('h', "help", bHelp);
		argp.addSwitch('m', "matching", matchingInput);
		argp.addSwitch('o', "output", resultsPath);
		argp.addSwitch('w', "work", worksizeLocal);
		argp.addSwitch('W', "work-max", worksizeMax);
		argp.addSwitch('i', "inverse-size", inverseSize);
		argp.addSwitch('I', "inverse-multiple", inverseMultiple);
		argp.addSwitch('b', "prefix-count", prefixCount);
		argp.addSwitch('e', "suffix-count", suffixCount);
		argp.addSwitch('q', "quit-count", quitCount);
		argp.addSwitch('t', "benchmark-seconds", benchmarkSeconds);
		argp.addSwitch('n', "no-cache", noCache);
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

		if (suffixCount > 12)
		{
			std::cout << "error: the number of suffix matches cannot be greater than 12 :<" << std::endl;
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
			const auto strVendor = clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_VENDOR);
			const bool isAmd = strVendor.find("Advanced Micro Devices") != std::string::npos || strVendor.find("AMD") != std::string::npos || strName.find("AMD") != std::string::npos;
			if (isAmd)
			{
				std::cout << "  GPU-" << i << ": " << strName << " skipped (AMD support removed)" << std::endl;
				continue;
			}
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
		const std::string kernelKeccak = loadKernelSource(".", "kernel_keccak.cl");
		const std::string kernelSha256 = loadKernelSource(".", "kernel_sha256.cl");
		const std::string kernelProfanity = loadKernelSource(".", "kernel_profanity.cl");
		const std::string strBuildOptions = "-D PROFANITY_INVERSE_SIZE=" + toString(inverseSize) + " -D PROFANITY_MAX_SCORE=" + toString(PROFANITY_MAX_SCORE);
		const std::string cacheKey = makeProgramCacheKey(vDevices, strBuildOptions, kernelKeccak, kernelSha256, kernelProfanity);
		const std::string cachePath = makeProgramCachePath(vDevices, cacheKey);
		const bool allowCache = !noCache;

		clProgram = allowCache ? tryLoadProgramBinary(clContext, vDevices, cachePath) : NULL;
		if (clProgram == NULL)
		{
			const char *szKernels[] = { kernelKeccak.c_str(), kernelSha256.c_str(), kernelProfanity.c_str() };
			clProgram = clCreateProgramWithSource(clContext, sizeof(szKernels) / sizeof(char *), szKernels, NULL, &errorCode);
			if (printResult(clProgram, errorCode))
			{
				return 1;
			}
		}
		else
		{
			std::cout << "Done (cache)" << std::endl;
		}

		std::cout << "  Program building ..." << std::flush;
		const auto buildResult = clBuildProgram(clProgram, vDevices.size(), vDevices.data(), strBuildOptions.c_str(), NULL, NULL);
		if (buildResult != CL_SUCCESS)
		{
			printBuildLogs(clProgram, vDevices);
		}
		if (printResult(buildResult))
		{
			return 1;
		}
		if (allowCache)
		{
			saveProgramBinary(clProgram, cachePath);
		}

		std::cout << std::endl;

		Dispatcher d(clContext, clProgram, mode, worksizeMax == 0 ? inverseSize * inverseMultiple : worksizeMax, inverseSize, inverseMultiple, quitCount, benchmarkSeconds, resultsPath);

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
