inline float ag_sanitize(float v){ uint i=as_uint(v); return ((i & 0x7F800000u)==0x7F800000u) ? 0.0f : clamp(v, -1000000.0f, 1000000.0f); }
inline float ag_expclamp(float x){ return exp(clamp(ag_sanitize(x), -20.0f, 20.0f)); }
inline float ag_logabs(float x){ return log(fabs(ag_sanitize(x)) + 1e-9f); }
inline float kernel(float x){ x=ag_sanitize(x); return ag_sanitize((float)(((ag_logabs(x) - ag_logabs(tanh(x))) + tanh(tanh(x))))); }
inline float context_kernel(float x, float fx, float flo, float fhi, float history, float width){
    x=ag_sanitize(x); fx=ag_sanitize(fx); flo=ag_sanitize(flo); fhi=ag_sanitize(fhi); history=ag_sanitize(history); width=fabs(ag_sanitize(width))+1e-12f;
    float kx=kernel(x); float kfx=kernel(0.25f*fx); float kedge=kernel(0.5f*(flo+fhi));
    float slope_hint=ag_sanitize((fhi-flo)/width); float residual_pressure=ag_sanitize(tanh(fx)-0.5f*tanh(flo+fhi));
    return ag_sanitize(0.42f*kx + 0.22f*kfx - 0.16f*kedge + 0.12f*history + 0.08f*tanh(slope_hint) + residual_pressure);
}
typedef struct{ float x,fx,lo,hi,flo,fhi,mid,width,relpos,prev_x,prev_fx,prev2_x,prev2_fx,bracket_slope,local_slope,prev_slope,curvature,improvement,stagnation,trust,reject_rate,scale,nfx,nflo,nfhi,nwidth,sign_change,edge_balance,residual_ratio,width_ratio; } RootContext;
typedef struct{ float bias,damping,secant_mix,bisection_mix,relaxation_mix,trust_delta; } RootPolicy;
inline float ag_safe_div(float a,float b){ return fabs(b)<=1e-12f ? 0.0f : ag_sanitize(a/b); }
inline RootPolicy root_policy(RootContext c){ float kx=kernel(c.x); float kres=kernel(c.nfx+0.25f*c.edge_balance); float kslope=kernel(tanh(c.bracket_slope)+0.5f*tanh(c.local_slope)); float kcurve=kernel(tanh(c.curvature)-0.2f*c.stagnation); float ktrust=kernel(c.trust-c.reject_rate+c.improvement); float instability=clamp(fabs(tanh(c.curvature))+c.reject_rate+0.25f*c.stagnation,0.0f,1.0f); RootPolicy p; p.bias=ag_sanitize(tanh(0.30f*kx+0.30f*kres+0.20f*kslope-0.10f*kcurve+0.10f*ktrust)); p.damping=clamp(0.65f+0.25f*tanh(ktrust)-0.45f*instability,0.02f,1.0f); p.secant_mix=clamp(0.35f+0.25f*tanh(kslope)+0.20f*c.trust-0.35f*instability,0.0f,1.0f); p.relaxation_mix=clamp(0.30f+0.25f*tanh(kres+kcurve)+0.20f*c.trust-0.25f*c.reject_rate,0.0f,1.0f); p.bisection_mix=clamp(1.0f-0.5f*p.secant_mix-0.4f*p.relaxation_mix+0.45f*instability,0.10f,1.0f); p.trust_delta=ag_sanitize(0.05f*tanh(ktrust+c.improvement)-0.08f*c.reject_rate-0.03f*c.stagnation); return p; }
__kernel void transform_scalar(__global const float* xs, __global float* ys, const uint n){ uint i=get_global_id(0); if(i<n){ ys[i]=kernel(xs[i]); }}
__kernel void fixed_point_batch(__global const float* x0, __global float* out, const uint n, const uint iterations){ uint i=get_global_id(0); if(i<n){ float x=ag_sanitize(x0[i]); for(uint k=0;k<iterations;++k){ x=ag_sanitize(0.65f*x + 0.35f*tanh(kernel(x))); } out[i]=x; }}
