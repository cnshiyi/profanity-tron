#include "Dispatcher.hpp"
#include "picosha2.h"
// Includes
#include <stdexcept>
#include <iostream>
#include <thread>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <array>
#include <tuple>
#include <ctime>

#include "precomp.hpp"
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#elif defined(__APPLE__) || defined(__MACOSX)
#include <Security/Security.h>
#else
#include <fstream>
#endif

static const uint8_t base58Alphabet[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static cl_uchar base58IndexOf(const cl_uchar c)
{
	for (cl_uchar i = 0; i < 58; ++i)
	{
		if (base58Alphabet[i] == c)
		{
			return i;
		}
	}
	return 0xff;
}

static std::string base58Encode(const std::vector<uint8_t> &data)
{
	std::vector<uint8_t> digits((data.size() * 138 / 100) + 1);
	size_t digitslen = 1;
	for (size_t i = 0; i < data.size(); i++)
	{
		uint32_t carry = static_cast<uint32_t>(data[i]);
		for (size_t j = 0; j < digitslen; j++)
		{
			carry = carry + static_cast<uint32_t>(digits[j] << 8);
			digits[j] = static_cast<uint8_t>(carry % 58);
			carry /= 58;
		}
		for (; carry; carry /= 58)
		{
			digits[digitslen++] = static_cast<uint8_t>(carry % 58);
		}
	}
	std::string result;
	for (size_t i = 0; i < (data.size() - 1) && !data[i]; i++)
	{
		result.push_back(base58Alphabet[0]);
	}
	for (size_t i = 0; i < digitslen; i++)
	{
		result.push_back(base58Alphabet[digits[digitslen - 1 - i]]);
	}
	return result;
}

static std::string hexToStr(const std::string &str)
{
	std::string result;
	for (size_t i = 0; i < str.length(); i += 2)
	{
		std::string byte = str.substr(i, 2);
		char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
		result.push_back(chr);
	}
	return result;
}

static std::string toHex(const uint8_t *const s, const size_t len)
{
	std::string b("0123456789abcdef");
	std::string r;

	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char h = s[i] / 16;
		const unsigned char l = s[i] % 16;
		r = r + b.substr(h, 1) + b.substr(l, 1);
	}

	return r;
}

static std::string toTron(const std::string &str)
{
	std::string tronHex = "41" + str;
	std::string tronFirstHashStr = hexToStr(tronHex);
	std::string tronFirstHashRes = picosha2::hash256_hex_string(tronFirstHashStr);
	std::string tronSecondHashStr = hexToStr(tronFirstHashRes);
	std::string tronSecondHashRes = picosha2::hash256_hex_string(tronSecondHashStr);
	std::string tronChecksum = tronSecondHashRes.substr(0, 8);
	std::string tronAddressHex = tronHex + tronChecksum;
	std::string tronAddressStr = hexToStr(tronAddressHex);
	std::vector<uint8_t> vec(tronAddressStr.c_str(), tronAddressStr.c_str() + tronAddressStr.size());
	std::string tronAddressBase58 = base58Encode(vec);
	return tronAddressBase58;
}

unsigned int getKernelExecutionTimeMicros(cl_event &e)
{
	cl_ulong timeStart = 0, timeEnd = 0;
	clWaitForEvents(1, &e);
	clGetEventProfilingInfo(e, CL_PROFILING_COMMAND_START, sizeof(timeStart), &timeStart, NULL);
	clGetEventProfilingInfo(e, CL_PROFILING_COMMAND_END, sizeof(timeEnd), &timeEnd, NULL);
	return (timeEnd - timeStart) / 1000;
}

Dispatcher::OpenCLException::OpenCLException(
	const std::string s,
	const cl_int res)
	: std::runtime_error(s + " (res = " + toString(res) + ")"),
	  m_res(res)
{
}

