
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from .core import TriadicConfig, TriadicGenesisEngine, benchmark, parse_series, synthetic_series, LINEAGE


def _load_values(path: str | None) -> list[float]:
    if not path:
        return synthetic_series()
    text = Path(path).read_text(encoding="utf-8")
    return parse_series(text)


def cmd_lineage(args: argparse.Namespace) -> int:
    print(json.dumps([x.__dict__ for x in LINEAGE], indent=2))
    return 0


def cmd_demo(args: argparse.Namespace) -> int:
    values = synthetic_series(args.samples)
    engine = TriadicGenesisEngine(TriadicConfig(horizon=args.horizon))
    report = engine.run(values, horizon=args.horizon)
    print(report.to_json())
    return 0


def cmd_forecast(args: argparse.Namespace) -> int:
    values = _load_values(args.data)
    cfg = TriadicConfig(horizon=args.horizon, window=args.window)
    engine = TriadicGenesisEngine(cfg)
    report = engine.run(values, horizon=args.horizon)
    output = report.to_dict()
    if args.json:
        Path(args.json).write_text(json.dumps(output, indent=2), encoding="utf-8")
        print(json.dumps({"written": args.json, "metrics": output.get("metrics", output.get("baseline", {}))}, indent=2))
    else:
        print(json.dumps(output, indent=2))
    return 0


def cmd_benchmark(args: argparse.Namespace) -> int:
    values = _load_values(args.data)
    output = benchmark(values, horizon=args.horizon)
    if args.json:
        Path(args.json).write_text(json.dumps(output, indent=2), encoding="utf-8")
        print(json.dumps({"written": args.json, "baseline": output.get("baseline", {})}, indent=2))
    else:
        print(json.dumps(output, indent=2))
    return 0


def cmd_self_test(args: argparse.Namespace) -> int:
    values = synthetic_series(80)
    report = TriadicGenesisEngine().run(values, horizon=8)
    assert len(report.steps) == 80
    assert len(report.forecasts) == 8
    assert all(abs(x) < 1_000_000 for x in report.forecasts)
    output = benchmark(values)
    assert "baseline" in output
    print("ok")
    return 0


def cmd_web(args: argparse.Namespace) -> int:
    from .webapp import run_server
    run_server(host=args.host, port=args.port)
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="triadic-genesis")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("lineage", help="show the three connected source algorithms")
    p.set_defaults(func=cmd_lineage)

    p = sub.add_parser("demo", help="run deterministic demo on synthetic data")
    p.add_argument("--samples", type=int, default=160)
    p.add_argument("--horizon", type=int, default=16)
    p.set_defaults(func=cmd_demo)

    p = sub.add_parser("forecast", help="run triadic engine on a data file")
    p.add_argument("--data", help="CSV/newline/space separated numeric values")
    p.add_argument("--horizon", type=int, default=16)
    p.add_argument("--window", type=int, default=64)
    p.add_argument("--json", help="write result JSON")
    p.set_defaults(func=cmd_forecast)

    p = sub.add_parser("benchmark", help="compare engine against naive and delta baselines")
    p.add_argument("--data", help="CSV/newline/space separated numeric values")
    p.add_argument("--horizon", type=int, default=16)
    p.add_argument("--json", help="write benchmark JSON")
    p.set_defaults(func=cmd_benchmark)

    p = sub.add_parser("self-test", help="run embedded assertions")
    p.set_defaults(func=cmd_self_test)

    p = sub.add_parser("web", help="start local research web UI")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, default=8777)
    p.set_defaults(func=cmd_web)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main())
