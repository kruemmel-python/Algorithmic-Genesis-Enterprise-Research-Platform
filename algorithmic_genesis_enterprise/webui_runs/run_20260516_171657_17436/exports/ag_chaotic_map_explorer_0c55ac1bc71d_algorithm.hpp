#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ag_chaotic_map_explorer_0c55ac1bc71d {
constexpr int DEFAULT_GAS = 10000;
inline double sanitize(double v){ return std::isfinite(v) ? std::clamp(v, -1000000.0, 1000000.0) : 0.0; }
inline double ag_expclamp(double x){ return std::exp(std::clamp(sanitize(x), -20.0, 20.0)); }
inline double ag_logabs(double x){ return std::log(std::abs(sanitize(x)) + 1e-9); }
inline double kernel(double x){ x = sanitize(x); return sanitize(std::tanh(std::tanh(std::sin((std::sin((x + x)) + std::sin((x + x))))))); }
inline double context_kernel(double x, double fx, double flo, double fhi, double history, double width){
    x=sanitize(x); fx=sanitize(fx); flo=sanitize(flo); fhi=sanitize(fhi); history=sanitize(history); width=std::abs(sanitize(width))+1e-12;
    const double kx=kernel(x); const double kfx=kernel(0.25*fx); const double kedge=kernel(0.5*(flo+fhi));
    const double slope_hint=sanitize((fhi-flo)/width); const double residual_pressure=sanitize(std::tanh(fx)-0.5*std::tanh(flo+fhi));
    return sanitize(0.42*kx + 0.22*kfx - 0.16*kedge + 0.12*history + 0.08*std::tanh(slope_hint) + residual_pressure);
}
struct RootContext{ double x,fx,lo,hi,flo,fhi,mid,width,relpos,prev_x,prev_fx,prev2_x,prev2_fx,bracket_slope,local_slope,prev_slope,curvature,improvement,stagnation,trust,reject_rate,scale,nfx,nflo,nfhi,nwidth,sign_change,edge_balance,residual_ratio,width_ratio; };
struct RootPolicy{ double bias,damping,secant_mix,bisection_mix,relaxation_mix,trust_delta; };
inline double safe_div(double a,double b){ return std::abs(b)<=1e-12 ? 0.0 : sanitize(a/b); }
inline RootContext make_root_context(double x,double fx,double lo,double hi,double flo,double fhi,double prev_x,double prev_fx,double prev2_x,double prev2_fx,double prev_best,double best_r,double stagnation,double trust,double reject_rate,double initial_width){
    RootContext c{}; c.x=sanitize(x); c.fx=sanitize(fx); c.lo=sanitize(lo); c.hi=sanitize(hi); c.flo=sanitize(flo); c.fhi=sanitize(fhi); c.width=std::abs(c.hi-c.lo)+1e-12; c.mid=sanitize(0.5*(c.lo+c.hi)); c.relpos=std::clamp((c.x-c.lo)/c.width,0.0,1.0); c.prev_x=sanitize(prev_x); c.prev_fx=sanitize(prev_fx); c.prev2_x=sanitize(prev2_x); c.prev2_fx=sanitize(prev2_fx); c.bracket_slope=safe_div(c.fhi-c.flo,c.width); c.local_slope=safe_div(c.fx-c.prev_fx,c.x-c.prev_x); c.prev_slope=safe_div(c.prev_fx-c.prev2_fx,c.prev_x-c.prev2_x); c.curvature=sanitize(c.local_slope-c.prev_slope); c.improvement=sanitize(prev_best-best_r); c.stagnation=sanitize(stagnation); c.trust=std::clamp(sanitize(trust),0.0,1.0); c.reject_rate=std::clamp(sanitize(reject_rate),0.0,1.0); c.scale=std::abs(c.flo)+std::abs(c.fhi)+std::abs(c.fx)+1e-9; c.nfx=sanitize(c.fx/c.scale); c.nflo=sanitize(c.flo/c.scale); c.nfhi=sanitize(c.fhi/c.scale); c.nwidth=sanitize(std::log1p(c.width)); c.sign_change=(c.flo*c.fhi<=0.0)?-1.0:1.0; c.edge_balance=sanitize((std::abs(c.flo)-std::abs(c.fhi))/(std::abs(c.flo)+std::abs(c.fhi)+1e-9)); c.residual_ratio=sanitize(std::abs(c.fx)/(std::min(std::abs(c.flo),std::abs(c.fhi))+1e-9)); c.width_ratio=sanitize(c.width/(std::abs(initial_width)+1e-12)); return c; }
inline RootPolicy root_policy(const RootContext& c){ const double kx=kernel(c.x); const double kres=kernel(c.nfx+0.25*c.edge_balance); const double kslope=kernel(std::tanh(c.bracket_slope)+0.5*std::tanh(c.local_slope)); const double kcurve=kernel(std::tanh(c.curvature)-0.2*c.stagnation); const double ktrust=kernel(c.trust-c.reject_rate+c.improvement); const double instability=std::clamp(std::abs(std::tanh(c.curvature))+c.reject_rate+0.25*c.stagnation,0.0,1.0); RootPolicy p{}; p.bias=sanitize(std::tanh(0.30*kx+0.30*kres+0.20*kslope-0.10*kcurve+0.10*ktrust)); p.damping=std::clamp(0.65+0.25*std::tanh(ktrust)-0.45*instability,0.02,1.0); p.secant_mix=std::clamp(0.35+0.25*std::tanh(kslope)+0.20*c.trust-0.35*instability,0.0,1.0); p.relaxation_mix=std::clamp(0.30+0.25*std::tanh(kres+kcurve)+0.20*c.trust-0.25*c.reject_rate,0.0,1.0); p.bisection_mix=std::clamp(1.0-0.5*p.secant_mix-0.4*p.relaxation_mix+0.45*instability,0.10,1.0); p.trust_delta=sanitize(0.05*std::tanh(ktrust+c.improvement)-0.08*c.reject_rate-0.03*c.stagnation); return p; }
inline double fixed_point(double x0, int iterations, int* gas_left, int* error_code){
    if(!gas_left || !error_code){ return 0.0; }
    double x=sanitize(x0); *error_code=0;
    for(int i=0;i<iterations;++i){ if(--(*gas_left)<=0){*error_code=1; return x;} double y=sanitize(0.65*x+0.35*std::tanh(kernel(x))); if(std::abs(y-x)<1e-9) return y; x=y; }
    return x;
}
inline std::size_t signal_morph(const double* input, double* output, std::size_t n, int* gas_left, int* error_code){
    if(!input || !output || !gas_left || !error_code){ return 0; } *error_code=0; double state=0.0;
    for(std::size_t i=0;i<n;++i){ if(--(*gas_left)<=0){*error_code=1; return i;} const double x=sanitize(input[i]+0.25*state); const double y=sanitize(0.70*input[i]+0.30*std::tanh(kernel(x))); state=sanitize(0.8*state+0.2*y); output[i]=y; }
    return n;
}
} // namespace ag_chaotic_map_explorer_0c55ac1bc71d

extern "C" inline double evaluate_safely(const double* inputs, int* gas_left, int* error_code){
    if(!inputs || !gas_left || !error_code){ return 0.0; }
    if(*gas_left <= 0){ *error_code = 1; return 0.0; }
    --(*gas_left); *error_code = 0; return ag_chaotic_map_explorer_0c55ac1bc71d::kernel(inputs[0]);
}