void Dispatcher::OpenCLException::OpenCLException::throwIfError(const std::string s, const cl_int res)
{
	if (res != CL_SUCCESS)
	{
		throw OpenCLException(s, res);
	}
}

cl_command_queue Dispatcher::Device::createQueue(cl_context &clContext, cl_device_id &clDeviceId)
{
	// nVidia CUDA Toolkit 10.1 only supports OpenCL 1.2 so we revert back to older functions for compatability
#ifdef PROFANITY_DEBUG
	cl_command_queue_properties p = CL_QUEUE_PROFILING_ENABLE;
#else
	cl_command_queue_properties p = 0;
#endif

#ifdef CL_VERSION_2_0
	const cl_command_queue ret = clCreateCommandQueueWithProperties(clContext, clDeviceId, &p, NULL);
#else
	const cl_command_queue ret = clCreateCommandQueue(clContext, clDeviceId, p, NULL);
#endif
	return ret == NULL ? throw std::runtime_error("failed to create command queue") : ret;
}

cl_kernel Dispatcher::Device::createKernel(cl_program &clProgram, const std::string s)
{
	cl_kernel ret = clCreateKernel(clProgram, s.c_str(), NULL);
	return ret == NULL ? throw std::runtime_error("failed to create kernel \"" + s + "\"") : ret;
}

