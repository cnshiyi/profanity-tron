#include <CL/cl.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
const char *kKernelSource = R"CLC(
typedef struct {
    uint found;
    uint foundId;
    uchar foundHash[20];
} result;

#define SCORE_SLOTS 121

__kernel void clear_results_only(__global result *results)
{
    const uint id = get_global_id(0);
    if (id < SCORE_SLOTS) {
        results[id].found = 0;
    }
}

__kernel void init_with_inline_clear(__global ulong *state, __global result *results)
{
    const uint id = get_global_id(0);
    ulong v = (ulong)(id + 1) * 0x9E3779B97F4A7C15UL;
    for (uint i = 0; i < 64; ++i) {
        v ^= (v << 13);
        v ^= (v >> 7);
        v ^= (v << 17);
    }
    state[id] = v;

    for (uint i = 0; i < SCORE_SLOTS; ++i) {
        results[i].found = 0;
    }
}

__kernel void init_without_inline_clear(__global ulong *state)
{
    const uint id = get_global_id(0);
    ulong v = (ulong)(id + 1) * 0x9E3779B97F4A7C15UL;
    for (uint i = 0; i < 64; ++i) {
        v ^= (v << 13);
        v ^= (v >> 7);
        v ^= (v << 17);
    }
    state[id] = v;
}
)CLC";

template <typename T>
T getInfoStringLike(cl_device_id deviceId, cl_device_info param)
{
    size_t len = 0;
    clGetDeviceInfo(deviceId, param, 0, nullptr, &len);
    std::vector<char> buffer(len);
    clGetDeviceInfo(deviceId, param, len, buffer.data(), nullptr);
    return T(buffer.data());
}

std::string buildLog(cl_program program, cl_device_id deviceId)
{
    size_t len = 0;
    clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &len);
    std::vector<char> buffer(len ? len : 1);
    clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, buffer.size(), buffer.data(), nullptr);
    return std::string(buffer.data());
}

void must(cl_int err, const char *what)
{
    if (err != CL_SUCCESS) {
        std::ostringstream oss;
        oss << what << " failed with " << err;
        throw std::runtime_error(oss.str());
    }
}

double runKernel(cl_command_queue queue, cl_kernel kernel, size_t globalSize, size_t localSize, int rounds)
{
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < rounds; ++i) {
        must(clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, localSize == 0 ? nullptr : &localSize, 0, nullptr, nullptr), "clEnqueueNDRangeKernel");
    }
    must(clFinish(queue), "clFinish");
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}
}

int main()
{
    try {
        cl_uint platformCount = 0;
        must(clGetPlatformIDs(0, nullptr, &platformCount), "clGetPlatformIDs(count)");
        if (platformCount == 0) {
            throw std::runtime_error("no OpenCL platforms found");
        }

        std::vector<cl_platform_id> platforms(platformCount);
        must(clGetPlatformIDs(platformCount, platforms.data(), nullptr), "clGetPlatformIDs(list)");

        cl_device_id deviceId = nullptr;
        for (auto platform : platforms) {
            cl_uint deviceCount = 0;
            if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &deviceCount) != CL_SUCCESS || deviceCount == 0) {
                continue;
            }
            std::vector<cl_device_id> devices(deviceCount);
            must(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, deviceCount, devices.data(), nullptr), "clGetDeviceIDs");
            deviceId = devices.front();
            break;
        }

        if (deviceId == nullptr) {
            throw std::runtime_error("no GPU OpenCL device found");
        }

        std::cout << "device: " << getInfoStringLike<std::string>(deviceId, CL_DEVICE_NAME) << "\n";
        std::cout << "vendor: " << getInfoStringLike<std::string>(deviceId, CL_DEVICE_VENDOR) << "\n";

        cl_int err = CL_SUCCESS;
        cl_context context = clCreateContext(nullptr, 1, &deviceId, nullptr, nullptr, &err);
        must(err, "clCreateContext");

        cl_command_queue queue = clCreateCommandQueue(context, deviceId, 0, &err);
        must(err, "clCreateCommandQueue");

        const char *sources[] = {kKernelSource};
        cl_program program = clCreateProgramWithSource(context, 1, sources, nullptr, &err);
        must(err, "clCreateProgramWithSource");

        err = clBuildProgram(program, 1, &deviceId, nullptr, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << buildLog(program, deviceId) << "\n";
            must(err, "clBuildProgram");
        }

        cl_kernel clearKernel = clCreateKernel(program, "clear_results_only", &err);
        must(err, "clCreateKernel(clear_results_only)");
        cl_kernel inlineKernel = clCreateKernel(program, "init_with_inline_clear", &err);
        must(err, "clCreateKernel(init_with_inline_clear)");
        cl_kernel splitKernel = clCreateKernel(program, "init_without_inline_clear", &err);
        must(err, "clCreateKernel(init_without_inline_clear)");

        const size_t globalSize = 255u * 32768u;
        const size_t localSize = 256u;
        const int rounds = 20;
        const size_t stateBytes = globalSize * sizeof(cl_ulong);
        const size_t resultBytes = 121u * 32u;

        cl_mem state = clCreateBuffer(context, CL_MEM_READ_WRITE, stateBytes, nullptr, &err);
        must(err, "clCreateBuffer(state)");
        cl_mem results = clCreateBuffer(context, CL_MEM_READ_WRITE, resultBytes, nullptr, &err);
        must(err, "clCreateBuffer(results)");

        must(clSetKernelArg(clearKernel, 0, sizeof(cl_mem), &results), "clSetKernelArg(clear)");
        must(clSetKernelArg(inlineKernel, 0, sizeof(cl_mem), &state), "clSetKernelArg(inline state)");
        must(clSetKernelArg(inlineKernel, 1, sizeof(cl_mem), &results), "clSetKernelArg(inline results)");
        must(clSetKernelArg(splitKernel, 0, sizeof(cl_mem), &state), "clSetKernelArg(split state)");

        const double inlineMs = runKernel(queue, inlineKernel, globalSize, localSize, rounds);

        auto startSplit = std::chrono::steady_clock::now();
        for (int i = 0; i < rounds; ++i) {
            const size_t clearGlobal = 128;
            must(clEnqueueNDRangeKernel(queue, clearKernel, 1, nullptr, &clearGlobal, nullptr, 0, nullptr, nullptr), "enqueue clear");
            must(clEnqueueNDRangeKernel(queue, splitKernel, 1, nullptr, &globalSize, &localSize, 0, nullptr, nullptr), "enqueue split");
        }
        must(clFinish(queue), "finish split");
        auto endSplit = std::chrono::steady_clock::now();
        const double splitMs = std::chrono::duration<double, std::milli>(endSplit - startSplit).count();

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "inline_clear_ms=" << inlineMs << "\n";
        std::cout << "split_clear_ms=" << splitMs << "\n";
        std::cout << "speedup=" << (inlineMs / splitMs) << "x\n";

        clReleaseMemObject(results);
        clReleaseMemObject(state);
        clReleaseKernel(splitKernel);
        clReleaseKernel(inlineKernel);
        clReleaseKernel(clearKernel);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
