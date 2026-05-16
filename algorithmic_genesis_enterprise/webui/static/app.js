let catalog = null;
let selected = new Set(["tanh", "curvature", "secant_mix", "bracket_guard", "finite_sanitize"]);
let currentJob = null;

const $ = id => document.getElementById(id);

async function api(path, opts={}) {
  const res = await fetch(path, opts);
  const text = await res.text();
  try { return JSON.parse(text); } catch { return text; }
}

function buildManifest() {
  return {
    name: $("name").value,
    domain: $("domain").value,
    profile: $("profile").value,
    seed: Number($("seed").value),
    population: Number($("population").value),
    generations: Number($("generations").value),
    generator: $("generator").value,
    fitness_backend: $("fitness_backend").value,
    vm_differential: true,
    require_nontrivial: true,
    genome_length: 48,
    samples: 128,
    save_top: 32,
    gas_limit: 10000,
    selected_parts: Array.from(selected)
  };
}

function renderSelection() {
  const byCat = {};
  for (const id of selected) {
    const p = catalog.parts.find(x => x.id === id);
    const cat = p ? p.category : "unknown";
    byCat[cat] = (byCat[cat] || 0) + 1;
  }
  $("selectionSummary").innerHTML =
    `<b>${selected.size}</b> Teile ausgewählt. ` +
    (selected.size < 3 ? `<span style="color:var(--bad)">Mindestens 3 erforderlich.</span>` : `<span style="color:var(--good)">gültig.</span>`) +
    `<br>Kategorien: ${Object.entries(byCat).map(([k,v]) => `${k}:${v}`).join(", ")}`;
}

function renderParts() {
  const container = $("parts");
  container.innerHTML = "";
  const groups = {};
  for (const p of catalog.parts) {
    (groups[p.category] ||= []).push(p);
  }
  for (const cat of Object.keys(groups).sort()) {
    for (const p of groups[cat]) {
      const el = document.createElement("div");
      el.className = "part" + (selected.has(p.id) ? " selected" : "");
      el.innerHTML = `<div class="category">${p.category} / ${p.role || ""}</div>
        <strong>${p.label || p.id}</strong>
        <small>${p.description || ""}</small>`;
      el.onclick = () => {
        if (selected.has(p.id)) selected.delete(p.id); else selected.add(p.id);
        renderParts(); renderSelection();
      };
      container.appendChild(el);
    }
  }
  renderSelection();
}

async function refreshJob() {
  if (!currentJob) return;
  const status = await api(`/api/job/${currentJob}`);
  $("status").textContent = JSON.stringify(status, null, 2);

  const artifacts = await api(`/api/job/${currentJob}/artifacts`);
  const list = $("artifacts");
  list.innerHTML = "";
  if (artifacts.files) {
    for (const f of artifacts.files) {
      const chip = document.createElement("span");
      chip.className = "file-chip";
      chip.textContent = `${f.name} (${f.size} B)`;
      chip.onclick = async () => {
        const txt = await fetch(`/api/job/${currentJob}/file/${encodeURIComponent(f.name)}`).then(r => r.text());
        $("fileView").textContent = txt;
      };
      list.appendChild(chip);
    }
  }

  const result = await fetch(`/api/job/${currentJob}/result`);
  if (result.ok) $("result").textContent = await result.text();
  const report = await fetch(`/api/job/${currentJob}/report`);
  if (report.ok) $("report").textContent = await report.text();
}

async function start() {
  const manifest = buildManifest();
  if (manifest.selected_parts.length < 3) {
    alert("Bitte mindestens 3 Formel-/Strategie-Teile auswählen.");
    return;
  }
  $("status").textContent = "Starte Experiment...";
  const res = await api("/api/start", {
    method: "POST",
    headers: {"Content-Type": "application/json"},
    body: JSON.stringify(manifest)
  });
  if (res.error) {
    $("status").textContent = JSON.stringify(res, null, 2);
    return;
  }
  currentJob = res.job_id;
  await refreshJob();
}

async function runTests() {
  if (!currentJob) return alert("Kein Job aktiv.");
  $("testOutput").textContent = "Tests laufen...";
  const res = await api(`/api/job/${currentJob}/test`);
  $("testOutput").textContent = JSON.stringify(res, null, 2);
}



function fmt(v, digits=4) {
  if (typeof v !== "number" || !Number.isFinite(v)) return String(v);
  return Math.abs(v) >= 1000 ? v.toExponential(3) : v.toFixed(digits);
}

