
from __future__ import annotations

from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse
import json
import html

from .core import TriadicGenesisEngine, TriadicConfig, parse_series, synthetic_series, benchmark, LINEAGE


INDEX = """<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<title>Triadic Genesis Engine</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
:root{--bg:#0b1020;--panel:#131a2e;--line:#29324d;--txt:#e8ecff;--muted:#a7b0ca;--a:#8bd3ff;--b:#b28cff;--g:#7af0b1;--r:#ff6b6b}
*{box-sizing:border-box} body{margin:0;background:radial-gradient(circle at top left,#1d254a,var(--bg) 45%);color:var(--txt);font-family:Inter,Segoe UI,system-ui,sans-serif}
header,main{padding:2rem 3rem} header{border-bottom:1px solid var(--line)} h1{margin:0;font-size:2.2rem} p{color:var(--muted)} .panel{background:linear-gradient(180deg,#151c32,#101729);border:1px solid var(--line);border-radius:18px;padding:1.2rem;margin:1rem 0;box-shadow:0 20px 40px #0005}
textarea{width:100%;height:180px;background:#070b15;color:var(--txt);border:1px solid var(--line);border-radius:12px;padding:1rem;font-family:Consolas,monospace}
input{background:#070b15;color:var(--txt);border:1px solid var(--line);border-radius:10px;padding:.7rem} button{background:linear-gradient(135deg,var(--a),var(--b));border:0;border-radius:12px;padding:.8rem 1rem;font-weight:700;cursor:pointer}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:.8rem}.metric{background:#080d1a;border:1px solid var(--line);border-radius:14px;padding:.8rem}.metric span{color:var(--muted);display:block}.metric b{color:var(--a);font-size:1.1rem}
pre{background:#070b15;border:1px solid var(--line);border-radius:12px;padding:1rem;white-space:pre-wrap;overflow:auto;max-height:500px} canvas{width:100%;height:auto;border:1px solid var(--line);border-radius:12px;background:#070b15}
</style>
</head>
<body>
<header>
<h1>Triadic Genesis Engine</h1>
<p>Fusion aus drei generierten Algorithmen: chaotische Exploration + kontraktiver Anker + Sequenz-Extrapolator.</p>
</header>
<main>
<section class="panel">
<h2>Daten eingeben</h2>
<p>CSV, Leerzeichen oder Zeilenumbrüche. Leer lassen für synthetische Demo-Serie.</p>
<textarea id="data"></textarea>
<div class="grid"><label>Horizon <input id="horizon" type="number" value="16"></label><label>Window <input id="window" type="number" value="64"></label></div>
<p><button onclick="run()">Analyse starten</button> <button onclick="demo()">Demo-Daten laden</button> <button onclick="lineage()">Lineage anzeigen</button></p>
</section>
<section class="panel">
<h2>Metriken</h2><div id="metrics" class="grid"></div>
<canvas id="chart" width="1100" height="280"></canvas>
</section>
<section class="panel"><h2>JSON</h2><pre id="out">Noch keine Analyse.</pre></section>
</main>
<script>
function $(id){return document.getElementById(id)}
function demo(){let arr=[]; for(let i=0;i<160;i++){let x=.03*i+Math.sin(i*.17)+.35*Math.sin(i*.031); if([45,92,130].includes(i)) x += i===92 ? -3.2 : 4; arr.push(x.toFixed(6));} $('data').value=arr.join("\\n")}
async function lineage(){let r=await fetch('/api/lineage'); $('out').textContent=JSON.stringify(await r.json(),null,2)}
function draw(vals, forecasts){let c=$('chart'),ctx=c.getContext('2d'),w=c.width,h=c.height;ctx.clearRect(0,0,w,h);ctx.fillStyle='#070b15';ctx.fillRect(0,0,w,h);let all=vals.concat(forecasts||[]); if(!all.length)return; let mn=Math.min(...all),mx=Math.max(...all); if(mx-mn<1e-9){mx=mn+1} ctx.strokeStyle='#29324d'; for(let i=0;i<=5;i++){let y=i*h/5;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke()} function line(a,offset,color){ctx.strokeStyle=color;ctx.lineWidth=2;ctx.beginPath();a.forEach((v,i)=>{let x=(i+offset)*w/Math.max(1,all.length-1),y=h-((v-mn)/(mx-mn))*h;if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y)});ctx.stroke()} line(vals,0,'#8bd3ff'); line(forecasts,vals.length,'#b28cff')}
async function run(){let payload={data:$('data').value,horizon:Number($('horizon').value),window:Number($('window').value)}; let r=await fetch('/api/run',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)}); let j=await r.json(); $('out').textContent=JSON.stringify(j,null,2); let m=j.metrics||{}; $('metrics').innerHTML=Object.entries(m).map(([k,v])=>`<div class="metric"><span>${k}</span><b>${typeof v==='number'?v.toFixed(5):v}</b></div>`).join(''); draw((j.steps||[]).map(s=>s.observation),j.forecasts||[])}
</script>
</body>
</html>"""


class Handler(BaseHTTPRequestHandler):
    def _send(self, payload: str, content_type: str = "text/plain; charset=utf-8", status: int = 200) -> None:
        data = payload.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _json(self, obj, status: int = 200) -> None:
        self._send(json.dumps(obj, indent=2), "application/json; charset=utf-8", status)

    def do_GET(self):  # noqa: N802
        path = urlparse(self.path).path
        if path == "/":
            self._send(INDEX, "text/html; charset=utf-8")
        elif path == "/api/lineage":
            self._json([x.__dict__ for x in LINEAGE])
        elif path == "/api/demo":
            self._json({"values": synthetic_series()})
        else:
            self._json({"error": "not found"}, 404)

    def do_POST(self):  # noqa: N802
        path = urlparse(self.path).path
        if path != "/api/run":
            return self._json({"error": "not found"}, 404)
        length = int(self.headers.get("Content-Length", "0"))
        body = self.rfile.read(length).decode("utf-8")
        try:
            payload = json.loads(body)
            values = parse_series(payload.get("data", "")) if payload.get("data") else synthetic_series()
            cfg = TriadicConfig(horizon=int(payload.get("horizon", 16)), window=int(payload.get("window", 64)))
            report = TriadicGenesisEngine(cfg).run(values, horizon=cfg.horizon)
            return self._json(report.to_dict())
        except Exception as exc:  # noqa: BLE001
            return self._json({"error": str(exc)}, 500)


def run_server(host: str = "127.0.0.1", port: int = 8777) -> None:
    print(f"Triadic Genesis Engine Web UI: http://{host}:{port}")
    ThreadingHTTPServer((host, port), Handler).serve_forever()
