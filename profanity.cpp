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
#include <cctype>
#include <limits>

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
		cl_uint countDevice = 0;
		const cl_int countResult = clGetDeviceIDs(*it, deviceType, 0, NULL, &countDevice);
		if (countResult != CL_SUCCESS || countDevice == 0)
		{
			continue;
		}

		std::vector<cl_device_id> deviceIds(countDevice);
		const cl_int listResult = clGetDeviceIDs(*it, deviceType, countDevice, deviceIds.data(), &countDevice);
		if (listResult != CL_SUCCESS)
		{
			continue;
		}

		std::copy(deviceIds.begin(), deviceIds.end(), std::back_inserter(vDevices));
	}

	return vDevices;
}

std::vector<cl_device_id> getCpuDevicesForPlatform(cl_platform_id platformId)
{
	cl_uint countDevice = 0;
	const cl_int countResult = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_CPU, 0, NULL, &countDevice);
	if (countResult != CL_SUCCESS || countDevice == 0)
	{
		return {};
	}

	std::vector<cl_device_id> deviceIds(countDevice);
	const cl_int listResult = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_CPU, countDevice, deviceIds.data(), &countDevice);
	if (listResult != CL_SUCCESS)
	{
		return {};
	}
	return deviceIds;
}

cl_platform_id getDevicePlatform(cl_device_id deviceId)
{
	cl_platform_id platformId = NULL;
	clGetDeviceInfo(deviceId, CL_DEVICE_PLATFORM, sizeof(platformId), &platformId, NULL);
	return platformId;
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

static bool isBlank(const std::string &value)
{
	return value.find_first_not_of(" \t\r\n") == std::string::npos;
}

static std::string normalizePrivateKeyHex(const std::string &input, const char *name)
{
	std::string text;
	for (const unsigned char c : input)
	{
		if (!std::isspace(c))
		{
			text.push_back(static_cast<char>(c));
		}
	}

	if (text.size() >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
	{
		text.erase(0, 2);
	}

	if (text.empty())
	{
		return text;
	}

	if (text.size() > 64)
	{
		throw std::runtime_error(std::string(name) + " must be at most 64 hex chars");
	}

	for (const unsigned char c : text)
	{
		if (!std::isxdigit(c))
		{
			throw std::runtime_error(std::string(name) + " must be hex");
		}
	}

	return std::string(64 - text.size(), '0') + text;
}

static cl_ulong parseHex64(const std::string &hex, size_t offset)
{
	cl_ulong value = 0;
	for (size_t i = 0; i < 16; ++i)
	{
		const char c = hex[offset + i];
		unsigned int n = 0;
		if (c >= '0' && c <= '9')
		{
			n = c - '0';
		}
		else if (c >= 'a' && c <= 'f')
		{
			n = 10 + c - 'a';
		}
		else
		{
			n = 10 + c - 'A';
		}
		value = (value << 4) | n;
	}
	return value;
}

static cl_ulong4 hexToClUlong4(const std::string &hex)
{
	cl_ulong4 value;
	value.s[3] = parseHex64(hex, 0);
	value.s[2] = parseHex64(hex, 16);
	value.s[1] = parseHex64(hex, 32);
	value.s[0] = parseHex64(hex, 48);
	return value;
}

static std::string clUlong4ToHex(const cl_ulong4 &value)
{
	std::ostringstream ss;
	ss << std::hex << std::setfill('0')
	   << std::setw(16) << value.s[3]
	   << std::setw(16) << value.s[2]
	   << std::setw(16) << value.s[1]
	   << std::setw(16) << value.s[0];
	return ss.str();
}

static cl_ulong makeNibbleMask(unsigned int nibbles)
{
	if (nibbles == 0)
	{
		return 0;
	}
	if (nibbles >= 16)
	{
		return (std::numeric_limits<cl_ulong>::max)();
	}
	return (1ull << (nibbles * 4)) - 1ull;
}

static int compareHexStrings(const std::string &a, const std::string &b)
{
	for (size_t i = 0; i < a.size() && i < b.size(); ++i)
	{
		const char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
		const char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(b[i])));
		if (ca < cb)
		{
			return -1;
		}
		if (ca > cb)
		{
			return 1;
		}
	}
	if (a.size() < b.size())
	{
		return -1;
	}
	if (a.size() > b.size())
	{
		return 1;
	}
	return 0;
}