// The original software has a vulnerability where the method lacks sufficient randomness in generating private keys.
// ref: https://medium.com/amber-group/exploiting-the-profanity-flaw-e986576de7ab
cl_ulong4 Dispatcher::Device::createSeed()
{
	cl_ulong4 r;
	const size_t seedSize = sizeof(r.s);

#ifdef _WIN32
	const NTSTATUS status = BCryptGenRandom(NULL, reinterpret_cast<PUCHAR>(&r.s[0]), (ULONG)seedSize, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
	if (status < 0)
	{
		throw std::runtime_error("failed to get secure random bytes from BCryptGenRandom");
	}
#elif defined(__APPLE__) || defined(__MACOSX)
	if (SecRandomCopyBytes(kSecRandomDefault, seedSize, reinterpret_cast<uint8_t *>(&r.s[0])) != errSecSuccess)
	{
		throw std::runtime_error("failed to get secure random bytes from SecRandomCopyBytes");
	}
#else
	std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
	if (!urandom.is_open())
	{
		throw std::runtime_error("failed to open /dev/urandom");
	}

	urandom.read(reinterpret_cast<char *>(&r.s[0]), seedSize);
	if (urandom.gcount() != static_cast<std::streamsize>(seedSize))
	{
		throw std::runtime_error("failed to read secure random bytes from /dev/urandom");
	}
#endif

	return r;
}

Dispatcher::Device::Device(
	Dispatcher &parent,
	cl_context &clContext,
	cl_program &clProgram,
	cl_device_id clDeviceId,
	const size_t worksizeLocal,
	const size_t size,
	const size_t index,
	const Mode &mode)
	: m_parent(parent),
	  m_index(index),
	  m_clDeviceId(clDeviceId),
	  m_worksizeLocal(worksizeLocal),
	  m_clScoreMax(0),
	  m_clQueue(createQueue(clContext, clDeviceId)),
	  m_kernelInit(createKernel(clProgram, "profanity_init")),
	  m_kernelInverse(createKernel(clProgram, "profanity_inverse")),
	  m_kernelIterate(createKernel(clProgram, "profanity_iterate")),
	  m_kernelScore(createKernel(clProgram, "profanity_score_matching")),
	  m_memPrecomp(clContext, m_clQueue, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, sizeof(g_precomp), g_precomp),
	  m_memPointsDeltaX(clContext, m_clQueue, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, size, true),
	  m_memInversedNegativeDoubleGy(clContext, m_clQueue, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, size, true),
	  m_memPrevLambda(clContext, m_clQueue, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, size, true),
	  m_memResult(clContext, m_clQueue, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, PROFANITY_MAX_SCORE + 1),
	  m_memData1(clContext, m_clQueue, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, 20 * mode.matchingCount),
	  m_memData2(clContext, m_clQueue, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, 20 * mode.matchingCount),
	  m_clSeed(createSeed()),
	  m_round(0),
	  m_speed(PROFANITY_SPEEDSAMPLES),
	  m_sizeInitialized(0),
	  m_eventFinished(NULL)
{
}

Dispatcher::Device::~Device()
{
	if (m_kernelScore != NULL)
	{
		clReleaseKernel(m_kernelScore);
	}
	if (m_kernelIterate != NULL)
	{
		clReleaseKernel(m_kernelIterate);
	}
	if (m_kernelInverse != NULL)
	{
		clReleaseKernel(m_kernelInverse);
	}
	if (m_kernelInit != NULL)
	{
		clReleaseKernel(m_kernelInit);
	}
	if (m_clQueue != NULL)
	{
		clReleaseCommandQueue(m_clQueue);
	}
}

Dispatcher::Dispatcher(cl_context &clContext,
					   cl_program &clProgram,
					   const Mode mode,
					   const size_t worksizeMax,
					   const size_t inverseSize,
					   const size_t inverseMultiple,
					   const cl_uchar clScoreQuit,
					   const size_t benchmarkSeconds,
					   const std::string &resultsPath)
	: m_clContext(clContext),
	  m_clProgram(clProgram),
	  m_mode(mode),
	  m_worksizeMax(worksizeMax),
	  m_inverseSize(inverseSize),
	  m_size(inverseSize * inverseMultiple),
	  m_clScoreMax(mode.score),
	  m_clScoreQuit(clScoreQuit),
	  m_benchmarkSeconds(benchmarkSeconds),
	  m_resultsPath(resultsPath),
	  m_resultsValidated(0),
	  m_resultsSaved(0),
	  m_resultThreadStop(false),
	  m_eventFinished(NULL),
	  m_countPrint(0)
{
}

Dispatcher::~Dispatcher()
{
	{
		std::lock_guard<std::mutex> lock(m_resultQueueMutex);
		m_resultThreadStop = true;
	}
	m_resultQueueCv.notify_all();
	if (m_resultThread.joinable())
	{
		m_resultThread.join();
	}
	for (auto *device : m_vDevices)
	{
		delete device;
	}
	m_vDevices.clear();
}

void Dispatcher::addDevice(
	cl_device_id clDeviceId,
	const size_t worksizeLocal,
	const size_t index)
{
	Device *pDevice = new Device(*this, m_clContext, m_clProgram, clDeviceId, worksizeLocal, m_size, index, m_mode);
	m_vDevices.push_back(pDevice);
}

void Dispatcher::run()
{
	m_eventFinished = clCreateUserEvent(m_clContext, NULL);
	timeStart = std::chrono::steady_clock::now();

	init();

	const auto timeInitialization = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - timeStart).count();
	std::cout << "  Initialization time: " << timeInitialization << " seconds" << std::endl;

	m_quit = false;
	m_countRunning = m_vDevices.size();
	timeRunStart = std::chrono::steady_clock::now();
	m_resultThreadStop = false;
	m_resultThread = std::thread(&Dispatcher::processResultQueue, this);

	std::cout << std::endl;
	std::cout << "Running..." << std::endl;
	std::cout << "  @shiyi 开发构建" << std::endl;
	std::cout << std::endl;

	for (auto it = m_vDevices.begin(); it != m_vDevices.end(); ++it)
	{
		dispatch(*(*it));
	}

	clWaitForEvents(1, &m_eventFinished);
	{
		std::lock_guard<std::mutex> lock(m_resultQueueMutex);
		m_resultThreadStop = true;
	}
	m_resultQueueCv.notify_all();
	if (m_resultThread.joinable())
	{
		m_resultThread.join();
	}
	clReleaseEvent(m_eventFinished);
	m_eventFinished = NULL;

	if (m_benchmarkSeconds == 0)
	{
		std::cout << "  Validated hits: " << m_resultsValidated << std::endl;
		if (!m_resultsPath.empty())
		{
			std::cout << "  Saved hits: " << m_resultsSaved << " -> " << m_resultsPath << std::endl;
		}
	}
}