function drawLine(canvasId, values, lo=-1, hi=1) {
  const canvas = $(canvasId); if (!canvas) return;
  const ctx = canvas.getContext("2d");
  const w = canvas.width, h = canvas.height;
  ctx.clearRect(0,0,w,h);
  ctx.fillStyle = "#070b15"; ctx.fillRect(0,0,w,h);
  ctx.strokeStyle = "#29324d"; ctx.lineWidth = 1;
  for (let i=0;i<=6;i++) { const y=i*h/6; ctx.beginPath(); ctx.moveTo(0,y); ctx.lineTo(w,y); ctx.stroke(); }
  if (!values || values.length < 2) return;
  ctx.strokeStyle = "#8bd3ff"; ctx.lineWidth = 2;
  ctx.beginPath();
  values.forEach((v,i) => {
    const x = i * w / Math.max(1, values.length-1);
    const y = h - ((Math.max(lo, Math.min(hi, v)) - lo) / (hi-lo)) * h;
    if (i===0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
  });
  ctx.stroke();
}

function drawHist(canvasId, counts) {
  const canvas = $(canvasId); if (!canvas) return;
  const ctx = canvas.getContext("2d");
  const w = canvas.width, h = canvas.height;
  ctx.clearRect(0,0,w,h);
  ctx.fillStyle = "#070b15"; ctx.fillRect(0,0,w,h);
  if (!counts || !counts.length) return;
  const maxv = Math.max(1, ...counts);
  const bw = w / counts.length;
  ctx.fillStyle = "#b28cff";
  counts.forEach((c,i) => {
    const bh = (c/maxv) * (h-20);
    ctx.fillRect(i*bw, h-bh, Math.max(1,bw-1), bh);
  });
}

function drawReturnMap(canvasId, pairs) {
  const canvas = $(canvasId); if (!canvas) return;
  const ctx = canvas.getContext("2d");
  const w = canvas.width, h = canvas.height;
  ctx.clearRect(0,0,w,h);
  ctx.fillStyle = "#070b15"; ctx.fillRect(0,0,w,h);
  ctx.strokeStyle = "#29324d";
  for (let i=0;i<=4;i++) {
    const p=i/4; ctx.beginPath(); ctx.moveTo(p*w,0); ctx.lineTo(p*w,h); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(0,p*h); ctx.lineTo(w,p*h); ctx.stroke();
  }
  if (!pairs || !pairs.length) return;
  ctx.fillStyle = "#7af0b1";
  for (const [a,b] of pairs) {
    const x = ((Math.max(-1, Math.min(1, a)) + 1) / 2) * w;
    const y = h - ((Math.max(-1, Math.min(1, b)) + 1) / 2) * h;
    ctx.fillRect(x, y, 2, 2);
  }
}

function renderDomainEval(data) {
  $("domainEval").textContent = JSON.stringify(data, null, 2);
  if (data.error || data.available === false) {
    $("domainCards").innerHTML = `<div class="metric badCard">${data.error || data.message}</div>`;
    return;
  }
  if (data.generated) {
    const g = data.generated;
    $("domainCards").innerHTML = `
      <div class="metric"><span>Research Score</span><b>${fmt(data.research_score)}</b></div>
      <div class="metric"><span>Lyapunov Band</span><b>${g.lyapunov_band}</b></div>
      <div class="metric"><span>Lyapunov-like</span><b>${fmt(g.lyapunov_like)}</b></div>
      <div class="metric"><span>Orbit Entropy</span><b>${fmt(g.orbit_entropy)}</b></div>
      <div class="metric"><span>Return Diversity</span><b>${fmt(g.return_map_diversity)}</b></div>
      <div class="metric"><span>Attractor Spread</span><b>${fmt(g.attractor_spread)}</b></div>
      <div class="metric"><span>Period</span><b>${g.periodicity.best_period}</b></div>
      <div class="metric"><span>Periodicity Score</span><b>${fmt(g.periodicity.periodicity_score)}</b></div>
      <div class="metric"><span>Finite / Bounded</span><b>${fmt(g.finite_ratio)} / ${g.bounded ? "yes" : "no"}</b></div>
    `;
    drawLine("orbitChart", g.orbit_sample || []);
    drawHist("histChart", g.histogram || []);
    drawReturnMap("returnMapChart", g.return_map_sample || []);

    const rows = [g, ...(data.baselines || [])].map(m => `
      <tr>
        <td>${m.name}</td>
        <td>${fmt(m.lyapunov_like)}</td>
        <td>${m.lyapunov_band}</td>
        <td>${fmt(m.orbit_entropy)}</td>
        <td>${fmt(m.return_map_diversity)}</td>
        <td>${fmt(m.attractor_spread)}</td>
        <td>${m.periodicity ? m.periodicity.best_period : "-"}</td>
        <td>${m.periodicity ? fmt(m.periodicity.periodicity_score) : "-"}</td>
      </tr>`).join("");
    $("baselineTable").innerHTML = `<table>
      <thead><tr><th>System</th><th>Lyap</th><th>Band</th><th>Entropy</th><th>Return</th><th>Spread</th><th>Period</th><th>Periodicity</th></tr></thead>
      <tbody>${rows}</tbody></table>`;
    return;
  }

  const omit = new Set(["available", "algorithm_file"]);
  $("domainCards").innerHTML = Object.entries(data)
    .filter(([k]) => !omit.has(k))
    .slice(0, 12)
    .map(([k, v]) => `<div class="metric"><span>${k}</span><b>${typeof v === "object" ? JSON.stringify(v) : v}</b></div>`)
    .join("");
  $("baselineTable").innerHTML = `<table><thead><tr><th>Metric</th><th>Value</th></tr></thead><tbody>${
    Object.entries(data).map(([k, v]) => `<tr><td>${k}</td><td>${typeof v === "object" ? JSON.stringify(v) : v}</td></tr>`).join("")
  }</tbody></table>`;
  drawLine("orbitChart", []);
  drawHist("histChart", []);
  drawReturnMap("returnMapChart", []);
}

async function runDomainEval() {
  if (!currentJob) return alert("Kein Job aktiv.");
  $("domainEval").textContent = "Domain-Evaluation läuft...";
  const res = await api(`/api/job/${currentJob}/domain-eval`);
  renderDomainEval(res);
}

async function init() {
  catalog = await api("/api/catalog");
  renderParts();
  $("start").onclick = start;
  $("refresh").onclick = refreshJob;
  $("runTests").onclick = runTests;
  $("runDomainEval").onclick = runDomainEval;
  setInterval(refreshJob, 4000);
}

init().catch(e => { $("status").textContent = e.stack || String(e); });
