import math
import ag_chaotic_map_explorer_0c55ac1bc71d_algorithm as alg

def test_finite_kernel():
    for x in [-10,-1,0,0.5,1,10,float('nan'),float('inf')]:
        y = alg.kernel(x)
        assert math.isfinite(y)

def test_gas_limit():
    y, err = alg.transform_scalar(0.5, 0)
    assert err == 1

def test_algorithm_smoke():
    y, err = alg.fixed_point(0.25, iterations=16)
    assert math.isfinite(y)
    ys, err = alg.signal_morph([0.0, 0.5, 1.0])
    assert len(ys) == 3
    assert all(math.isfinite(v) for v in ys)
    z, err = alg.predict_next([0.0, 0.5, 0.75])
    assert math.isfinite(z)
    r, err = alg.root_refine(lambda x: x*x - 2.0, 0.0, 2.0, iterations=64)
    assert err in (0, 1)
    assert 0.0 <= r <= 2.0
    assert abs(r*r - 2.0) < 1e-4
    ctxv = alg.context_kernel(0.5, -0.2, -1.0, 1.0, 0.1, 2.0)
    assert math.isfinite(ctxv)
    ctx = alg.make_root_context(0.5, -0.2, 0.0, 2.0, -2.0, 2.0, 0.4, -0.3, 0.3, -0.4, 0.5, 0.2, 0.0, 0.5, 0.0, 2.0)
    assert all(k in ctx for k in alg.ROOT_FEATURES)
    pol = alg.root_policy(ctx)
    assert all(math.isfinite(v) for v in pol.values())

if __name__ == '__main__':
    test_finite_kernel(); test_gas_limit(); test_algorithm_smoke(); print('ok')
