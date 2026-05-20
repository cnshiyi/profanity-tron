#include <CL/cl.h>

#include <chrono>
#include <cstdint>
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

uint pseudo_mix(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

uint heavy_round(uint x)
{
    x = rotate(x ^ 0x9e3779b9U, 5U) + 0x7f4a7c15U;
    x ^= rotate(x, 11U);
    x += rotate(x, 17U);
    return x;
}

__kernel void baseline_full_score(__global uint *out)
{
    const uint id = get_global_id(0);
    uint v = pseudo_mix(id);

    // Simulate expensive encoding path.
    for (uint i = 0; i < 80; ++i) {
        v = heavy_round(v);
    }

    out[id] = v;
}

__kernel void aggressive_prefilter_score(__global uint *out)
{
    const uint id = get_global_id(0);
    uint v = pseudo_mix(id);

    // Cheap prefilter: only a small fraction enters the expensive path.
    const uint gate = v & 1023U;
    if (gate < 12U) {
        for (uint i = 0; i < 80; ++i) {
            v = heavy_round(v);
        }
    } else {
        for (uint i = 0; i < 4; ++i) {
            v = heavy_round(v);
        }
    }

    out[id] = v;
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

        cl_kernel baseline = clCreateKernel(program, "baseline_full_score", &err);
        must(err, "clCreateKernel(baseline_full_score)");
        cl_kernel aggressive = clCreateKernel(program, "aggressive_prefilter_score", &err);
        must(err, "clCreateKernel(aggressive_prefilter_score)");

        const size_t globalSize = 255u * 32768u;
        const size_t localSize = 256u;
        const int rounds = 20;
        const size_t outBytes = globalSize * sizeof(cl_uint);

        cl_mem out = clCreateBuffer(context, CL_MEM_READ_WRITE, outBytes, nullptr, &err);
        must(err, "clCreateBuffer(out)");

        must(clSetKernelArg(baseline, 0, sizeof(cl_mem), &out), "clSetKernelArg(baseline)");
        must(clSetKernelArg(aggressive, 0, sizeof(cl_mem), &out), "clSetKernelArg(aggressive)");

        const double baselineMs = runKernel(queue, baseline, globalSize, localSize, rounds);
        const double aggressiveMs = runKernel(queue, aggressive, globalSize, localSize, rounds);

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "baseline_full_score_ms=" << baselineMs << "\n";
        std::cout << "aggressive_prefilter_ms=" << aggressiveMs << "\n";
        std::cout << "speedup=" << (baselineMs / aggressiveMs) << "x\n";

        clReleaseMemObject(out);
        clReleaseKernel(aggressive);
        clReleaseKernel(baseline);
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
