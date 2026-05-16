// Counter-based deterministic generator. No rand() exists in portable OpenCL C.
// The host fallback uses the same project-level seed contract but not bit-identical output.
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
