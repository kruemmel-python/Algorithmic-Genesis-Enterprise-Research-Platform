// Recurrent LIF/STDP OpenCL kernels. Host embeds the same source for deployment.

static uint ag_xorshift(uint x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
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
        if (prev_spikes[src] != 0u) syn += weights[edge];
    }
    v += syn;

    uint r = ag_xorshift(seed ^ (i * 747796405u) ^ (step * 2891336453u));
    const float u = (float)(r & 0x00FFFFFFu) / (float)0x01000000u;
    if (u < 0.03f) {
        uint r2 = ag_xorshift(r + 0x9e3779b9u);
        const float amp = 0.35f + 0.2f * ((float)(r2 & 0x00FFFFFFu) / (float)0x01000000u);
        v += amp;
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
        weights[e] = w;
    }
}

__kernel void copy_u32(__global const uint* src,
                       __global uint* dst,
                       const uint count) {
    const uint i = get_global_id(0);
    if (i < count) dst[i] = src[i];
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