void Dispatcher::init()
{
	std::cout << "Initializing:" << std::endl;
	std::cout << "  Should be no longer than 1 minute..." << std::endl;

	const auto deviceCount = m_vDevices.size();
	m_sizeInitTotal = m_size * deviceCount;
	m_sizeInitDone = 0;

	cl_event *const pInitEvents = new cl_event[deviceCount];

	for (size_t i = 0; i < deviceCount; ++i)
	{
		pInitEvents[i] = clCreateUserEvent(m_clContext, NULL);
		m_vDevices[i]->m_eventFinished = pInitEvents[i];
		initBegin(*m_vDevices[i]);
	}

	clWaitForEvents(deviceCount, pInitEvents);
	for (size_t i = 0; i < deviceCount; ++i)
	{
		m_vDevices[i]->m_eventFinished = NULL;
		clReleaseEvent(pInitEvents[i]);
	}

	delete[] pInitEvents;
}

void Dispatcher::initBegin(Device &d)
{
	// Set mode data
	for (auto i = 0; i < m_mode.matchingCount * 20; ++i)
	{
		d.m_memData1[i] = m_mode.data1[i];
		d.m_memData2[i] = m_mode.data2[i];
	}

	// Write precompute table and mode data
	d.m_memPrecomp.write(true);
	d.m_memData1.write(true);
	d.m_memData2.write(true);

	// Kernel arguments - profanity_begin
	d.m_memPrecomp.setKernelArg(d.m_kernelInit, 0);
	d.m_memPointsDeltaX.setKernelArg(d.m_kernelInit, 1);
	d.m_memPrevLambda.setKernelArg(d.m_kernelInit, 2);
	d.m_memResult.setKernelArg(d.m_kernelInit, 3);
	CLMemory<cl_ulong4>::setKernelArg(d.m_kernelInit, 4, d.m_clSeed);

	// Kernel arguments - profanity_inverse
	d.m_memPointsDeltaX.setKernelArg(d.m_kernelInverse, 0);
	d.m_memInversedNegativeDoubleGy.setKernelArg(d.m_kernelInverse, 1);

	// Kernel arguments - profanity_iterate
	d.m_memPointsDeltaX.setKernelArg(d.m_kernelIterate, 0);
	d.m_memInversedNegativeDoubleGy.setKernelArg(d.m_kernelIterate, 1);
	d.m_memPrevLambda.setKernelArg(d.m_kernelIterate, 2);

	// Kernel arguments - profanity_score_*
	d.m_memInversedNegativeDoubleGy.setKernelArg(d.m_kernelScore, 0);
	d.m_memResult.setKernelArg(d.m_kernelScore, 1);
	d.m_memData1.setKernelArg(d.m_kernelScore, 2);
	d.m_memData2.setKernelArg(d.m_kernelScore, 3);
	CLMemory<cl_uchar>::setKernelArg(d.m_kernelScore, 4, d.m_clScoreMax);
	CLMemory<cl_uchar>::setKernelArg(d.m_kernelScore, 5, m_mode.matchingCount);
	CLMemory<cl_uchar>::setKernelArg(d.m_kernelScore, 6, m_mode.prefixCount);
	CLMemory<cl_uchar>::setKernelArg(d.m_kernelScore, 7, m_mode.suffixCount);
	const cl_uchar suffixMatchIndex = (m_mode.matchingCount == 1 && m_mode.suffixCount == 1 && m_mode.data1.size() >= 20 && m_mode.data1[19] == 0xff)
		? base58IndexOf(m_mode.data2[19])
		: 0xff;
	CLMemory<cl_uchar>::setKernelArg(d.m_kernelScore, 8, suffixMatchIndex);
	
	// Seed device
	initContinue(d);
}

