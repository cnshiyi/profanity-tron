#ifndef HPP_DISPATCHER
#define HPP_DISPATCHER

#include <stdexcept>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <deque>
#include <thread>
#include <tuple>
#include <condition_variable>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#define clCreateCommandQueueWithProperties clCreateCommandQueue
#else
#include <CL/cl.h>
#endif

#include "SpeedSample.hpp"
#include "CLMemory.hpp"
#include "types.hpp"
#include "Mode.hpp"

#define PROFANITY_SPEEDSAMPLES 20
#define PROFANITY_MAX_SCORE 120

struct SearchRange {
	bool enabled;
	bool descending;
	cl_ulong4 start;
	cl_ulong4 end;
	unsigned int hexFirst;
	unsigned int hexLast;
	cl_ulong prefixHigh;
	cl_ulong counterStart;
	cl_ulong counterMask;
	cl_ulong counterMax;
	cl_uint counterShift;

	SearchRange();
};

class Dispatcher {
	private:
		class OpenCLException : public std::runtime_error {
			public:
				OpenCLException(const std::string s, const cl_int res);

				static void throwIfError(const std::string s, const cl_int res);

				const cl_int m_res;
		};

		struct Device {
			static cl_command_queue createQueue(cl_context & clContext, cl_device_id & clDeviceId);
			static cl_kernel createKernel(cl_program & clProgram, const std::string s);
			static cl_ulong4 createSeed();

			Device(Dispatcher & parent, cl_context & clContext, cl_program & clProgram, cl_device_id clDeviceId, const size_t worksizeLocal, const size_t size, const size_t index, const Mode & mode, const SearchRange & range);
			~Device();

			Dispatcher & m_parent;
			const size_t m_index;

			cl_device_id m_clDeviceId;
			size_t m_worksizeLocal;
			cl_uchar m_clScoreMax;
			cl_command_queue m_clQueue;

			cl_kernel m_kernelInit;
			cl_kernel m_kernelInverse;
			cl_kernel m_kernelIterate;
			cl_kernel m_kernelScore;

			CLMemory<point> m_memPrecomp;
			CLMemory<mp_number> m_memPointsDeltaX;
			CLMemory<mp_number> m_memInversedNegativeDoubleGy;
			CLMemory<mp_number> m_memPrevLambda;
			CLMemory<result> m_memResult;

			// Data parameters used in some modes
			CLMemory<cl_uchar> m_memData1;
			CLMemory<cl_uchar> m_memData2;
			CLMemory<cl_uchar> m_memSuffix1Allowed;

			// Seed and round information
			cl_ulong4 m_clSeed;
			cl_ulong m_round;

			// Speed sampling
			SpeedSample m_speed;

			// Initialization
			size_t m_sizeInitialized;
			cl_event m_eventFinished;
		};

	public:
		Dispatcher(cl_context & clContext, cl_program & clProgram, const Mode mode, const size_t worksizeMax, const size_t inverseSize, const size_t inverseMultiple, const cl_uchar clScoreQuit = 0, const size_t benchmarkSeconds = 0, const std::string & resultsPath = std::string(), const SearchRange & range = SearchRange());
		~Dispatcher();

		void addDevice(cl_device_id clDeviceId, const size_t worksizeLocal, const size_t index);
		void run();

	private:
		void init();
		void initBegin(Device & d);
		void initContinue(Device & d);

		void dispatch(Device & d);
		void enqueueKernel(cl_command_queue & clQueue, cl_kernel & clKernel, size_t worksizeGlobal, const size_t worksizeLocal, cl_event * pEvent);
		void enqueueKernelDevice(Device & d, cl_kernel & clKernel, size_t worksizeGlobal, cl_event * pEvent);

		void handleResult(Device & d);
		void enqueueResult(Device & d, const result & r, cl_uchar score);
		void processResultQueue();
		bool validateResult(const result & r) const;
		void appendResultToFile(const cl_ulong4 & seed, cl_ulong round, const result & r, cl_uchar score);
		void prepareRangeDevice(Device & d);
		cl_ulong4 candidatePrivate(const cl_ulong4 & seed, cl_ulong round, cl_uint foundId) const;

		void onEvent(cl_event event, cl_int status, Device & d);

		void printSpeed();
		void printFinalSpeed();

	private:
		static void CL_CALLBACK staticCallback(cl_event event, cl_int event_command_exec_status, void * user_data);

		static std::string formatSpeed(double s);
	private: /* Instance variables */
		cl_context & m_clContext;
		cl_program & m_clProgram;
		const Mode m_mode;
		const size_t m_worksizeMax;
		const size_t m_inverseSize;
		const size_t m_size;
		cl_uchar m_clScoreMax;
		cl_uchar m_clScoreQuit;
		const size_t m_benchmarkSeconds;
		const std::string m_resultsPath;
		const SearchRange m_range;
		size_t m_resultsValidated;
		size_t m_resultsSaved;
		std::vector<Device *> m_vDevices;

		cl_event m_eventFinished;

		// Run information
		std::mutex m_mutex;
		std::mutex m_resultQueueMutex;
		std::condition_variable m_resultQueueCv;
		std::deque<std::tuple<cl_ulong4, cl_ulong, result, cl_uchar>> m_resultQueue;
		std::thread m_resultThread;
		bool m_resultThreadStop;
		std::chrono::time_point<std::chrono::steady_clock> timeStart;
		std::chrono::time_point<std::chrono::steady_clock> timeRunStart;
		unsigned int m_countPrint;
		unsigned int m_countRunning;
		size_t m_sizeInitTotal;
		size_t m_sizeInitDone;
		bool m_quit;
};

#endif /* HPP_DISPATCHER */
