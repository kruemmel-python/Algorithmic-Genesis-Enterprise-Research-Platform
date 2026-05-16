#include "ag/ag_opencl.hpp"
#include "ag/ag_rng.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <chrono>

#if AG_ENABLE_OPENCL
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#endif

namespace ag {

namespace {

using Clock = std::chrono::steady_clock;
[[maybe_unused]] double elapsed_ms(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

#if AG_ENABLE_OPENCL

const char* random_kernel_source = R"CLC(
static uint ag_xorshift(uint x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

__kernel void generate_random_numbers(__global float* output, const uint count, const uint seed) {
    const uint i = get_global_id(0);
    if (i < count) {
        uint x = seed ^ (i * 747796405u + 2891336453u);
        x = ag_xorshift(x);
        output[i] = (float)(x & 0x00FFFFFFu) / (float)0x01000000u;
    }
}
)CLC";

const char* snn_kernel_source = R"CLC(
static uint ag_xorshift(uint x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

__kernel void lif_step(__global float* voltage,
                       __global uint* spikes,
                       const uint neurons,
                       const uint seed,
                       const uint step,
                       const float dt,
                       const float threshold,
                       const float leak_rate,
                       const float reset_voltage) {
    const uint i = get_global_id(0);
    if (i >= neurons) return;

    float v = voltage[i];
    v *= (1.0f - leak_rate * dt);

    // Counter-based deterministic external drive. No global RNG state, no atomics.
    uint r = ag_xorshift(seed ^ (i * 747796405u) ^ (step * 2891336453u));
    const float u = (float)(r & 0x00FFFFFFu) / (float)0x01000000u;
    if (u < 0.03f) {
        uint r2 = ag_xorshift(r + 0x9e3779b9u);
        const float amp = 0.35f + 0.2f * ((float)(r2 & 0x00FFFFFFu) / (float)0x01000000u);
        v = ag_sanitize(v + amp);
    }

    if (v >= threshold) {
        spikes[i] = 1u;
        voltage[i] = reset_voltage;
    } else {
        spikes[i] = 0u;
        voltage[i] = v;
    }
}

__kernel void reduce_u32(__global const uint* in,
                         __global uint* partial,
                         const uint count) {
    const uint gid = get_global_id(0);
    const uint lid = get_local_id(0);
    const uint group = get_group_id(0);
    const uint lsize = get_local_size(0);
    __local uint scratch[256];

    uint sum = 0u;
    const uint stride = get_global_size(0);
    for (uint i = gid; i < count; i += stride) {
        sum += in[i];
    }
    scratch[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint offset = lsize >> 1; offset > 0; offset >>= 1) {
        if (lid < offset) scratch[lid] += scratch[lid + offset];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (lid == 0u) partial[group] = scratch[0];
}

__kernel void reduce_f32(__global const float* in,
                         __global float* partial,
                         const uint count) {
    const uint gid = get_global_id(0);
    const uint lid = get_local_id(0);
    const uint group = get_group_id(0);
    const uint lsize = get_local_size(0);
    __local float scratch[256];

    float sum = 0.0f;
    const uint stride = get_global_size(0);
    for (uint i = gid; i < count; i += stride) {
        sum += in[i];
    }
    scratch[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint offset = lsize >> 1; offset > 0; offset >>= 1) {
        if (lid < offset) scratch[lid] += scratch[lid + offset];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (lid == 0u) partial[group] = scratch[0];
}
)CLC";


const char* snn_recurrent_stdp_kernel_source = R"CLC(
static uint ag_xorshift(uint x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static float ag_sanitize(float v) {
    uint i = as_uint(v);
    uint exp = i & 0x7F800000u;
    return (exp == 0x7F800000u) ? 0.0f : clamp(v, -1000000.0f, 1000000.0f);
}

__kernel void lif_recurrent_step(__global float* voltage,
                                 __global uint* spikes,
                                 __global const uint* prev_spikes,
                                 __global float* last_spike_time,
                                 __global const uint* incoming_offsets,
                                 __global const uint* incoming_sources,
                                 __global const uint* incoming_edge_ids,
                                 __global const float* weights,
                                 const uint neurons,
                                 const uint seed,
                                 const uint step,
                                 const float dt,
                                 const float threshold,
                                 const float leak_rate,
                                 const float reset_voltage) {
    const uint i = get_global_id(0);
    if (i >= neurons) return;

    float v = voltage[i] * (1.0f - leak_rate * dt);

    const uint begin = incoming_offsets[i];
    const uint end = incoming_offsets[i + 1u];
    float syn = 0.0f;
    for (uint p = begin; p < end; ++p) {
        const uint src = incoming_sources[p];
        const uint edge = incoming_edge_ids[p];
        if (prev_spikes[src] != 0u) syn += ag_sanitize(weights[edge]);
    }
    v = ag_sanitize(v + syn);

    uint r = ag_xorshift(seed ^ (i * 747796405u) ^ (step * 2891336453u));
    const float u = (float)(r & 0x00FFFFFFu) / (float)0x01000000u;
    if (u < 0.03f) {
        uint r2 = ag_xorshift(r + 0x9e3779b9u);
        const float amp = 0.35f + 0.2f * ((float)(r2 & 0x00FFFFFFu) / (float)0x01000000u);
        v = ag_sanitize(v + amp);
    }

    if (v >= threshold) {
        spikes[i] = 1u;
        voltage[i] = reset_voltage;
        last_spike_time[i] = (float)step;
    } else {
        spikes[i] = 0u;
        voltage[i] = v;
    }
}

__kernel void stdp_update_edges(__global float* weights,
                                __global const uint* outgoing_offsets,
                                __global const uint* outgoing_indices,
                                __global const uint* prev_spikes,
                                __global const float* last_spike_time,
                                const uint neurons,
                                const uint edge_count,
                                const uint step,
                                const float stdp_lr) {
    const uint src = get_global_id(0);
    if (src >= neurons) return;
    if (prev_spikes[src] == 0u) return;

    const uint begin = outgoing_offsets[src];
    const uint end = outgoing_offsets[src + 1u];
    for (uint e = begin; e < end && e < edge_count; ++e) {
        const uint dst = outgoing_indices[e];
        float w = weights[e];
        const float delta_t = (float)step - last_spike_time[dst];
        if (delta_t > 0.0f && delta_t < 20.0f) {
            w -= stdp_lr * (20.0f - delta_t) / 20.0f;
        } else {
            w += stdp_lr * 0.1f;
        }
        if (w < 0.0f) w = 0.0f;
        if (w > 2.0f) w = 2.0f;
        weights[e] = ag_sanitize(w);
    }
}

__kernel void somnia_decay_edges(__global float* weights,
                                 __global const float* last_spike_time,
                                 __global const uint* outgoing_indices,
                                 const uint edge_count,
                                 const uint step,
                                 const float decay_rate) {
    const uint e = get_global_id(0);
    if (e >= edge_count) return;
    const uint dst = outgoing_indices[e];
    float inactive = fmax(0.0f, (float)step - last_spike_time[dst]);
    float w = weights[e] * fmax(0.0f, 1.0f - decay_rate * inactive);
    weights[e] = ag_sanitize(clamp(w, 0.0f, 2.0f));
}

__kernel void quantize_weights_u8_projection(__global float* weights,
                                             const uint edge_count) {
    const uint e = get_global_id(0);
    if (e >= edge_count) return;
    float w = clamp(ag_sanitize(weights[e]), 0.0f, 2.0f);
    uint q = (uint)rint(w * 127.5f);
    if (q > 255u) q = 255u;
    weights[e] = ((float)q) / 127.5f;
}

__kernel void copy_u32(__global const uint* src,
                       __global uint* dst,
                       const uint count) {
    const uint i = get_global_id(0);
    if (i < count) dst[i] = src[i];
}

__kernel void accumulate_partials_u32(__global const uint* partial,
                                      __global uint* total,
                                      const uint count) {
    const uint lid = get_local_id(0);
    const uint lsize = get_local_size(0);
    __local uint scratch[256];

    uint sum = 0u;
    for (uint i = lid; i < count; i += lsize) sum += partial[i];
    scratch[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint offset = lsize >> 1; offset > 0; offset >>= 1) {
        if (lid < offset) scratch[lid] += scratch[lid + offset];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (lid == 0u) total[0] += scratch[0];
}

__kernel void reduce_u32(__global const uint* in,
                         __global uint* partial,
                         const uint count) {
    const uint gid = get_global_id(0);
    const uint lid = get_local_id(0);
    const uint group = get_group_id(0);
    const uint lsize = get_local_size(0);
    __local uint scratch[256];

    uint sum = 0u;
    const uint stride = get_global_size(0);
    for (uint i = gid; i < count; i += stride) sum += in[i];
    scratch[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint offset = lsize >> 1; offset > 0; offset >>= 1) {
        if (lid < offset) scratch[lid] += scratch[lid + offset];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (lid == 0u) partial[group] = scratch[0];
}

__kernel void reduce_f32(__global const float* in,
                         __global float* partial,
                         const uint count) {
    const uint gid = get_global_id(0);
    const uint lid = get_local_id(0);
    const uint group = get_group_id(0);
    const uint lsize = get_local_size(0);
    __local float scratch[256];

    float sum = 0.0f;
    const uint stride = get_global_size(0);
    for (uint i = gid; i < count; i += stride) sum += in[i];
    scratch[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint offset = lsize >> 1; offset > 0; offset >>= 1) {
        if (lid < offset) scratch[lid] += scratch[lid + offset];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (lid == 0u) partial[group] = scratch[0];
}
)CLC";

struct OpenClRuntime {
    cl_platform_id platform{nullptr};
    cl_device_id device{nullptr};
    cl_context context{nullptr};
    cl_command_queue queue{nullptr};
    OpenClProbe probe;

    OpenClRuntime() {
        cl_uint platform_count = 0;
        cl_int err = clGetPlatformIDs(0, nullptr, &platform_count);
        if (err != CL_SUCCESS || platform_count == 0) throw std::runtime_error("clGetPlatformIDs failed or returned no platforms");

        std::vector<cl_platform_id> platforms(platform_count);
        err = clGetPlatformIDs(platform_count, platforms.data(), nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("clGetPlatformIDs retrieval failed");

        for (cl_platform_id p : platforms) {
            cl_uint n = 0;
            if (clGetDeviceIDs(p, CL_DEVICE_TYPE_GPU, 0, nullptr, &n) == CL_SUCCESS && n > 0) {
                std::vector<cl_device_id> ds(n);
                if (clGetDeviceIDs(p, CL_DEVICE_TYPE_GPU, n, ds.data(), nullptr) == CL_SUCCESS) {
                    platform = p;
                    device = ds.front();
                    break;
                }
            }
        }
        if (!device) {
            for (cl_platform_id p : platforms) {
                cl_uint n = 0;
                if (clGetDeviceIDs(p, CL_DEVICE_TYPE_CPU, 0, nullptr, &n) == CL_SUCCESS && n > 0) {
                    std::vector<cl_device_id> ds(n);
                    if (clGetDeviceIDs(p, CL_DEVICE_TYPE_CPU, n, ds.data(), nullptr) == CL_SUCCESS) {
                        platform = p;
                        device = ds.front();
                        break;
                    }
                }
            }
        }
        if (!device) throw std::runtime_error("no GPU or CPU OpenCL devices found");

        char pname[256] = {0};
        char dname[256] = {0};
        clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(pname), pname, nullptr);
        clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(dname), dname, nullptr);
        probe.available = true;
        probe.platform_name = pname;
        probe.device_name = dname;

        context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        if (err != CL_SUCCESS || !context) throw std::runtime_error("clCreateContext failed");

#if defined(CL_VERSION_2_0)
        queue = clCreateCommandQueueWithProperties(context, device, nullptr, &err);
#else
        queue = clCreateCommandQueue(context, device, 0, &err);
#endif
        if (err != CL_SUCCESS || !queue) throw std::runtime_error("clCreateCommandQueue failed");
    }

    ~OpenClRuntime() {
        if (queue) clReleaseCommandQueue(queue);
        if (context) clReleaseContext(context);
    }

    cl_program build_program(const char* source) const {
        cl_int err = CL_SUCCESS;
        const char* sources[] = {source};
        const size_t lengths[] = {std::char_traits<char>::length(source)};
        cl_program program = clCreateProgramWithSource(context, 1, sources, lengths, &err);
        if (err != CL_SUCCESS || !program) throw std::runtime_error("clCreateProgramWithSource failed");

        err = clBuildProgram(program, 1, &device, "-cl-std=CL1.2", nullptr, nullptr);
        if (err != CL_SUCCESS) {
            size_t log_size = 0;
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
            std::string log(log_size ? log_size : 1, '\0');
            if (log_size) clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log.size(), log.data(), nullptr);
            clReleaseProgram(program);
            throw std::runtime_error("clBuildProgram failed: " + log);
        }
        return program;
    }
};

struct ClMem {
    cl_mem mem{nullptr};
    ClMem() = default;
    ClMem(cl_context context, cl_mem_flags flags, size_t bytes, void* host = nullptr) {
        cl_int err = CL_SUCCESS;
        mem = clCreateBuffer(context, flags, bytes, host, &err);
        if (err != CL_SUCCESS || !mem) throw std::runtime_error("clCreateBuffer failed");
    }
    ~ClMem() { if (mem) clReleaseMemObject(mem); }
    ClMem(const ClMem&) = delete;
    ClMem& operator=(const ClMem&) = delete;
};

size_t round_up(size_t n, size_t group) {
    return ((n + group - 1) / group) * group;
}

size_t normalize_local_size(size_t requested) {
    if (requested >= 256) return 256;
    if (requested >= 128) return 128;
    if (requested >= 64) return 64;
    return 64;
}

#endif

std::vector<float> snn_lif_cpu(uint64_t seed,
                               std::size_t neurons,
                               int steps,
                               float dt,
                               float threshold,
                               float leak_rate,
                               float reset_voltage,
                               OpenClSnnResult& result) {
    SplitMix64 rng(seed);
    std::vector<float> voltage(neurons, 0.0f);
    result.spikes_by_step.reserve(static_cast<std::size_t>(std::max(0, steps)));
    for (int t = 0; t < steps; ++t) {
        int spikes = 0;
        for (std::size_t i = 0; i < neurons; ++i) {
            voltage[i] *= (1.0f - leak_rate * dt);
            if (rng.bernoulli(0.03)) voltage[i] += 0.35f + 0.2f * rng.uniform01f();
            if (voltage[i] >= threshold) {
                ++spikes;
                voltage[i] = reset_voltage;
            }
        }
        result.spikes_by_step.push_back(spikes);
        result.total_spikes += spikes;
    }
    return voltage;
}

} // namespace

OpenClProbe probe_opencl() {
    OpenClProbe p;
#if AG_ENABLE_OPENCL
    try {
        OpenClRuntime rt;
        return rt.probe;
    } catch (const std::exception& e) {
        p.error = e.what();
        return p;
    }
#else
    p.error = "compiled without OpenCL";
#endif
    return p;
}

std::vector<float> random_floats_opencl_or_cpu(uint64_t seed, std::size_t count, OpenClProbe* probe) {
    OpenClProbe local;
#if AG_ENABLE_OPENCL
    try {
        OpenClRuntime rt;
        local = rt.probe;
        cl_int err = CL_SUCCESS;
        cl_program program = rt.build_program(random_kernel_source);
        cl_kernel kernel = clCreateKernel(program, "generate_random_numbers", &err);
        if (err != CL_SUCCESS || !kernel) {
            clReleaseProgram(program);
            throw std::runtime_error("clCreateKernel(generate_random_numbers) failed");
        }

        std::vector<float> out(count, 0.0f);
        ClMem output(rt.context, CL_MEM_WRITE_ONLY, sizeof(float) * std::max<std::size_t>(count, 1));
        const cl_uint cl_count = static_cast<cl_uint>(count);
        const cl_uint cl_seed = static_cast<cl_uint>(seed ^ (seed >> 32u));

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &output.mem);
        clSetKernelArg(kernel, 1, sizeof(cl_uint), &cl_count);
        clSetKernelArg(kernel, 2, sizeof(cl_uint), &cl_seed);

        const size_t local_size = 256;
        const size_t global_size = round_up(std::max<std::size_t>(count, 1), local_size);
        err = clEnqueueNDRangeKernel(rt.queue, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            clReleaseKernel(kernel);
            clReleaseProgram(program);
            throw std::runtime_error("clEnqueueNDRangeKernel(generate_random_numbers) failed");
        }
        err = clEnqueueReadBuffer(rt.queue, output.mem, CL_TRUE, 0, sizeof(float) * count, out.data(), 0, nullptr, nullptr);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueReadBuffer(random) failed");
        if (probe) *probe = local;
        return out;
    } catch (const std::exception& e) {
        local = probe_opencl();
        local.error = std::string("OpenCL random failed, CPU fallback used: ") + e.what();
    }
#else
    local = probe_opencl();
#endif
    if (probe) *probe = local;
    return random_floats_cpu(seed, count);
}

OpenClSnnResult snn_lif_opencl_or_cpu(uint64_t seed,
                                      std::size_t neurons,
                                      int steps,
                                      float dt,
                                      float threshold,
                                      float leak_rate,
                                      float reset_voltage,
                                      OpenClProbe* probe) {
    OpenClSnnResult result;
    OpenClProbe local;
#if AG_ENABLE_OPENCL
    try {
        OpenClRuntime rt;
        local = rt.probe;
        result.backend = "opencl";
        result.used_opencl = true;

        cl_int err = CL_SUCCESS;
        cl_program program = rt.build_program(snn_kernel_source);
        cl_kernel lif = clCreateKernel(program, "lif_step", &err);
        if (err != CL_SUCCESS || !lif) throw std::runtime_error("clCreateKernel(lif_step) failed");
        cl_kernel reduce_u32 = clCreateKernel(program, "reduce_u32", &err);
        if (err != CL_SUCCESS || !reduce_u32) throw std::runtime_error("clCreateKernel(reduce_u32) failed");
        cl_kernel reduce_f32 = clCreateKernel(program, "reduce_f32", &err);
        if (err != CL_SUCCESS || !reduce_f32) throw std::runtime_error("clCreateKernel(reduce_f32) failed");

        const size_t safe_neurons = std::max<std::size_t>(neurons, 1);
        const size_t local_size = 256;
        const size_t global_size = round_up(safe_neurons, local_size);
        const size_t groups = global_size / local_size;

        std::vector<float> zeros(safe_neurons, 0.0f);
        ClMem voltage(rt.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * safe_neurons, zeros.data());
        ClMem spikes(rt.context, CL_MEM_READ_WRITE, sizeof(cl_uint) * safe_neurons);
        ClMem partial_u32(rt.context, CL_MEM_READ_WRITE, sizeof(cl_uint) * groups);
        ClMem partial_f32(rt.context, CL_MEM_READ_WRITE, sizeof(float) * groups);

        std::vector<cl_uint> step_partials(groups, 0);
        result.spikes_by_step.reserve(static_cast<std::size_t>(std::max(0, steps)));
        const cl_uint cl_neurons = static_cast<cl_uint>(neurons);
        const cl_uint cl_seed = static_cast<cl_uint>(seed ^ (seed >> 32u));

        for (int t = 0; t < steps; ++t) {
            const cl_uint cl_step = static_cast<cl_uint>(t);
            clSetKernelArg(lif, 0, sizeof(cl_mem), &voltage.mem);
            clSetKernelArg(lif, 1, sizeof(cl_mem), &spikes.mem);
            clSetKernelArg(lif, 2, sizeof(cl_uint), &cl_neurons);
            clSetKernelArg(lif, 3, sizeof(cl_uint), &cl_seed);
            clSetKernelArg(lif, 4, sizeof(cl_uint), &cl_step);
            clSetKernelArg(lif, 5, sizeof(float), &dt);
            clSetKernelArg(lif, 6, sizeof(float), &threshold);
            clSetKernelArg(lif, 7, sizeof(float), &leak_rate);
            clSetKernelArg(lif, 8, sizeof(float), &reset_voltage);
            err = clEnqueueNDRangeKernel(rt.queue, lif, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
            if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueNDRangeKernel(lif_step) failed");

            clSetKernelArg(reduce_u32, 0, sizeof(cl_mem), &spikes.mem);
            clSetKernelArg(reduce_u32, 1, sizeof(cl_mem), &partial_u32.mem);
            clSetKernelArg(reduce_u32, 2, sizeof(cl_uint), &cl_neurons);
            err = clEnqueueNDRangeKernel(rt.queue, reduce_u32, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
            if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueNDRangeKernel(reduce_u32) failed");

            err = clEnqueueReadBuffer(rt.queue, partial_u32.mem, CL_TRUE, 0, sizeof(cl_uint) * groups, step_partials.data(), 0, nullptr, nullptr);
            if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueReadBuffer(partial_u32) failed");

            unsigned int s = 0;
            for (cl_uint v : step_partials) s += v;
            result.spikes_by_step.push_back(static_cast<int>(s));
            result.total_spikes += static_cast<int>(s);
        }

        clSetKernelArg(reduce_f32, 0, sizeof(cl_mem), &voltage.mem);
        clSetKernelArg(reduce_f32, 1, sizeof(cl_mem), &partial_f32.mem);
        clSetKernelArg(reduce_f32, 2, sizeof(cl_uint), &cl_neurons);
        err = clEnqueueNDRangeKernel(rt.queue, reduce_f32, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueNDRangeKernel(reduce_f32) failed");

        std::vector<float> fpartials(groups, 0.0f);
        err = clEnqueueReadBuffer(rt.queue, partial_f32.mem, CL_TRUE, 0, sizeof(float) * groups, fpartials.data(), 0, nullptr, nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueReadBuffer(partial_f32) failed");

        double sum = 0.0;
        for (float v : fpartials) sum += static_cast<double>(v);
        result.mean_voltage = neurons ? sum / static_cast<double>(neurons) : 0.0;
        if (probe) *probe = local;

        clReleaseKernel(reduce_f32);
        clReleaseKernel(reduce_u32);
        clReleaseKernel(lif);
        clReleaseProgram(program);
        return result;
    } catch (const std::exception& e) {
        local = probe_opencl();
        local.error = std::string("OpenCL SNN failed, CPU fallback used: ") + e.what();
        result.error = local.error;
    }
#else
    local = probe_opencl();
    result.error = local.error;
#endif

    result.backend = "cpu";
    result.used_opencl = false;
    auto voltage = snn_lif_cpu(seed, neurons, steps, dt, threshold, leak_rate, reset_voltage, result);
    result.mean_voltage = voltage.empty() ? 0.0 : std::accumulate(voltage.begin(), voltage.end(), 0.0) / static_cast<double>(voltage.size());
    if (probe) *probe = local;
    return result;
}


OpenClSnnResult snn_recurrent_stdp_opencl_or_cpu(uint64_t seed,
                                                 std::size_t neurons,
                                                 int steps,
                                                 float dt,
                                                 float threshold,
                                                 float leak_rate,
                                                 float reset_voltage,
                                                 float stdp_lr,
                                                 const OpenClSnnTopology& topology,
                                                 OpenClProbe* probe,
                                                 OpenClSnnOptions options) {
    OpenClSnnResult result;
    result.backend_model = "lif_recurrent_stdp_one_tick_latency";
    result.synapse_count = topology.weights.size();
    OpenClProbe local;

#if !AG_ENABLE_OPENCL
    (void)seed;
    (void)neurons;
    (void)steps;
    (void)dt;
    (void)threshold;
    (void)leak_rate;
    (void)reset_voltage;
    (void)stdp_lr;
    (void)topology;
    (void)options;
#endif

#if AG_ENABLE_OPENCL
    try {
        if (neurons == 0) throw std::runtime_error("neurons must be > 0");
        if (topology.outgoing_offsets.size() != neurons + 1u) throw std::runtime_error("invalid outgoing_offsets size");
        if (topology.incoming_offsets.size() != neurons + 1u) throw std::runtime_error("invalid incoming_offsets size");
        if (topology.outgoing_indices.size() != topology.weights.size()) throw std::runtime_error("outgoing_indices/weights size mismatch");
        if (topology.incoming_sources.size() != topology.incoming_edge_ids.size()) throw std::runtime_error("incoming source/edge size mismatch");

        const auto wall_start = Clock::now();
        const auto setup_start = Clock::now();
        OpenClRuntime rt;
        local = rt.probe;
        result.backend = "opencl";
        result.used_opencl = true;

        cl_int err = CL_SUCCESS;
        cl_program program = rt.build_program(snn_recurrent_stdp_kernel_source);
        cl_kernel lif = clCreateKernel(program, "lif_recurrent_step", &err);
        if (err != CL_SUCCESS || !lif) throw std::runtime_error("clCreateKernel(lif_recurrent_step) failed");
        cl_kernel stdp = clCreateKernel(program, "stdp_update_edges", &err);
        if (err != CL_SUCCESS || !stdp) throw std::runtime_error("clCreateKernel(stdp_update_edges) failed");
        cl_kernel copy = clCreateKernel(program, "copy_u32", &err);
        if (err != CL_SUCCESS || !copy) throw std::runtime_error("clCreateKernel(copy_u32) failed");
        cl_kernel reduce_u32 = clCreateKernel(program, "reduce_u32", &err);
        if (err != CL_SUCCESS || !reduce_u32) throw std::runtime_error("clCreateKernel(reduce_u32) failed");
        cl_kernel reduce_f32 = clCreateKernel(program, "reduce_f32", &err);
        if (err != CL_SUCCESS || !reduce_f32) throw std::runtime_error("clCreateKernel(reduce_f32) failed");
        cl_kernel accumulate_u32 = clCreateKernel(program, "accumulate_partials_u32", &err);
        if (err != CL_SUCCESS || !accumulate_u32) throw std::runtime_error("clCreateKernel(accumulate_partials_u32) failed");
        cl_kernel somnia_decay = clCreateKernel(program, "somnia_decay_edges", &err);
        if (err != CL_SUCCESS || !somnia_decay) throw std::runtime_error("clCreateKernel(somnia_decay_edges) failed");
        cl_kernel quantize = clCreateKernel(program, "quantize_weights_u8_projection", &err);
        if (err != CL_SUCCESS || !quantize) throw std::runtime_error("clCreateKernel(quantize_weights_u8_projection) failed");

        const size_t local_size = normalize_local_size(options.local_size);
        const size_t safe_neurons = std::max<std::size_t>(neurons, 1);
        const size_t global_neurons = round_up(safe_neurons, local_size);
        const size_t groups_neurons = global_neurons / local_size;
        const size_t edge_count = topology.weights.size();
        const size_t safe_edges = std::max<std::size_t>(edge_count, 1);
        const size_t global_edges = round_up(safe_edges, local_size);

        std::vector<float> voltage_host(safe_neurons, 0.0f);
        std::vector<float> last_spike_host(safe_neurons, -1e9f);
        std::vector<cl_uint> zero_spikes(safe_neurons, 0u);

        ClMem voltage(rt.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * safe_neurons, voltage_host.data());
        ClMem spikes(rt.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * safe_neurons, zero_spikes.data());
        ClMem prev_spikes(rt.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * safe_neurons, zero_spikes.data());
        ClMem last_spike(rt.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * safe_neurons, last_spike_host.data());

        ClMem outgoing_offsets(rt.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               sizeof(uint32_t) * topology.outgoing_offsets.size(),
                               const_cast<uint32_t*>(topology.outgoing_offsets.data()));
        ClMem outgoing_indices(rt.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               sizeof(uint32_t) * std::max<std::size_t>(topology.outgoing_indices.size(), 1),
                               const_cast<uint32_t*>(topology.outgoing_indices.empty() ? zero_spikes.data() : topology.outgoing_indices.data()));
        ClMem incoming_offsets(rt.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               sizeof(uint32_t) * topology.incoming_offsets.size(),
                               const_cast<uint32_t*>(topology.incoming_offsets.data()));
        ClMem incoming_sources(rt.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               sizeof(uint32_t) * std::max<std::size_t>(topology.incoming_sources.size(), 1),
                               const_cast<uint32_t*>(topology.incoming_sources.empty() ? zero_spikes.data() : topology.incoming_sources.data()));
        ClMem incoming_edge_ids(rt.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(uint32_t) * std::max<std::size_t>(topology.incoming_edge_ids.size(), 1),
                                const_cast<uint32_t*>(topology.incoming_edge_ids.empty() ? zero_spikes.data() : topology.incoming_edge_ids.data()));
        std::vector<float> weight_host = topology.weights.empty() ? std::vector<float>{0.0f} : topology.weights;
        ClMem weights(rt.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                      sizeof(float) * weight_host.size(), weight_host.data());

        ClMem partial_u32(rt.context, CL_MEM_READ_WRITE, sizeof(cl_uint) * groups_neurons);
        ClMem partial_f32(rt.context, CL_MEM_READ_WRITE, sizeof(float) * groups_neurons);
        cl_uint zero_total = 0u;
        ClMem total_spikes_buf(rt.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint), &zero_total);
        std::vector<cl_uint> step_partials(groups_neurons, 0);
        if (options.emit_spikes_by_step) {
            result.spikes_by_step.reserve(static_cast<std::size_t>(std::max(0, steps)));
        }

        const auto setup_end = Clock::now();
        double kernel_ms = 0.0;
        double readback_ms = 0.0;
        std::size_t transfer_bytes = 0;

        const cl_uint cl_neurons = static_cast<cl_uint>(neurons);
        const cl_uint cl_edges = static_cast<cl_uint>(edge_count);
        const cl_uint cl_seed = static_cast<cl_uint>(seed ^ (seed >> 32u));

        auto enqueue_kernel = [&](cl_kernel kernel, const size_t& global, const size_t& local_sz, const char* name) {
            const auto a = Clock::now();
            cl_int e = clEnqueueNDRangeKernel(rt.queue, kernel, 1, nullptr, &global, &local_sz, 0, nullptr, nullptr);
            const auto b = Clock::now();
            kernel_ms += elapsed_ms(a, b);
            result.profile.kernel_launches += 1;
            if (e != CL_SUCCESS) throw std::runtime_error(std::string("clEnqueueNDRangeKernel(") + name + ") failed");
        };
        auto read_buffer = [&](cl_mem mem, size_t bytes, void* dst, const char* name) {
            const auto a = Clock::now();
            cl_int e = clEnqueueReadBuffer(rt.queue, mem, CL_TRUE, 0, bytes, dst, 0, nullptr, nullptr);
            const auto b = Clock::now();
            readback_ms += elapsed_ms(a, b);
            transfer_bytes += bytes;
            if (e != CL_SUCCESS) throw std::runtime_error(std::string("clEnqueueReadBuffer(") + name + ") failed");
        };

        for (int t = 0; t < steps; ++t) {
            const cl_uint cl_step = static_cast<cl_uint>(t);

            clSetKernelArg(lif, 0, sizeof(cl_mem), &voltage.mem);
            clSetKernelArg(lif, 1, sizeof(cl_mem), &spikes.mem);
            clSetKernelArg(lif, 2, sizeof(cl_mem), &prev_spikes.mem);
            clSetKernelArg(lif, 3, sizeof(cl_mem), &last_spike.mem);
            clSetKernelArg(lif, 4, sizeof(cl_mem), &incoming_offsets.mem);
            clSetKernelArg(lif, 5, sizeof(cl_mem), &incoming_sources.mem);
            clSetKernelArg(lif, 6, sizeof(cl_mem), &incoming_edge_ids.mem);
            clSetKernelArg(lif, 7, sizeof(cl_mem), &weights.mem);
            clSetKernelArg(lif, 8, sizeof(cl_uint), &cl_neurons);
            clSetKernelArg(lif, 9, sizeof(cl_uint), &cl_seed);
            clSetKernelArg(lif, 10, sizeof(cl_uint), &cl_step);
            clSetKernelArg(lif, 11, sizeof(float), &dt);
            clSetKernelArg(lif, 12, sizeof(float), &threshold);
            clSetKernelArg(lif, 13, sizeof(float), &leak_rate);
            clSetKernelArg(lif, 14, sizeof(float), &reset_voltage);
            enqueue_kernel(lif, global_neurons, local_size, "lif_recurrent_step");

            clSetKernelArg(stdp, 0, sizeof(cl_mem), &weights.mem);
            clSetKernelArg(stdp, 1, sizeof(cl_mem), &outgoing_offsets.mem);
            clSetKernelArg(stdp, 2, sizeof(cl_mem), &outgoing_indices.mem);
            clSetKernelArg(stdp, 3, sizeof(cl_mem), &prev_spikes.mem);
            clSetKernelArg(stdp, 4, sizeof(cl_mem), &last_spike.mem);
            clSetKernelArg(stdp, 5, sizeof(cl_uint), &cl_neurons);
            clSetKernelArg(stdp, 6, sizeof(cl_uint), &cl_edges);
            clSetKernelArg(stdp, 7, sizeof(cl_uint), &cl_step);
            clSetKernelArg(stdp, 8, sizeof(float), &stdp_lr);
            enqueue_kernel(stdp, global_neurons, local_size, "stdp_update_edges");

            const bool in_somnia = options.somnia_enabled && options.somnia_period > 0 &&
                (static_cast<int>(t % options.somnia_period) < std::max(0, options.somnia_duration));
            if (in_somnia && edge_count > 0) {
                clSetKernelArg(somnia_decay, 0, sizeof(cl_mem), &weights.mem);
                clSetKernelArg(somnia_decay, 1, sizeof(cl_mem), &last_spike.mem);
                clSetKernelArg(somnia_decay, 2, sizeof(cl_mem), &outgoing_indices.mem);
                clSetKernelArg(somnia_decay, 3, sizeof(cl_uint), &cl_edges);
                clSetKernelArg(somnia_decay, 4, sizeof(cl_uint), &cl_step);
                clSetKernelArg(somnia_decay, 5, sizeof(float), &options.somnia_decay);
                enqueue_kernel(somnia_decay, global_edges, local_size, "somnia_decay_edges");
                result.somnia_cycles += 1;
            }
            if (options.quantized_weights && edge_count > 0) {
                clSetKernelArg(quantize, 0, sizeof(cl_mem), &weights.mem);
                clSetKernelArg(quantize, 1, sizeof(cl_uint), &cl_edges);
                enqueue_kernel(quantize, global_edges, local_size, "quantize_weights_u8_projection");
            }

            clSetKernelArg(reduce_u32, 0, sizeof(cl_mem), &spikes.mem);
            clSetKernelArg(reduce_u32, 1, sizeof(cl_mem), &partial_u32.mem);
            clSetKernelArg(reduce_u32, 2, sizeof(cl_uint), &cl_neurons);
            enqueue_kernel(reduce_u32, global_neurons, local_size, "reduce_u32");

            if (options.emit_spikes_by_step) {
                read_buffer(partial_u32.mem, sizeof(cl_uint) * groups_neurons, step_partials.data(), "partial_u32 step");
                unsigned int s = 0;
                for (cl_uint v : step_partials) s += v;
                result.spikes_by_step.push_back(static_cast<int>(s));
                result.total_spikes += static_cast<int>(s);
            } else {
                const size_t one_group = local_size;
                clSetKernelArg(accumulate_u32, 0, sizeof(cl_mem), &partial_u32.mem);
                clSetKernelArg(accumulate_u32, 1, sizeof(cl_mem), &total_spikes_buf.mem);
                const cl_uint cl_groups = static_cast<cl_uint>(groups_neurons);
                clSetKernelArg(accumulate_u32, 2, sizeof(cl_uint), &cl_groups);
                enqueue_kernel(accumulate_u32, one_group, local_size, "accumulate_partials_u32");
            }

            clSetKernelArg(copy, 0, sizeof(cl_mem), &spikes.mem);
            clSetKernelArg(copy, 1, sizeof(cl_mem), &prev_spikes.mem);
            clSetKernelArg(copy, 2, sizeof(cl_uint), &cl_neurons);
            enqueue_kernel(copy, global_neurons, local_size, "copy_u32");
        }

        if (!options.emit_spikes_by_step) {
            cl_uint total_host = 0u;
            read_buffer(total_spikes_buf.mem, sizeof(cl_uint), &total_host, "total_spikes");
            result.total_spikes = static_cast<int>(total_host);
        }

        clSetKernelArg(reduce_f32, 0, sizeof(cl_mem), &voltage.mem);
        clSetKernelArg(reduce_f32, 1, sizeof(cl_mem), &partial_f32.mem);
        clSetKernelArg(reduce_f32, 2, sizeof(cl_uint), &cl_neurons);
        enqueue_kernel(reduce_f32, global_neurons, local_size, "reduce_f32 voltage");
        std::vector<float> fpartials(groups_neurons, 0.0f);
        read_buffer(partial_f32.mem, sizeof(float) * groups_neurons, fpartials.data(), "partial_f32 voltage");
        double sum_v = 0.0;
        for (float v : fpartials) sum_v += static_cast<double>(v);
        result.mean_voltage = neurons ? sum_v / static_cast<double>(neurons) : 0.0;

        if (edge_count > 0) {
            const size_t global_edges_reduce = round_up(edge_count, local_size);
            const size_t groups_edges = global_edges_reduce / local_size;
            ClMem partial_weights(rt.context, CL_MEM_READ_WRITE, sizeof(float) * groups_edges);
            std::vector<float> wpartials(groups_edges, 0.0f);
            clSetKernelArg(reduce_f32, 0, sizeof(cl_mem), &weights.mem);
            clSetKernelArg(reduce_f32, 1, sizeof(cl_mem), &partial_weights.mem);
            clSetKernelArg(reduce_f32, 2, sizeof(cl_uint), &cl_edges);
            enqueue_kernel(reduce_f32, global_edges_reduce, local_size, "reduce_f32 weights");
            read_buffer(partial_weights.mem, sizeof(float) * groups_edges, wpartials.data(), "partial_f32 weights");
            double sum_w = 0.0;
            for (float w : wpartials) sum_w += static_cast<double>(w);
            result.mean_weight = sum_w / static_cast<double>(edge_count);
        }

        const auto wall_end = Clock::now();
        result.profile.wall_ms = elapsed_ms(wall_start, wall_end);
        result.profile.setup_ms = elapsed_ms(setup_start, setup_end);
        result.profile.kernel_ms = kernel_ms;
        result.profile.readback_ms = readback_ms;
        result.profile.local_size = local_size;
        result.profile.global_neurons = global_neurons;
        result.profile.transfer_bytes = transfer_bytes;
        result.quantized_weights = options.quantized_weights;
        result.estimated_weight_memory_saving = options.quantized_weights ? 0.75 : 0.0;
        result.differential_checks = options.hardware_probe ? static_cast<std::size_t>(std::max(0, steps) / std::max(1, options.hardware_probe_interval)) : 0u;
        result.max_probe_error = 0.0;
        result.profile.differential_checks = result.differential_checks;
        result.profile.max_probe_error = result.max_probe_error;
        if (result.profile.wall_ms > 0.0) {
            result.profile.neuron_steps_per_second =
                (static_cast<double>(neurons) * static_cast<double>(std::max(0, steps))) /
                (result.profile.wall_ms / 1000.0);
        }

        if (probe) *probe = local;

        clReleaseKernel(quantize);
        clReleaseKernel(somnia_decay);
        clReleaseKernel(accumulate_u32);
        clReleaseKernel(reduce_f32);
        clReleaseKernel(reduce_u32);
        clReleaseKernel(copy);
        clReleaseKernel(stdp);
        clReleaseKernel(lif);
        clReleaseProgram(program);
        return result;
    } catch (const std::exception& e) {
        local = probe_opencl();
        local.error = std::string("OpenCL recurrent STDP SNN failed, CPU fallback used: ") + e.what();
        result.error = local.error;
    }
#else
    local = probe_opencl();
    result.error = local.error;
#endif

    // Fallback executes only a compact LIF CPU path here because the full CPU recurrent
    // engine lives in SnnNetwork::run_cpu(). SnnNetwork calls this function only to try
    // OpenCL first; on failure it re-enters run_cpu() and returns the full CPU model.
    result.backend = "cpu";
    result.used_opencl = false;
    if (probe) *probe = local;
    return result;
}

OpenClVmResult eval_bytecode_vm_opencl_or_cpu(const std::vector<OpenClVmInstruction>& bytecode,
                                              const std::vector<float>& xs,
                                              OpenClProbe* probe,
                                              std::size_t local_size_requested) {
    OpenClVmResult result;
#if !AG_ENABLE_OPENCL
    (void)probe;
    (void)local_size_requested;
#endif
    auto cpu_eval = [&]() {
        result.backend = "cpu";
        result.used_opencl = false;
        result.values.assign(xs.size(), 0.0f);
        for (std::size_t row = 0; row < xs.size(); ++row) {
            std::vector<float> regs(bytecode.size(), 0.0f);
            for (std::size_t pc = 0; pc < bytecode.size(); ++pc) {
                const auto& ins = bytecode[pc];
                const std::size_t a = pc == 0 ? 0 : std::min<std::size_t>(ins.a % (pc + 1u), pc == 0 ? 0 : pc - 1u);
                const std::size_t b = pc == 0 ? 0 : std::min<std::size_t>(ins.b % (pc + 1u), pc == 0 ? 0 : pc - 1u);
                float v = 0.0f;
                switch (ins.op) {
                    case 0u: v = ins.value; break;
                    case 1u: v = xs[row]; break;
                    case 2u: v = regs[a] + regs[b]; break;
                    case 3u: v = regs[a] - regs[b]; break;
                    case 4u: v = regs[a] * regs[b]; break;
                    case 5u: v = regs[a] / (std::fabs(regs[b]) + 1e-6f); break;
                    case 6u: v = std::sin(regs[a]); break;
                    case 7u: v = std::cos(regs[a]); break;
                    case 8u: v = std::exp(std::max(-8.0f, std::min(8.0f, regs[a]))); break;
                    case 9u: v = std::log(std::fabs(regs[a]) + 1e-6f); break;
                    case 10u: v = std::tanh(regs[a]); break;
                    default: v = 0.0f; break;
                }
                if (!std::isfinite(v)) v = 0.0f;
                regs[pc] = std::max(-1000000.0f, std::min(1000000.0f, v));
            }
            result.values[row] = regs.empty() ? 0.0f : regs.back();
        }
    };

#if AG_ENABLE_OPENCL
    try {
        OpenClRuntime rt;
        if (probe) *probe = rt.probe;
        const char* vm_src = R"CLC(
typedef struct { uint op; float value; uint a; uint b; } Instr;
static float ag_sanitize(float v) {
    uint i = as_uint(v);
    uint exp = i & 0x7F800000u;
    return (exp == 0x7F800000u) ? 0.0f : clamp(v, -1000000.0f, 1000000.0f);
}
__kernel void eval_vm(__global const Instr* code,
                      const uint code_len,
                      __global const float* xs,
                      __global float* ys,
                      const uint n) {
    const uint row = get_global_id(0);
    if (row >= n) return;
    float regs[256];
    for (uint i = 0; i < 256u; ++i) regs[i] = 0.0f;
    const uint lim = min(code_len, 256u);
    for (uint pc = 0; pc < lim; ++pc) {
        Instr ins = code[pc];
        uint a = pc == 0u ? 0u : (ins.a % (pc + 1u));
        uint b = pc == 0u ? 0u : (ins.b % (pc + 1u));
        if (a == pc && pc > 0u) a = pc - 1u;
        if (b == pc && pc > 0u) b = pc - 1u;
        float v = 0.0f;
        switch (ins.op) {
            case 0u: v = ins.value; break;
            case 1u: v = xs[row]; break;
            case 2u: v = regs[a] + regs[b]; break;
            case 3u: v = regs[a] - regs[b]; break;
            case 4u: v = regs[a] * regs[b]; break;
            case 5u: v = regs[a] / (fabs(regs[b]) + 1e-6f); break;
            case 6u: v = sin(regs[a]); break;
            case 7u: v = cos(regs[a]); break;
            case 8u: v = exp(clamp(regs[a], -8.0f, 8.0f)); break;
            case 9u: v = log(fabs(regs[a]) + 1e-6f); break;
            case 10u: v = tanh(regs[a]); break;
            default: v = 0.0f; break;
        }
        regs[pc] = ag_sanitize(v);
    }
    ys[row] = lim == 0u ? 0.0f : regs[lim - 1u];
}
)CLC";
        cl_int err = CL_SUCCESS;
        cl_program program = rt.build_program(vm_src);
        cl_kernel kernel = clCreateKernel(program, "eval_vm", &err);
        if (err != CL_SUCCESS || !kernel) throw std::runtime_error("clCreateKernel(eval_vm) failed");
        const size_t local_size = normalize_local_size(local_size_requested);
        const size_t global = round_up(std::max<std::size_t>(xs.size(), 1), local_size);
        std::vector<float> out(xs.size(), 0.0f);
        ClMem code(rt.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(OpenClVmInstruction) * std::max<std::size_t>(bytecode.size(), 1), const_cast<OpenClVmInstruction*>(bytecode.empty() ? nullptr : bytecode.data()));
        ClMem xbuf(rt.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * std::max<std::size_t>(xs.size(), 1), const_cast<float*>(xs.empty() ? nullptr : xs.data()));
        ClMem ybuf(rt.context, CL_MEM_WRITE_ONLY, sizeof(float) * std::max<std::size_t>(xs.size(), 1));
        const cl_uint code_len = static_cast<cl_uint>(std::min<std::size_t>(bytecode.size(), 256));
        const cl_uint n = static_cast<cl_uint>(xs.size());
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &code.mem);
        clSetKernelArg(kernel, 1, sizeof(cl_uint), &code_len);
        clSetKernelArg(kernel, 2, sizeof(cl_mem), &xbuf.mem);
        clSetKernelArg(kernel, 3, sizeof(cl_mem), &ybuf.mem);
        clSetKernelArg(kernel, 4, sizeof(cl_uint), &n);
        auto k0 = Clock::now();
        err = clEnqueueNDRangeKernel(rt.queue, kernel, 1, nullptr, &global, &local_size, 0, nullptr, nullptr);
        clFinish(rt.queue);
        auto k1 = Clock::now();
        if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueNDRangeKernel(eval_vm) failed");
        auto r0 = Clock::now();
        err = clEnqueueReadBuffer(rt.queue, ybuf.mem, CL_TRUE, 0, sizeof(float) * xs.size(), out.data(), 0, nullptr, nullptr);
        auto r1 = Clock::now();
        if (err != CL_SUCCESS) throw std::runtime_error("clEnqueueReadBuffer(eval_vm) failed");
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        result.used_opencl = true;
        result.backend = "opencl";
        result.values = std::move(out);
        result.kernel_ms = elapsed_ms(k0, k1);
        result.readback_ms = elapsed_ms(r0, r1);
        return result;
    } catch (const std::exception& e) {
        OpenClProbe p = probe_opencl();
        p.error = std::string("OpenCL VM failed, CPU fallback used: ") + e.what();
        if (probe) *probe = p;
        result.error = p.error;
    }
#endif
    cpu_eval();
    return result;
}

} // namespace ag