void Dispatcher::initContinue(Device &d)
{
	size_t sizeLeft = m_size - d.m_sizeInitialized;
	const size_t sizeInitLimit = m_size / 20;

	// Print progress
	const size_t percentDone = m_sizeInitDone * 100 / m_sizeInitTotal;
	std::cout << "  " << percentDone << "%\r" << std::flush;

	if (sizeLeft)
	{
		cl_event event;
		const size_t sizeRun = (std::min)(sizeInitLimit, (std::min)(sizeLeft, m_worksizeMax));
		const auto resEnqueue = clEnqueueNDRangeKernel(d.m_clQueue, d.m_kernelInit, 1, &d.m_sizeInitialized, &sizeRun, NULL, 0, NULL, &event);
		OpenCLException::throwIfError("kernel queueing failed during initilization", resEnqueue);

		// See: https://www.khronos.org/registry/OpenCL/sdk/1.2/docs/man/xhtml/clSetEventCallback.html
		// If an application needs to wait for completion of a routine from the above list in a callback, please use the non-blocking form of the function, and
		// assign a completion callback to it to do the remainder of your work. Note that when a callback (or other code) enqueues commands to a command-queue,
		// the commands are not required to begin execution until the queue is flushed. In standard usage, blocking enqueue calls serve this role by implicitly
		// flushing the queue. Since blocking calls are not permitted in callbacks, those callbacks that enqueue commands on a command queue should either call
		// clFlush on the queue before returning or arrange for clFlush to be called later on another thread.
		clFlush(d.m_clQueue);

		std::lock_guard<std::mutex> lock(m_mutex);
		d.m_sizeInitialized += sizeRun;
		m_sizeInitDone += sizeRun;

		const auto resCallback = clSetEventCallback(event, CL_COMPLETE, staticCallback, &d);
		OpenCLException::throwIfError("failed to set custom callback during initialization", resCallback);
	}
	else
	{
		// Printing one whole string at once helps in avoiding garbled output when executed in parallell
		const std::string strOutput = "  GPU-" + toString(d.m_index) + " initialized ...Done";
		std::cout << strOutput << std::endl;
		clSetUserEventStatus(d.m_eventFinished, CL_COMPLETE);
	}
}

void Dispatcher::enqueueKernelDevice(Device &d, cl_kernel &clKernel, size_t worksizeGlobal, cl_event *pEvent = NULL)
{
	try
	{
		enqueueKernel(d.m_clQueue, clKernel, worksizeGlobal, d.m_worksizeLocal, pEvent);
	}
	catch (OpenCLException &e)
	{
		// If local work size is invalid, abandon it and let implementation decide
		if ((e.m_res == CL_INVALID_WORK_GROUP_SIZE || e.m_res == CL_INVALID_WORK_ITEM_SIZE) && d.m_worksizeLocal != 0)
		{
			std::cout << std::endl
					  << "warning: local work size abandoned on GPU" << d.m_index << std::endl;
			d.m_worksizeLocal = 0;
			enqueueKernel(d.m_clQueue, clKernel, worksizeGlobal, d.m_worksizeLocal, pEvent);
		}
		else
		{
			throw;
		}
	}
}

void Dispatcher::enqueueKernel(cl_command_queue &clQueue, cl_kernel &clKernel, size_t worksizeGlobal, const size_t worksizeLocal, cl_event *pEvent = NULL)
{
	const size_t worksizeMax = m_worksizeMax;
	size_t worksizeOffset = 0;
	while (worksizeGlobal)
	{
		const size_t worksizeRun = (std::min)(worksizeGlobal, worksizeMax);
		const size_t *const pWorksizeLocal = (worksizeLocal == 0 ? NULL : &worksizeLocal);
		const auto res = clEnqueueNDRangeKernel(clQueue, clKernel, 1, &worksizeOffset, &worksizeRun, pWorksizeLocal, 0, NULL, pEvent);
		OpenCLException::throwIfError("kernel queueing failed", res);

		worksizeGlobal -= worksizeRun;
		worksizeOffset += worksizeRun;
	}
}