static SearchRange buildSearchRange(
	const std::string &rangeStartInput,
	const std::string &rangeEndInput,
	const std::string &rangeDirectionInput)
{
	SearchRange range;
	const bool hasStart = !isBlank(rangeStartInput);
	const bool hasEnd = !isBlank(rangeEndInput);
	const bool hasDirection = !isBlank(rangeDirectionInput);
	if (!hasStart && !hasEnd && !hasDirection)
	{
		return range;
	}

	if (!hasStart || !hasEnd)
	{
		throw std::runtime_error("--range-start and --range-end must be both set, or both empty");
	}

	std::string direction = rangeDirectionInput;
	std::transform(direction.begin(), direction.end(), direction.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if (direction.empty())
	{
		direction = "up";
	}
	if (direction != "up" && direction != "down")
	{
		throw std::runtime_error("--range-direction must be up or down");
	}
	const bool descending = direction == "down";

	const std::string startHex = normalizePrivateKeyHex(rangeStartInput, "--range-start");
	const std::string endHex = normalizePrivateKeyHex(rangeEndInput, "--range-end");
	if (startHex.empty() || endHex.empty())
	{
		throw std::runtime_error("--range-start and --range-end cannot be blank when range mode is enabled");
	}

	size_t firstDiff = 64;
	size_t lastDiff = 0;
	for (size_t i = 0; i < 64; ++i)
	{
		if (std::tolower(static_cast<unsigned char>(startHex[i])) != std::tolower(static_cast<unsigned char>(endHex[i])))
		{
			firstDiff = (std::min)(firstDiff, i);
			lastDiff = i;
		}
	}

	const int order = compareHexStrings(startHex, endHex);
	if (!descending && order > 0)
	{
		throw std::runtime_error("up range requires start <= end");
	}
	if (descending && order < 0)
	{
		throw std::runtime_error("down range requires start >= end");
	}

	if (firstDiff == 64)
	{
		throw std::runtime_error("range start and end are identical");
	}

	if (lastDiff > 15)
	{
		throw std::runtime_error("current range mode supports changing only the first 16 hex chars of the private key");
	}

	const unsigned int variableNibbles = static_cast<unsigned int>(lastDiff - firstDiff + 1);
	if (variableNibbles > 16)
	{
		throw std::runtime_error("range window cannot be wider than 16 hex chars");
	}
	const unsigned int lowFixedNibblesInHighWord = 15u - static_cast<unsigned int>(lastDiff);
	if (lowFixedNibblesInHighWord > 31)
	{
		throw std::runtime_error("range window is too low for foundId-based control");
	}

	for (size_t i = 0; i < 64; ++i)
	{
		if (i < firstDiff || i > lastDiff)
		{
			if (std::tolower(static_cast<unsigned char>(startHex[i])) != std::tolower(static_cast<unsigned char>(endHex[i])))
			{
				throw std::runtime_error("range start/end must differ only inside one continuous hex window");
			}
		}
	}

	range.enabled = true;
	range.descending = descending;
	range.hexFirst = static_cast<unsigned int>(firstDiff);
	range.hexLast = static_cast<unsigned int>(lastDiff);
	range.start = hexToClUlong4(startHex);
	range.end = hexToClUlong4(endHex);

	const cl_ulong variableMask = makeNibbleMask(variableNibbles) << (lowFixedNibblesInHighWord * 4);
	const cl_ulong counterLow = (range.start.s[3] & variableMask) >> (lowFixedNibblesInHighWord * 4);
	const cl_ulong counterEnd = (range.end.s[3] & variableMask) >> (lowFixedNibblesInHighWord * 4);
	const cl_ulong counterDistance = range.descending ? (counterLow - counterEnd) : (counterEnd - counterLow);
	range.counterMask = makeNibbleMask(variableNibbles);
	range.counterMax = counterDistance == (std::numeric_limits<cl_ulong>::max)() ? counterDistance : (counterDistance + 1);
	range.prefixHigh = range.start.s[3] & ~variableMask;
	range.counterStart = counterLow;
	range.counterShift = lowFixedNibblesInHighWord * 4;

	std::cout << "Range:" << std::endl;
	std::cout << "  start = " << clUlong4ToHex(range.start) << std::endl;
	std::cout << "  end = " << clUlong4ToHex(range.end) << std::endl;
	std::cout << "  direction = " << (range.descending ? "down" : "up") << std::endl;
	std::cout << "  variable hex = " << (range.hexFirst + 1) << "-" << (range.hexLast + 1) << std::endl;
	if (counterDistance == (std::numeric_limits<cl_ulong>::max)())
	{
		std::cout << "  max steps = 18446744073709551616" << std::endl;
	}
	else
	{
		std::cout << "  max steps = " << range.counterMax << std::endl;
	}
	std::cout << "  note = range mode constrains the first 64-bit private-key lane" << std::endl;

	return range;
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
		std::string rangeStartInput;
		std::string rangeEndInput;
		std::string rangeDirectionInput;
		std::vector<size_t> vDeviceSkipIndex;
		size_t worksizeLocal = 64;
		size_t worksizeMax = 0;
		size_t inverseSize = 255;
		size_t inverseMultiple = 16384;
		size_t prefixCount = 0;
		size_t suffixCount = 6;
		size_t quitCount = 0;
		size_t benchmarkSeconds = 0;
		size_t cpuAssistInverseMultiple = 128;
		bool noCache = false;
		bool cpuAssist = false;
		const bool userSetWorksizeLocal = hasCliSwitch(argc, argv, {"-w", "--work"});
		const bool userSetWorksizeMax = hasCliSwitch(argc, argv, {"-W", "--work-max"});
		const bool userSetInverseMultiple = hasCliSwitch(argc, argv, {"-I", "--inverse-multiple"});

		argp.addSwitch('h', "help", bHelp);
		argp.addSwitch('m', "matching", matchingInput);
		argp.addSwitch('o', "output", resultsPath);
		argp.addSwitch('x', "range-start", rangeStartInput);
		argp.addSwitch('y', "range-end", rangeEndInput);
		argp.addSwitch('z', "range-direction", rangeDirectionInput);
		argp.addSwitch('w', "work", worksizeLocal);
		argp.addSwitch('W', "work-max", worksizeMax);
		argp.addSwitch('i', "inverse-size", inverseSize);
		argp.addSwitch('I', "inverse-multiple", inverseMultiple);
		argp.addSwitch('b', "prefix-count", prefixCount);
		argp.addSwitch('e', "suffix-count", suffixCount);
		argp.addSwitch('q', "quit-count", quitCount);
		argp.addSwitch('t', "benchmark-seconds", benchmarkSeconds);
		argp.addSwitch('n', "no-cache", noCache);
		argp.addSwitch('c', "cpu-assist", cpuAssist);
		argp.addSwitch('C', "cpu-assist-inverse-multiple", cpuAssistInverseMultiple);
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

		SearchRange searchRange = buildSearchRange(rangeStartInput, rangeEndInput, rangeDirectionInput);

		if (prefixCount > 10)
		{
			std::cout << "error: the number of prefix matches cannot be greater than 10 :<" << std::endl;
			return 1;
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
		std::vector<cl_device_id> vCpuAssistDevices;
		std::map<cl_device_id, size_t> mDeviceIndex;
		std::map<cl_device_id, std::string> mDeviceLabel;
		std::map<cl_device_id, size_t> mDeviceInverseMultiple;

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
			mDeviceLabel[vFoundDevices[i]] = "GPU" + toString(i);
		}

		if (vDevices.empty())
		{
			return 1;
		}

		const std::vector<cl_device_id> vGpuDevices = vDevices;

		if (cpuAssist)
		{
			std::set<cl_platform_id> gpuPlatforms;
			for (const auto &gpuDevice : vDevices)
			{
				gpuPlatforms.insert(getDevicePlatform(gpuDevice));
			}

			size_t cpuIndex = 0;
			for (const auto &platformId : gpuPlatforms)
			{
				const auto cpuDevices = getCpuDevicesForPlatform(platformId);
				for (const auto &cpuDevice : cpuDevices)
				{
					const auto strName = clGetWrapperString(clGetDeviceInfo, cpuDevice, CL_DEVICE_NAME);
					const auto computeUnits = clGetWrapper<cl_uint>(clGetDeviceInfo, cpuDevice, CL_DEVICE_MAX_COMPUTE_UNITS);
					const auto globalMemSize = clGetWrapper<cl_ulong>(clGetDeviceInfo, cpuDevice, CL_DEVICE_GLOBAL_MEM_SIZE);
					std::cout << "  CPU-" << cpuIndex << ": " << strName << ", " << globalMemSize << " bytes available, " << computeUnits << " compute units (assist)" << std::endl;
					vDevices.push_back(cpuDevice);
					vCpuAssistDevices.push_back(cpuDevice);
					mDeviceIndex[cpuDevice] = cpuIndex;
					mDeviceLabel[cpuDevice] = "CPU" + toString(cpuIndex);
					mDeviceInverseMultiple[cpuDevice] = cpuAssistInverseMultiple;
					++cpuIndex;
				}
			}
			if (vCpuAssistDevices.empty())
			{
				std::cout << "  CPU assist requested, but no OpenCL CPU device was found on the selected GPU platform" << std::endl;
			}
		}

		autoTuneForDevices(vGpuDevices, userSetWorksizeLocal, userSetInverseMultiple, worksizeLocal, inverseMultiple);

		if (!userSetWorksizeLocal || !userSetInverseMultiple || !userSetWorksizeMax)
		{
			std::cout << std::endl;
			std::cout << "Tuning:" << std::endl;
			std::cout << "  work = " << worksizeLocal << std::endl;
			std::cout << "  inverse-multiple = " << inverseMultiple << std::endl;
			std::cout << "  work-max = " << (worksizeMax == 0 ? inverseSize * inverseMultiple : worksizeMax) << std::endl;
			if (cpuAssist && !vCpuAssistDevices.empty())
			{
				std::cout << "  cpu-assist-inverse-multiple = " << cpuAssistInverseMultiple << std::endl;
			}
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

		Dispatcher d(clContext, clProgram, mode, worksizeMax == 0 ? inverseSize * inverseMultiple : worksizeMax, inverseSize, inverseMultiple, quitCount, benchmarkSeconds, resultsPath, searchRange);

		for (auto &i : vDevices)
		{
			const auto cpuAssistIt = mDeviceInverseMultiple.find(i);
			const bool isCpuAssistDevice = cpuAssistIt != mDeviceInverseMultiple.end();
			d.addDevice(i, isCpuAssistDevice ? 0 : worksizeLocal, mDeviceIndex[i], mDeviceLabel[i], isCpuAssistDevice ? cpuAssistIt->second : 0);
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