void Dispatcher::dispatch(Device &d)
{
	cl_event event;
	d.m_memResult.read(false, &event);
#ifdef PROFANITY_DEBUG
	cl_event eventInverse;
	cl_event eventIterate;
	enqueueKernelDevice(d, d.m_kernelInverse, m_size / m_inverseSize, &eventInverse);
	enqueueKernelDevice(d, d.m_kernelIterate, m_size, &eventIterate);
#else
	enqueueKernelDevice(d, d.m_kernelInverse, m_size / m_inverseSize);
	enqueueKernelDevice(d, d.m_kernelIterate, m_size);
#endif
	enqueueKernelDevice(d, d.m_kernelScore, m_size);
	clFlush(d.m_clQueue);
#ifdef PROFANITY_DEBUG
	// We're actually not allowed to call clFinish here because this function is ultimately asynchronously called by OpenCL.
	// However, this happens to work on my computer and it's not really intended for release, just something to aid me in
	// optimizations.
	clFinish(d.m_clQueue);
	std::cout << "Timing: profanity_inverse = " << getKernelExecutionTimeMicros(eventInverse) << "us, profanity_iterate = " << getKernelExecutionTimeMicros(eventIterate) << "us" << std::endl;
#endif
	const auto res = clSetEventCallback(event, CL_COMPLETE, staticCallback, &d);
	OpenCLException::throwIfError("failed to set custom callback", res);
}

static void printResult(
	cl_ulong4 seed,
	cl_ulong round,
	result r,
	cl_uchar score,
	const std::chrono::time_point<std::chrono::steady_clock> &timeStart,
	const Mode & mode)
{
	// Time delta
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - timeStart).count();

	// Format private key
	cl_ulong carry = 0;
	cl_ulong4 seedRes;

	seedRes.s[0] = seed.s[0] + round;
	carry = seedRes.s[0] < round;
	seedRes.s[1] = seed.s[1] + carry;
	carry = !seedRes.s[1];
	seedRes.s[2] = seed.s[2] + carry;
	carry = !seedRes.s[2];
	seedRes.s[3] = seed.s[3] + carry + r.foundId;

	std::ostringstream ss;
	ss << std::hex << std::setfill('0');
	ss << std::setw(16) << seedRes.s[3] << std::setw(16) << seedRes.s[2] << std::setw(16) << seedRes.s[1] << std::setw(16) << seedRes.s[0];
	const std::string strPrivate = ss.str();

	// Format public key
	const std::string strPublic = toHex(r.foundHash, 20);

	// Format public key for tron
	const std::string strPublicTron = toTron(strPublic);

	// Print
	const std::string strVT100ClearLine = "\33[2K\r";
	std::cout << strVT100ClearLine << "  Time: " << std::setw(5) << seconds << "s Address:" << strPublicTron << std::endl;

}

void Dispatcher::handleResult(Device &d)
{
	for (auto i = PROFANITY_MAX_SCORE; i > m_clScoreMax; --i)
	{
		result &r = d.m_memResult[i];

		if (r.found > 0 && i >= d.m_clScoreMax)
		{
			d.m_clScoreMax = i;
			CLMemory<cl_uchar>::setKernelArg(d.m_kernelScore, 4, d.m_clScoreMax);

			std::lock_guard<std::mutex> lock(m_mutex);
			if (i >= m_clScoreMax)
			{

				m_clScoreMax = i;

				if ((m_clScoreQuit && i >= m_clScoreQuit) || m_clScoreMax >= PROFANITY_MAX_SCORE)
				{
					m_quit = true;
				}

				if (m_benchmarkSeconds == 0)
				{
					enqueueResult(d, r, i);
				}
			}

			break;
		}
	}
}

void Dispatcher::onEvent(cl_event event, cl_int status, Device &d)
{
	if (status != CL_COMPLETE)
	{
		std::cout << "Dispatcher::onEvent - Got bad status: " << status << std::endl;
	}
	else if (d.m_eventFinished != NULL)
	{
		initContinue(d);
	}
	else
	{
		++d.m_round;
		handleResult(d);
		bool bDispatch = true;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			d.m_speed.sample(m_size);
			printSpeed();
			if (!m_quit && m_benchmarkSeconds > 0)
			{
				const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - timeRunStart).count();
				if (elapsed >= static_cast<long long>(m_benchmarkSeconds))
				{
					m_quit = true;
				}
			}

			if (m_quit)
			{
				bDispatch = false;
				if (--m_countRunning == 0)
				{
					printFinalSpeed();
					clSetUserEventStatus(m_eventFinished, CL_COMPLETE);
				}
			}
		}

		if (bDispatch)
		{
			dispatch(d);
		}
	}
}

void Dispatcher::enqueueResult(Device &d, const result &r, cl_uchar score)
{
	{
		std::lock_guard<std::mutex> lock(m_resultQueueMutex);
		m_resultQueue.emplace_back(d.m_clSeed, d.m_round, r, score);
	}
	m_resultQueueCv.notify_one();
}

bool Dispatcher::validateResult(const result &r) const
{
	const std::string strPublic = toHex(r.foundHash, 20);
	const std::string tronAddress = toTron(strPublic);

	if (tronAddress.size() != 34 || tronAddress[0] != 'T')
	{
		return false;
	}

	for (size_t j = 0; j < m_mode.matchingCount; ++j)
	{
		size_t scorePrefix = 1;
		size_t scoreSuffix = 0;
		const size_t dataBase = j * 20;

		if (m_mode.prefixCount > 1)
		{
			for (size_t i = 1; i < 10 && scorePrefix < m_mode.prefixCount; ++i)
			{
				const size_t dataIndex = dataBase + i;
				if (m_mode.data1[dataIndex] > 0 && ((unsigned char)tronAddress[i] & m_mode.data1[dataIndex]) == m_mode.data2[dataIndex])
				{
					++scorePrefix;
				}
				else
				{
					break;
				}
			}
		}

		if (m_mode.suffixCount > 0)
		{
			for (size_t i = 19; i > 10 && scoreSuffix < m_mode.suffixCount; --i)
			{
				const size_t dataIndex = dataBase + i;
				const size_t addressIndex = i + 14;
				if (m_mode.data1[dataIndex] > 0 && ((unsigned char)tronAddress[addressIndex] & m_mode.data1[dataIndex]) == m_mode.data2[dataIndex])
				{
					++scoreSuffix;
				}
				else
				{
					break;
				}
			}
		}

		if (scorePrefix >= m_mode.prefixCount && scoreSuffix >= m_mode.suffixCount)
		{
			return true;
		}
	}

	return false;
}

void Dispatcher::processResultQueue()
{
	for (;;)
	{
		std::tuple<cl_ulong4, cl_ulong, result, cl_uchar> item;
		{
			std::unique_lock<std::mutex> lock(m_resultQueueMutex);
			m_resultQueueCv.wait(lock, [&]() { return m_resultThreadStop || !m_resultQueue.empty(); });
			if (m_resultQueue.empty())
			{
				if (m_resultThreadStop)
				{
					return;
				}
				continue;
			}
			item = m_resultQueue.front();
			m_resultQueue.pop_front();
		}

		const cl_ulong4 seed = std::get<0>(item);
		const cl_ulong round = std::get<1>(item);
		const result r = std::get<2>(item);
		const cl_uchar score = std::get<3>(item);
		if (validateResult(r))
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				++m_resultsValidated;
			}
			printResult(seed, round, r, score, timeStart, m_mode);
			appendResultToFile(seed, round, r, score);
		}
	}
}

void Dispatcher::appendResultToFile(const cl_ulong4 &seed, cl_ulong round, const result &r, cl_uchar score)
{
	if (m_resultsPath.empty())
	{
		return;
	}

	cl_ulong carry = 0;
	cl_ulong4 seedRes;
	seedRes.s[0] = seed.s[0] + round;
	carry = seedRes.s[0] < round;
	seedRes.s[1] = seed.s[1] + carry;
	carry = !seedRes.s[1];
	seedRes.s[2] = seed.s[2] + carry;
	carry = !seedRes.s[2];
	seedRes.s[3] = seed.s[3] + carry + r.foundId;

	std::ostringstream privateKey;
	privateKey << std::hex << std::setfill('0');
	privateKey << std::setw(16) << seedRes.s[3] << std::setw(16) << seedRes.s[2] << std::setw(16) << seedRes.s[1] << std::setw(16) << seedRes.s[0];

	const std::string strPublic = toHex(r.foundHash, 20);
	const std::string strPublicTron = toTron(strPublic);
	const std::time_t now = std::time(NULL);
	std::tm timeInfo = {};
#ifdef _WIN32
	localtime_s(&timeInfo, &now);
#else
	localtime_r(&now, &timeInfo);
#endif
	char timeBuffer[32] = {0};
	std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

	std::ofstream out(m_resultsPath, std::ios::out | std::ios::app);
	if (!out.is_open())
	{
		return;
	}

	out << "time=" << timeBuffer
		<< " score=" << (unsigned int)score
		<< " prefix=" << (unsigned int)m_mode.prefixCount
		<< " suffix=" << (unsigned int)m_mode.suffixCount
		<< " address=" << strPublicTron
		<< " private=" << privateKey.str()
		<< std::endl;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		++m_resultsSaved;
	}
}

void Dispatcher::printFinalSpeed()
{
	std::string strGPUs;
	double speedTotal = 0;
	for (auto &e : m_vDevices)
	{
		const auto curSpeed = e->m_speed.getSpeed();
		speedTotal += curSpeed;
		strGPUs += " GPU" + toString(e->m_index) + ": " + formatSpeed(curSpeed);
	}

	std::cerr << std::endl
			  << "Final: " << formatSpeed(speedTotal) << " -" << strGPUs << std::endl;
}

void Dispatcher::printSpeed()
{
	++m_countPrint;
	if (m_countPrint > m_vDevices.size())
	{
		std::string strGPUs;
		double speedTotal = 0;
		unsigned int i = 0;
		for (auto &e : m_vDevices)
		{
			const auto curSpeed = e->m_speed.getSpeed();
			speedTotal += curSpeed;
			strGPUs += " GPU" + toString(e->m_index) + ": " + formatSpeed(curSpeed);
			++i;
		}

		const std::string strVT100ClearLine = "\33[2K\r";
		std::cerr << strVT100ClearLine << "Total: " << formatSpeed(speedTotal) << " -" << strGPUs << '\r' << std::flush;
		m_countPrint = 0;
	}
}

void CL_CALLBACK Dispatcher::staticCallback(cl_event event, cl_int event_command_exec_status, void *user_data)
{
	Device *const pDevice = static_cast<Device *>(user_data);
	pDevice->m_parent.onEvent(event, event_command_exec_status, *pDevice);
	clReleaseEvent(event);
}

std::string Dispatcher::formatSpeed(double f)
{
	const std::string S = " KMGT";

	unsigned int index = 0;
	while (f > 1000.0f && index < S.size())
	{
		f /= 1000.0f;
		++index;
	}

	std::ostringstream ss;
	ss << std::fixed << std::setprecision(3) << (double)f << " " << S[index] << "H/s";
	return ss.str();
}
