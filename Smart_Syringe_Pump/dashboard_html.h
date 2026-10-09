#pragma once

// Dashboard HTML. Kept in a .h file so the Arduino IDE does not parse the JavaScript as C++.
const char DASHBOARD_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Syringe Pump</title>
<style>
:root{--bg:#091018;--panel:#111c26;--panel2:#172431;--line:#2a3a49;--text:#eaf2f8;--muted:#91a4b6;--good:#42d392;--warn:#ffca5c;--bad:#ff6678;--accent:#59a7ff}
*{box-sizing:border-box}body{margin:0;background:linear-gradient(135deg,#071018,#0c1721 55%,#101c27);color:var(--text);font-family:Inter,Segoe UI,Arial,sans-serif}
.wrap{max-width:1280px;margin:auto;padding:18px}.top{display:flex;justify-content:space-between;gap:16px;align-items:center;margin-bottom:12px}.brand h1{font-size:24px;margin:0}.brand p{margin:5px 0 0;color:var(--muted);font-size:12px}.pill{padding:7px 11px;border:1px solid var(--line);border-radius:999px;background:var(--panel);font-size:12px}
.nav{display:flex;gap:7px;overflow:auto;padding:8px 0 14px}.nav button{white-space:nowrap;border:1px solid var(--line);border-radius:9px;background:#152431;color:var(--text);padding:9px 13px;cursor:pointer}.nav button.active{background:#174b78;border-color:#276a9e}
.page{display:none}.page.active{display:block}.grid{display:grid;grid-template-columns:1.1fr .9fr;gap:15px}.panel{background:rgba(17,28,38,.96);border:1px solid var(--line);border-radius:14px;padding:15px;box-shadow:0 10px 28px rgba(0,0,0,.18);margin-bottom:15px}.panel h2{font-size:15px;margin:0 0 13px}.panel h3{font-size:13px;margin:16px 0 9px;color:#dce7ef}
.status{display:grid;grid-template-columns:repeat(4,1fr);gap:9px}.metric{background:var(--panel2);border:1px solid var(--line);border-radius:10px;padding:11px}.metric .label{font-size:10px;color:var(--muted);text-transform:uppercase}.metric .value{font-size:21px;font-weight:700;margin-top:5px}.state{display:inline-flex;padding:6px 10px;border-radius:999px;font-size:12px;font-weight:700;text-transform:uppercase;background:#243241}.state.good{background:rgba(66,211,146,.14);color:var(--good)}.state.warn{background:rgba(255,202,92,.14);color:var(--warn)}.state.bad{background:rgba(255,102,120,.14);color:var(--bad)}
.alertBanner{display:none;margin:0 0 12px;padding:12px 14px;border:1px solid #71303b;border-radius:11px;background:rgba(255,102,120,.10)}.alertBanner.show{display:block}.alertBanner .title{font-weight:800;color:var(--bad);font-size:13px}.alertBanner .msg{font-size:12px;margin-top:4px}.safetyGrid{display:grid;grid-template-columns:repeat(3,1fr);gap:9px}.safetyCard{background:var(--panel2);border:1px solid var(--line);border-radius:10px;padding:11px}.safetyCard .name{font-size:10px;color:var(--muted);text-transform:uppercase}.safetyCard .status{display:block;margin-top:5px;font-size:14px;font-weight:800}.safetyCard.ok .status{color:var(--good)}.safetyCard.warn .status{color:var(--warn)}.safetyCard.bad .status{color:var(--bad)}.syringe{position:relative;height:240px;width:120px;margin:12px auto 8px;border:3px solid #7890a2;border-radius:9px;background:#0a1118;overflow:hidden}.syringeFill{position:absolute;left:0;right:0;bottom:0;height:0;background:linear-gradient(180deg,#59a7ff,#174b78);transition:height .25s}.syringePlunger{position:absolute;left:-15px;right:-15px;height:8px;background:#aab8c4;bottom:0;transition:bottom .25s}.syringeScale{display:flex;justify-content:space-between;gap:8px;font-size:11px;color:var(--muted)}.heroMetric{text-align:center;background:var(--panel2);border:1px solid var(--line);border-radius:11px;padding:14px}.heroMetric .big{font-size:30px;font-weight:800}.heroMetric .sub{font-size:11px;color:var(--muted);margin-top:3px}.countdown{font-size:24px;font-weight:800}.criticalNote{border-color:#71303b;background:rgba(255,102,120,.08);color:#ffb0ba}.mutedNote{border-color:var(--line);background:rgba(145,164,182,.06);color:var(--muted)}
.progress{height:13px;background:#0a1219;border:1px solid var(--line);border-radius:999px;overflow:hidden;margin:11px 0}.bar{height:100%;width:0%;background:var(--accent);transition:width .25s}
.controls{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}.controls button,.btn{border:1px solid var(--line);border-radius:8px;background:#1a2936;color:var(--text);padding:10px;font-weight:700;cursor:pointer}.controls button:hover,.btn:hover{filter:brightness(1.12)}.primary{background:#174b78!important}.danger{background:#5b1e29!important;border-color:#803140!important}.warn{background:#5b4720!important}.controls button:disabled,.btn:disabled{opacity:.4;cursor:not-allowed}
.form{display:grid;grid-template-columns:1fr 1fr;gap:10px}.field label{display:block;color:var(--muted);font-size:11px;margin-bottom:5px}.field input,.field select{width:100%;padding:9px;border-radius:8px;border:1px solid var(--line);background:#0c151d;color:var(--text);font-size:14px}
.kv{display:grid;grid-template-columns:1fr 1fr;gap:7px}.kv div{display:flex;justify-content:space-between;gap:10px;padding:7px 0;border-bottom:1px solid #1e2b37;font-size:12px}.kv span:first-child{color:var(--muted)}
.notice{margin-top:12px;padding:10px;border:1px solid #5d4b24;background:rgba(255,202,92,.08);color:#e9d59a;border-radius:9px;font-size:11px}.success{border-color:#245e48;background:rgba(66,211,146,.08);color:#9be5c3}.dangerBox{border-color:#71303b;background:rgba(255,102,120,.08);color:#ffb0ba}
.log{min-height:65px;padding:9px;background:#0a1118;border:1px solid var(--line);border-radius:8px;font-family:Consolas,monospace;font-size:11px;white-space:pre-wrap}.tableWrap{overflow:auto;border:1px solid var(--line);border-radius:9px}.table{width:100%;border-collapse:collapse;font-size:11px;min-width:720px}.table th,.table td{padding:8px;border-bottom:1px solid var(--line);text-align:left;white-space:nowrap}.table th{color:var(--muted);font-weight:600;background:#0d1821}.table tr:last-child td{border-bottom:0}.tag{display:inline-block;padding:3px 7px;border-radius:999px;background:#233342}.tag.good{color:var(--good);background:rgba(66,211,146,.1)}.tag.bad{color:var(--bad);background:rgba(255,102,120,.1)}.tag.warn{color:var(--warn);background:rgba(255,202,92,.1)}
.actions{display:flex;gap:8px;flex-wrap:wrap}.footer{margin-top:5px;color:var(--muted);font-size:10px;text-align:center}
.small{font-size:11px;color:var(--muted)}.resultGrid{display:grid;grid-template-columns:repeat(4,1fr);gap:9px}.result{background:var(--panel2);padding:10px;border-radius:9px;border:1px solid var(--line)}.result b{display:block;font-size:18px;margin-top:4px}

/* Progressive / responsive layout: desktop, tablet, phone and touch devices */
html{font-size:16px}body{min-width:320px;-webkit-text-size-adjust:100%;overflow-x:hidden}.wrap{width:100%}.nav{scrollbar-width:thin;-webkit-overflow-scrolling:touch}.nav button{min-height:42px;touch-action:manipulation}.controls button,.btn{min-height:44px;touch-action:manipulation}.field input,.field select{min-height:44px}button{font:inherit}.graphGrid{display:grid;grid-template-columns:1fr 1fr;gap:12px}.graphCard{min-width:0}.graphCanvas{display:block;width:100%;height:auto;max-width:100%;background:#091018;border:1px solid var(--line);border-radius:9px}.tableWrap{-webkit-overflow-scrolling:touch}.table th,.table td{padding:9px}
@media (min-width:1400px){.wrap{max-width:1440px}.panel{padding:18px}.metric .value{font-size:23px}}
@media (max-width:1100px){.grid{grid-template-columns:1fr}.graphGrid{grid-template-columns:1fr 1fr}}
@media (max-width:760px){.wrap{padding:13px}.top{align-items:flex-start;flex-direction:column}.brand h1{font-size:21px}.brand p{line-height:1.4}.pill{align-self:flex-start}.nav{gap:6px;padding-top:6px}.nav button{padding:9px 11px}.status{grid-template-columns:repeat(2,minmax(0,1fr))}.resultGrid{grid-template-columns:repeat(2,minmax(0,1fr))}.form{grid-template-columns:1fr}.controls{grid-template-columns:repeat(2,minmax(0,1fr))}.kv{grid-template-columns:1fr}.graphGrid{grid-template-columns:1fr}.graphCanvas{min-height:180px}.actions .btn{flex:1 1 180px}}
@media (max-width:480px){.wrap{padding:9px}.panel{padding:12px;border-radius:11px}.panel h2{font-size:14px}.status{gap:7px}.metric{padding:9px}.metric .value{font-size:18px}.controls{grid-template-columns:1fr 1fr}.controls button,.btn{padding:10px 8px}.resultGrid{grid-template-columns:1fr 1fr}.actions{gap:6px}.actions .btn{flex:1 1 100%}.table{min-width:680px;font-size:10px}.footer{line-height:1.5}}
@media (orientation:landscape) and (max-height:600px){.wrap{padding:9px}.top{margin-bottom:6px}.nav{padding:4px 0 8px}.panel{padding:11px;margin-bottom:10px}}
@media (prefers-reduced-motion:reduce){*{scroll-behavior:auto!important;transition:none!important}}
canvas{max-width:100%}.graphCanvas{touch-action:none;cursor:grab;user-select:none}.graphCanvas.dragging{cursor:grabbing}.auditWrap{max-height:240px;overflow:auto;border:1px solid var(--line);border-radius:9px;-webkit-overflow-scrolling:touch}.auditTable{width:100%;border-collapse:collapse;font-size:11px;line-height:1.3}.auditTable th,.auditTable td{padding:4px 8px;border-bottom:1px solid var(--line);text-align:left;white-space:nowrap}.auditTable thead th{position:sticky;top:0;z-index:1;background:#152431;font-size:11px}.auditTable tr:last-child td{border-bottom:0}</style>
</head>
<body>
<div class="wrap">
  <div class="top">
    <div class="brand"><h1>Smart Syringe Pump</h1><p>ESP32 local dashboard | <b>http://smartsyringe.local</b></p></div>
    <div style="display:flex;gap:8px;align-items:center"><div class="pill" id="net">Connecting...</div><button class="btn" onclick="logout()">LOG OUT</button></div>
  </div>

  <div id="alertBanner" class="alertBanner"><div class="title" id="alertTitle">PUMP ALARM</div><div class="msg" id="alertMessage">-</div><div class="actions" style="margin-top:9px"><button class="btn danger" onclick="cmd('/api/stop')">STOP PUMP</button><button class="btn" onclick="showPage('alarms')">VIEW ALARM HISTORY</button><button class="btn" onclick="snoozeAlert()">HIDE FOR 5 MIN</button></div></div>

  <div class="nav">
    <button data-page="dashboard" onclick="showPage('dashboard')">Dashboard</button>
    <button data-page="infusion" onclick="showPage('infusion')">Current Infusion</button>
    <button data-page="safety" onclick="showPage('safety')">Safety</button>
    <button data-page="setup" onclick="showPage('setup')">Calibration</button>
    <button data-page="calwizard" onclick="showPage('calwizard')">Calibration Wizard</button>
    <button data-page="gravimetric" onclick="showPage('gravimetric')">Gravimetric Test</button>
    <button data-page="history" onclick="showPage('history')">Infusion History</button>
    <button data-page="alarms" onclick="showPage('alarms')">Alarm History</button>
    <button data-page="diagnostics" onclick="showPage('diagnostics')">Diagnostics</button>
     <button data-page="graphs" onclick="showPage('graphs')">Graphs</button>
    <button data-page="validation" onclick="showPage('validation')">Validation</button>
    <button data-page="stability" onclick="showPage('stability')">Stability Test</button>
    <button data-page="audit" onclick="showPage('audit')">Audit Log</button>
  </div>

  <section id="page-dashboard" class="page">
    <div class="grid">
      <section class="panel">
        <h2>Run Status</h2>
        <div style="margin-bottom:11px"><span id="state" class="state">UNKNOWN</span> <span id="homed" class="pill">Homed: -</span></div>
        <div class="status">
          <div class="metric"><div class="label">Target</div><div class="value"><span id="target">0.000</span> mL</div></div>
          <div class="metric"><div class="label">Delivered</div><div class="value"><span id="delivered">0.000</span> mL</div></div>
          <div class="metric"><div class="label">Flow rate</div><div class="value"><span id="rate">0.0</span> mL/h</div></div>
          <div class="metric"><div class="label">Position</div><div class="value"><span id="pos">0</span></div></div>
        </div>
        <div class="progress"><div class="bar" id="bar"></div></div>
        <div class="kv">
          <div><span>Motor current ADC</span><span id="current">-</span></div>
          <div><span>Current delta</span><span id="currentDelta">-</span></div>
          <div><span>Load cell</span><span id="load">-</span></div>
          <div><span>HOME switch</span><span id="home">-</span></div>
          <div><span>MAX switch</span><span id="max">-</span></div>
          <div><span>Occlusion baseline</span><span id="baselineLock">-</span></div>
        </div>
      </section>

      <section class="panel">
        <h2>Infusion Command</h2>
        <div class="form">
          <div class="field"><label for="volume">Volume (mL)</label><input id="volume" type="number" min="0.001" max="60" step="0.001" value="10"></div>
          <div class="field"><label for="rateIn">Rate (mL/hr)</label><input id="rateIn" type="number" min="0.001" max="300" step="0.1" value="60"></div>
        </div>
        <div style="height:9px"></div>
        <div class="controls">
          <button class="primary" id="startBtn" onclick="startPump()">START</button>
          <button class="danger" onclick="cmd('/api/stop')">STOP</button>
          <button class="warn" id="pauseBtn" onclick="cmd('/api/pause')">PAUSE</button>
          <button class="primary" id="resumeBtn" onclick="cmd('/api/resume')">RESUME</button>
          <button onclick="cmd('/api/home')">HOME</button>
          <button onclick="cmd('/api/tare')">TARE</button>
        </div>
      </section>
    </div>

    <div class="grid">
      <section class="panel">
        <h2>Summary</h2>
        <div class="kv">
          <div><span>Device</span><span id="device">sp01</span></div>
          <div><span>Wi-Fi RSSI</span><span id="rssi">-</span></div>
          <div><span>MQTT</span><span id="mqtt">-</span></div>
          <div><span>Acceleration</span><span id="accel">-</span></div>
          <div><span>Steps/mL</span><span id="spm">-</span></div>
          <div><span>Max travel</span><span id="maxSteps">-</span></div>
        </div>
      </section>
      <section class="panel">
        <h2>Latest Event</h2>
        <div class="log" id="event">Waiting for event...</div>
      </section>
    </div>
  </section>


  <section id="page-validation" class="page">
    <section class="panel">
      <h2>Validation</h2>
      <p class="small">Gravimetric accuracy and occlusion response tests.</p>
    </section>

    <div class="grid">
      <section class="panel">
        <h2>Automated Gravimetric Validation</h2>
        <div class="form">
          <div class="field"><label>Target volume (mL)</label><input id="valTarget" type="number" min="0.1" max="60" step="0.001" value="10"></div>
          <div class="field"><label>Commanded flow (mL/hr)</label><input id="valRate" type="number" min="0.01" max="300" step="0.01" value="5"></div>
          <div class="field"><label>Fluid density (g/mL)</label><input id="valDensity" type="number" min="0.001" step="0.0001" value="1.0000"></div>
          <div class="field"><label>Acceptance error +/- (%)</label><input id="valTolerance" type="number" min="0.01" step="0.01" value="2.00"></div>
        </div>
        <div class="actions" style="margin-top:11px"><button class="btn primary" id="valStart" onclick="startValidationGravimetric()">START VALIDATION TEST</button><button class="btn" onclick="cmd('/api/tare')">TARE HX711</button></div>
        <div class="kv" style="margin-top:11px">
          <div><span>Test state</span><span id="valRunState">IDLE</span></div>
          <div><span>Measured volume</span><span id="valMeasured">-</span></div>
          <div><span>Volume error</span><span id="valError">-</span></div>
          <div><span>Measured flow</span><span id="valMeasuredFlow">-</span></div>
          <div><span>Elapsed time</span><span id="valElapsed">-</span></div>
          <div><span>Result</span><span id="valResult">-</span></div>
        </div>
      </section>

      <section class="panel">
        <h2>Live Validation Status</h2>
        <div class="status">
          <div class="heroMetric"><div class="sub">TARGET</div><div class="big"><span id="valLiveTarget">0.000</span> mL</div></div>
          <div class="heroMetric"><div class="sub">DELIVERED</div><div class="big"><span id="valLiveDelivered">0.000</span> mL</div></div>
          <div class="heroMetric"><div class="sub">LOAD</div><div class="big"><span id="valLiveLoad">0.00</span> g</div></div>
          <div class="heroMetric"><div class="sub">CURRENT Δ</div><div class="big"><span id="valLiveDelta">0.0</span></div></div>
        </div>
        <div class="progress" style="margin-top:12px"><div class="bar" id="valBar"></div></div>
        <div class="small" style="margin-top:9px">Flow = measured volume / elapsed time.</div>
      </section>
    </div>

    <div class="grid">
      <section class="panel">
        <h2>Occlusion Response Test</h2>
        <p class="small">Arm the timer, then induce the occlusion. Time is measured from the first OCCLUDED state seen by the browser.</p>
        <div class="actions"><button class="btn warn" id="occArm" onclick="armOcclusionTest()">ARM OCCLUSION TIMER</button><button class="btn" onclick="disarmOcclusionTest()">DISARM</button></div>
        <div class="kv" style="margin-top:10px">
          <div><span>Timer</span><span id="occTimer">NOT ARMED</span></div>
          <div><span>Observed detection time</span><span id="occDetection">-</span></div>
          <div><span>Peak current delta</span><span id="occPeak">-</span></div>
          <div><span>Firmware state</span><span id="occState">-</span></div>
          <div><span>Result</span><span id="occResult">-</span></div>
        </div>
      </section>

      <section class="panel">
        <h2>Acceptance Criteria</h2>
        <div class="kv">
          <div><span>Volume accuracy</span><span id="critVolume">+/-2.00%</span></div>
          <div><span>Flow accuracy</span><span>Calculated from gravimetric result</span></div>
          <div><span>Occlusion response</span><span>Observed firmware alarm transition</span></div>
          <div><span>Limit protection</span><span>HOME / MAX state observation</span></div>
        </div>
      </section>
    </div>

    <section class="panel">
      <div class="actions" style="justify-content:space-between;align-items:center"><h2 style="margin:0">Validation Session Records</h2><div><button class="btn" onclick="exportValidationCSV()">EXPORT CSV</button><button class="btn" onclick="exportValidationReport()">EXPORT HTML REPORT</button><button class="btn danger" onclick="clearValidationRecords()">CLEAR RECORDS</button></div></div>
      <div class="tableWrap" style="margin-top:10px"><table><thead><tr><th>ID</th><th>Test</th><th>Target</th><th>Measured</th><th>Error</th><th>Flow</th><th>Duration</th><th>Result</th></tr></thead><tbody id="validationRows"><tr><td colspan="8">No validation records.</td></tr></tbody></table></div>
    </section>
  </section>


  <section id="page-stability" class="page">
    <section class="panel">
      <h2>Stability Test</h2>
      <p class="small">Repeats the gravimetric test several times and reports mean, standard deviation and CV.</p>
    </section>
    <div class="grid">
      <section class="panel">
        <h2>Test Configuration</h2>
        <div class="form">
          <div class="field"><label>Number of runs</label><input id="stabRuns" type="number" min="2" max="20" step="1" value="5"></div>
          <div class="field"><label>Target volume / run (mL)</label><input id="stabTarget" type="number" min="0.1" max="60" step="0.001" value="5"></div>
          <div class="field"><label>Commanded flow (mL/hr)</label><input id="stabRate" type="number" min="0.01" max="300" step="0.01" value="60"></div>
          <div class="field"><label>Acceptance error +/- (%)</label><input id="stabTol" type="number" min="0.01" step="0.01" value="2.00"></div>
          <div class="field"><label>Maximum CV (%)</label><input id="stabCvTol" type="number" min="0.01" step="0.01" value="1.00"></div>
          <div class="field"><label>Fluid density (g/mL)</label><input id="stabDensity" type="number" min="0.001" step="0.0001" value="1.0000"></div>
        </div>
        <div class="actions" style="margin-top:11px"><button class="btn primary" id="stabStart" onclick="startStabilityTest()">START STABILITY TEST</button><button class="btn danger" id="stabStop" onclick="stopStabilityTest()">STOP / ABORT</button></div>
        <div class="notice mutedNote">Home the pump and check the load cell before starting.</div>
      </section>
      <section class="panel">
        <h2>Live Test Status</h2>
        <div class="status">
          <div class="heroMetric"><div class="sub">RUN</div><div class="big"><span id="stabRunNo">0 / 0</span></div></div>
          <div class="heroMetric"><div class="sub">STATE</div><div class="big" style="font-size:20px"><span id="stabState">IDLE</span></div></div>
          <div class="heroMetric"><div class="sub">MEASURED</div><div class="big"><span id="stabMeasured">-</span> mL</div></div>
          <div class="heroMetric"><div class="sub">ERROR</div><div class="big"><span id="stabError">-</span>%</div></div>
        </div>
        <div class="kv" style="margin-top:12px">
          <div><span>Mean measured volume</span><span id="stabMean">-</span></div>
          <div><span>Standard deviation</span><span id="stabStd">-</span></div>
          <div><span>Coefficient of variation</span><span id="stabCv">-</span></div>
          <div><span>Maximum absolute error</span><span id="stabMaxErr">-</span></div>
          <div><span>Acceptance</span><span id="stabResult">-</span></div>
        </div>
      </section>
    </div>
    <section class="panel">
      <h2>Stability Run Records</h2>
      <div class="tableWrap"><table><thead><tr><th>Run</th><th>Target</th><th>Measured</th><th>Error</th><th>Flow</th><th>Duration</th><th>Result</th></tr></thead><tbody id="stabRows"><tr><td colspan="7">No stability runs.</td></tr></tbody></table></div>
      <div class="actions" style="margin-top:10px"><button class="btn" onclick="exportStabilityCSV()">EXPORT STABILITY CSV</button><button class="btn" onclick="clearStabilityRuns()">CLEAR RUNS</button></div>
    </section>
  </section>

  <section id="page-audit" class="page">
    <section class="panel">
      <h2>Audit Log</h2>
      <p class="small">Operator actions are stored in this browser only.</p>
      <div class="actions" style="margin-top:11px"><button class="btn" onclick="exportAuditCSV()">EXPORT AUDIT CSV</button><button class="btn danger" onclick="clearAuditLog()">CLEAR AUDIT LOG</button></div>
    </section>
    <section class="panel">
      <div class="auditWrap"><table class="auditTable"><thead><tr><th>Timestamp</th><th>Action</th><th>Path / Detail</th><th>Device State</th></tr></thead><tbody id="auditRows"><tr><td colspan="4">No audit events.</td></tr></tbody></table></div>
    </section>
  </section>

  <section id="page-graphs" class="page">
    <section class="panel">
      <h2>Graphs</h2>
      <div class="small">Samples are taken about once per second. Download the data as CSV or each graph as PNG.</div>
      <div class="notice" style="margin-top:10px"><b>Interactive graph controls:</b> Mouse wheel / pinch = zoom | Drag = pan | Double-click = reset. Each graph can be zoomed independently.</div>
      <div class="actions" style="margin-top:10px">
        <button class="btn primary" onclick="downloadGraphCSV()">DOWNLOAD GRAPH DATA (CSV)</button>
        <button class="btn" onclick="downloadAllGraphsPNG()">DOWNLOAD ALL GRAPHS (PNG)</button>
        <button class="btn danger" onclick="clearGraphData()">CLEAR GRAPH DATA</button>
      </div>
    </section>
    <div class="grid">
      <section class="panel"><h2>Flow Rate - Commanded</h2><canvas id="flowCanvas" width="760" height="300" class="graphCanvas" width="760" height="300" style="width:100%;height:auto;background:#0a1118;border:1px solid var(--line);border-radius:9px"></canvas><div class="actions" style="margin-top:8px"><button class="btn" onclick="resetGraph('flowCanvas')">RESET VIEW</button><button class="btn" onclick="downloadCanvas('flowCanvas','flow_rate.png')">DOWNLOAD PNG</button></div></section>
      <section class="panel"><h2>Delivered Volume - Target vs Actual</h2><canvas id="volumeCanvas" width="760" height="300" class="graphCanvas" width="760" height="300" style="width:100%;height:auto;background:#0a1118;border:1px solid var(--line);border-radius:9px"></canvas><div class="actions" style="margin-top:8px"><button class="btn" onclick="resetGraph('volumeCanvas')">RESET VIEW</button><button class="btn" onclick="downloadCanvas('volumeCanvas','delivered_volume.png')">DOWNLOAD PNG</button></div></section>
    </div>
    <div class="grid">
      <section class="panel"><h2>Motor Current ADC</h2><canvas id="currentCanvas" width="760" height="300" class="graphCanvas" width="760" height="300" style="width:100%;height:auto;background:#0a1118;border:1px solid var(--line);border-radius:9px"></canvas><div class="actions" style="margin-top:8px"><button class="btn" onclick="resetGraph('currentCanvas')">RESET VIEW</button><button class="btn" onclick="downloadCanvas('currentCanvas','motor_current.png')">DOWNLOAD PNG</button></div></section>
      <section class="panel"><h2>Load Cell Mass</h2><canvas id="loadCanvas" width="760" height="300" class="graphCanvas" width="760" height="300" style="width:100%;height:auto;background:#0a1118;border:1px solid var(--line);border-radius:9px"></canvas><div class="actions" style="margin-top:8px"><button class="btn" onclick="resetGraph('loadCanvas')">RESET VIEW</button><button class="btn" onclick="downloadCanvas('loadCanvas','load_cell.png')">DOWNLOAD PNG</button></div></section>
    </div>
  </section>

  <section id="page-infusion" class="page">
    <div class="grid">
      <section class="panel">
        <h2>Current Infusion</h2>
        <div style="margin-bottom:11px"><span id="infState" class="state">UNKNOWN</span> <span id="infHomed" class="pill">Homed: -</span></div>
        <div class="status">
          <div class="heroMetric"><div class="sub">TARGET VOLUME</div><div class="big"><span id="infTarget">0.000</span> mL</div></div>
          <div class="heroMetric"><div class="sub">DELIVERED</div><div class="big"><span id="infDelivered">0.000</span> mL</div></div>
          <div class="heroMetric"><div class="sub">REMAINING</div><div class="big"><span id="infRemaining">0.000</span> mL</div></div>
          <div class="heroMetric"><div class="sub">PROGRESS</div><div class="big"><span id="infPercent">0.0</span>%</div></div>
        </div>
        <div class="progress"><div class="bar" id="infBar"></div></div>
        <div class="kv">
          <div><span>Commanded flow</span><span id="infRate">0.0 mL/hr</span></div>
          <div><span>Position</span><span id="infPosition">-</span></div>
          <div><span>Distance to go</span><span id="infDistance">-</span></div>
          <div><span>Motor current</span><span id="infCurrent">-</span></div>
          <div><span>Load cell</span><span id="infLoad">-</span></div>
          <div><span>Estimated remaining</span><span id="infEta">-</span></div>
        </div>
        <div class="actions" style="margin-top:11px"><button class="btn warn" id="infPause" onclick="cmd('/api/pause')">PAUSE</button><button class="btn primary" id="infResume" onclick="cmd('/api/resume')">RESUME</button><button class="btn danger" onclick="cmd('/api/stop')">STOP PUMP</button></div>
      </section>

      <section class="panel">
        <h2>Syringe Position</h2>
        <div class="syringe"><div id="syringeFill" class="syringeFill"></div><div id="syringePlunger" class="syringePlunger"></div></div>
        <div class="syringeScale"><span>FULL / HOME</span><span id="syringePct">0.0%</span><span>EMPTY</span></div>
        <div class="kv" style="margin-top:12px">
          <div><span>Current position</span><span id="syringePos">0</span></div>
          <div><span>Full position</span><span>0</span></div>
          <div><span>Empty position</span><span id="syringeMax">12463</span></div>
          <div><span>Available capacity</span><span id="syringeAvail">60.000 mL</span></div>
        </div>
      </section>
    </div>
  </section>

  <section id="page-safety" class="page">
    <section class="panel">
      <h2>Safety</h2>
      <p class="small">Live status from the firmware. Red means the operator must act.</p>
      <div class="safetyGrid">
        <div id="safeState" class="safetyCard ok"><div class="name">Pump state</div><div class="status">NORMAL</div></div>
        <div id="safeHome" class="safetyCard ok"><div class="name">HOME switch</div><div class="status">OPEN</div></div>
        <div id="safeMax" class="safetyCard ok"><div class="name">MAX limit</div><div class="status">OPEN</div></div>
        <div id="safeOcc" class="safetyCard ok"><div class="name">Occlusion monitor</div><div class="status">STANDBY</div></div>
        <div id="safeLoad" class="safetyCard ok"><div class="name">Load cell</div><div class="status">READY</div></div>
        <div id="safeWifi" class="safetyCard ok"><div class="name">Wi-Fi</div><div class="status">CONNECTED</div></div>
        <div id="safeMqtt" class="safetyCard ok"><div class="name">MQTT</div><div class="status">CONNECTED</div></div>
        <div id="safeHomeValid" class="safetyCard ok"><div class="name">Position validity</div><div class="status">HOMED</div></div>
        <div class="safetyCard warn"><div class="name">Physical E-stop</div><div class="status">NOT EXPOSED</div></div>
      </div>
      <div class="actions" style="margin-top:12px"><button class="btn danger" onclick="cmd('/api/stop')">STOP PUMP</button><button class="btn" onclick="cmd('/api/home')">HOME AXIS</button><button class="btn" onclick="loadAlarms();showPage('alarms')">VIEW ALARM HISTORY</button></div>
      <div id="safetyMessage" class="notice mutedNote" style="margin-top:11px">No active alarm.</div>
    </section>
    <section class="panel">
      <h2>Current Alarm / Fault</h2>
      <div class="log" id="safetyAlarm">No active alarm or fault message.</div>
      <div class="kv" style="margin-top:9px">
        <div><span>Current state</span><span id="safeCurrentState">-</span></div>
        <div><span>Current delta</span><span id="safeDelta">-</span></div>
        <div><span>Occlusion threshold</span><span id="safeThreshold">-</span></div>
        <div><span>Baseline locked</span><span id="safeBaseline">-</span></div>
        <div><span>Last event</span><span id="safeLastEvent">-</span></div>
        <div><span>Alarm records</span><span id="safeAlarmCount">0</span></div>
      </div>
    </section>
  </section>

  <section id="page-setup" class="page">
    <div class="grid">
      <section class="panel">
        <h2>Syringe Calibration</h2>
        <div class="form">
          <div class="field"><label>Measured steps / mL</label><input id="cfgSpm" type="number" min="50" max="1000" step="0.0001"></div>
          <div class="field"><label>HX711 scale factor</label><input id="cfgScale" type="number" step="0.000001"></div>
          <div class="field"><label>Fluid density (g/mL)</label><input id="cfgDensity" type="number" min="0.1" max="2" step="0.0001"></div>
          <div class="field"><label>Infusion acceleration (read-only)</label><input id="cfgAccel" disabled></div>
        </div>
        <div style="height:9px"></div>
        <div class="actions"><button class="btn primary" onclick="saveConfig()">SAVE CONFIGURATION</button><button class="btn" onclick="cmd('/api/tare')">TARE HX711</button><button class="btn" onclick="cmd('/api/home')">HOME AXIS</button></div>
        <div class="notice">Calibration is saved to flash. It is only accepted when the pump is idle, homed and at the HOME position.</div>
      </section>

      <section class="panel">
        <h2>Calibration Reference</h2>
        <div class="kv">
          <div><span>Full position</span><span id="fullPos">0 steps</span></div>
          <div><span>Empty position</span><span id="emptyPos">12463 steps</span></div>
          <div><span>Maximum volume</span><span>60.000 mL</span></div>
          <div><span>Current steps/mL</span><span id="currentSpm">-</span></div>
          <div><span>1 mL</span><span id="oneMlSteps">-</span></div>
          <div><span>60 mL</span><span id="sixtyMlSteps">-</span></div>
        </div>
        <div class="notice success">Default: 12,463 steps for 60 mL (about 207.7167 steps/mL). Check it with a gravimetric test.</div>
      </section>
    </div>
  </section>

  <section id="page-calwizard" class="page">
    <section class="panel">
      <h2>Calibration Wizard</h2>
      <p class="small">Runs a gravimetric test and calculates a corrected steps/mL value.</p>
      <div class="notice">Use a known fluid, put a vessel on the load cell, and make sure the pump is idle and at HOME.</div>
    </section>
    <div class="grid">
      <section class="panel">
        <h2>Step 1 - Review Current Calibration</h2>
        <div class="kv">
          <div><span>Current steps/mL</span><span id="wizCurrentSpm">-</span></div>
          <div><span>Current HX711 scale</span><span id="wizCurrentScale">-</span></div>
          <div><span>Current density</span><span id="wizCurrentDensity">-</span></div>
          <div><span>Pump position</span><span id="wizPosition">-</span></div>
          <div><span>Homed</span><span id="wizHomed">-</span></div>
        </div>
        <div class="actions" style="margin-top:10px"><button class="btn" onclick="loadCalibrationWizard()">REFRESH CALIBRATION</button><button class="btn" onclick="cmd('/api/home')">HOME AXIS</button></div>
      </section>
      <section class="panel">
        <h2>Step 2 - Gravimetric Calibration Run</h2>
        <div class="form">
          <div class="field"><label>Known target volume (mL)</label><input id="wizTarget" type="number" min="0.1" max="60" step="0.001" value="10.000"></div>
          <div class="field"><label>Commanded flow (mL/hr)</label><input id="wizRate" type="number" min="0.01" max="300" step="0.01" value="60.00"></div>
          <div class="field"><label>Fluid density (g/mL)</label><input id="wizDensity" type="number" min="0.1" max="2" step="0.0001" value="1.0000"></div>
        </div>
        <div class="actions" style="margin-top:10px"><button class="btn primary" id="wizStart" onclick="startCalibrationRun()">RUN CALIBRATION TEST</button><button class="btn" onclick="cmd('/api/tare')">TARE HX711</button></div>
        <div class="kv" style="margin-top:10px"><div><span>Run state</span><span id="wizRunState">IDLE</span></div><div><span>Measured volume</span><span id="wizMeasured">-</span></div><div><span>Measured error</span><span id="wizError">-</span></div></div>
      </section>
    </div>
    <section class="panel">
      <h2>Step 3 - Calculate Correction</h2>
      <p class="small">New steps/mL = current steps/mL x target volume / measured volume.</p>
      <div class="resultGrid">
        <div class="result"><span class="small">Current</span><b id="wizCalcCurrent">-</b></div>
        <div class="result"><span class="small">Target</span><b id="wizCalcTarget">-</b></div>
        <div class="result"><span class="small">Measured</span><b id="wizCalcMeasured">-</b></div>
        <div class="result"><span class="small">Proposed steps/mL</span><b id="wizProposed">-</b></div>
      </div>
      <div class="actions" style="margin-top:11px"><button class="btn primary" id="wizApply" onclick="applyWizardCalibration()" disabled>APPLY PROPOSED STEPS/mL</button><button class="btn" onclick="showPage('setup')">OPEN FULL SETUP</button></div>
      <div id="wizApplyStatus" class="notice mutedNote" style="margin-top:10px">No proposed calibration yet.</div>
    </section>
    <section class="panel">
      <h2>Step 4 - Verification</h2>
      <div class="kv"><div><span>Post-calibration recommendation</span><span>Repeat gravimetric test at a different volume/rate</span></div><div><span>Acceptance criterion</span><span id="wizVerifyTolerance">+/-2.00% (default)</span></div>      <div class="notice mutedNote" style="margin-top:10px">Do not treat a single successful run as proof of accuracy across flow rate, pressure, temperature, viscosity, syringe type, or long-duration operation.</div>
    </section>
  </section>

  <section id="page-gravimetric" class="page">
    <div class="grid">
      <section class="panel">
        <h2>Gravimetric Verification Test</h2>
        <div class="form">
          <div class="field"><label>Test volume (mL)</label><input id="gravVol" type="number" min="0.001" max="60" step="0.001" value="10"></div>
          <div class="field"><label>Rate (mL/hr)</label><input id="gravRate" type="number" min="0.001" max="300" step="0.1" value="60"></div>
          <div class="field"><label>Fluid density (g/mL)</label><input id="gravDensity" type="number" min="0.1" max="2" step="0.0001" value="1.0000"></div>
          <div class="field"><label>HX711 mass before test</label><input id="gravInitial" disabled></div>
        </div>
        <div style="height:9px"></div>
        <div class="actions"><button class="btn primary" id="gravStart" onclick="startGrav()">START GRAVIMETRIC TEST</button><button class="btn" onclick="cmd('/api/tare')">TARE BEFORE TEST</button></div>
      </section>

      <section class="panel">
        <h2>Live Test Result</h2>
        <div class="resultGrid">
          <div class="result"><span class="small">Status</span><b id="gravStatus">IDLE</b></div>
          <div class="result"><span class="small">Initial mass</span><b id="gravInitialR">-</b></div>
          <div class="result"><span class="small">Final mass</span><b id="gravFinal">-</b></div>
          <div class="result"><span class="small">Measured volume</span><b id="gravMeasured">-</b></div>
        </div>
        <div style="height:9px"></div>
        <div class="resultGrid">
          <div class="result"><span class="small">Pump indicated</span><b id="gravPump">-</b></div>
          <div class="result"><span class="small">Target</span><b id="gravTarget">-</b></div>
          <div class="result"><span class="small">Error</span><b id="gravError">-</b></div>
          <div class="result"><span class="small">Density</span><b id="gravDensityR">-</b></div>
        </div>
      </section>
    </div>
  </section>

  <section id="page-history" class="page">
    <section class="panel">
      <div class="actions" style="justify-content:space-between;align-items:center"><h2 style="margin:0">Infusion History</h2><div><button class="btn" onclick="loadHistory()">REFRESH</button><button class="btn danger" onclick="clearHistory()">CLEAR HISTORY</button></div></div>
      <p class="small">The last 20 runs are kept in ESP32 RAM.</p>
      <div class="tableWrap"><table class="table"><thead><tr><th>ID</th><th>Result</th><th>Target</th><th>Pump delivered</th><th>Rate</th><th>Duration</th><th>Gravimetric</th><th>Measured</th><th>Error</th></tr></thead><tbody id="infusionRows"><tr><td colspan="9">Loading...</td></tr></tbody></table></div>
    </section>
  </section>

  <section id="page-alarms" class="page">
    <section class="panel">
      <div class="actions" style="justify-content:space-between;align-items:center"><h2 style="margin:0">Alarm / Fault History</h2><div><button class="btn" onclick="loadAlarms()">REFRESH</button><button class="btn danger" onclick="clearHistory()">CLEAR HISTORY</button></div></div>
      <p class="small">Timestamps are device uptime in ms.</p>
      <div class="tableWrap"><table class="table"><thead><tr><th>Uptime</th><th>State</th><th>Message</th></tr></thead><tbody id="alarmRows"><tr><td colspan="3">Loading...</td></tr></tbody></table></div>
    </section>
  </section>

  <section id="page-diagnostics" class="page">
    <div class="grid">
      <section class="panel">
        <h2>Motion / Position</h2>
        <div class="kv">
          <div><span>State</span><span id="dxState">-</span></div>
          <div><span>Homed</span><span id="dxHomed">-</span></div>
          <div><span>Position</span><span id="dxPos">-</span></div>
          <div><span>Target position</span><span id="dxTargetPos">-</span></div>
          <div><span>Distance to go</span><span id="dxDistance">-</span></div>
          <div><span>Stepper speed</span><span id="dxSpeed">-</span></div>
          <div><span>Acceleration</span><span id="dxAccel">-</span></div>
          <div><span>Steps/mL</span><span id="dxSpm">-</span></div>
        </div>
      </section>
      <section class="panel">
        <h2>Current / Occlusion</h2>
        <div class="kv">
          <div><span>ADC raw</span><span id="dxRaw">-</span></div>
          <div><span>Filtered</span><span id="dxFiltered">-</span></div>
          <div><span>Baseline</span><span id="dxBaseline">-</span></div>
          <div><span>Delta</span><span id="dxDelta">-</span></div>
          <div><span>Threshold</span><span id="dxThreshold">-</span></div>
          <div><span>Hold time</span><span id="dxHold">-</span></div>
          <div><span>Grace time</span><span id="dxGrace">-</span></div>
          <div><span>Baseline locked</span><span id="dxLocked">-</span></div>
        </div>
      </section>
    </div>
    <div class="grid">
      <section class="panel">
        <h2>Limit Switches / HX711</h2>
        <div class="kv">
          <div><span>HOME</span><span id="dxHome">-</span></div>
          <div><span>MAX</span><span id="dxMax">-</span></div>
          <div><span>HX711 ready</span><span id="dxHx">-</span></div>
          <div><span>Load cell grams</span><span id="dxGrams">-</span></div>
          <div><span>Scale factor</span><span id="dxScale">-</span></div>
        </div>
      </section>
      <section class="panel">
        <h2>Network / System</h2>
        <div class="kv">
          <div><span>IP</span><span id="dxIp">-</span></div>
          <div><span>Wi-Fi RSSI</span><span id="dxRssi">-</span></div>
          <div><span>MQTT</span><span id="dxMqtt">-</span></div>
          <div><span>Free heap</span><span id="dxHeap">-</span></div>
          <div><span>Minimum free heap</span><span id="dxMinHeap">-</span></div>
          <div><span>CPU</span><span id="dxCpu">-</span></div>
          <div><span>Uptime</span><span id="dxUptime">-</span></div>
        </div>
      </section>
    </div>
    <section class="panel">
      <h2>System Message</h2>
      <div class="log" id="dxMessage">-</div>
      <div class="actions" style="margin-top:9px"><button class="btn" onclick="cmd('/api/ping')">PING</button><button class="btn" onclick="refresh()">REFRESH LIVE DATA</button></div>
    </section>
  </section>

  <div class="footer">Bench testing only. Not for use on patients.</div>
</div>

<script>
function $(id){return document.getElementById(id)}
function stateClass(s){if(['infusing','idle'].includes(s))return 'good';if(['homing','paused','complete'].includes(s))return 'warn';if(['occluded','alarm'].includes(s))return 'bad';return ''}
function esc(v){return String(v??'').replace(/[&<>"']/g,m=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[m]))}

const validationRecords=[];
let validationRunning=false, validationStartMs=0, validationCfg=null;
let occArmed=false, occArmMs=0, occPeakDelta=0, occSeenState=false;
function validationEsc(v){return esc(String(v??''));}
function validationNow(){return new Date().toISOString();}
function addValidationRecord(r){r.id='VT-'+String(validationRecords.length+1).padStart(3,'0');r.timestamp=validationNow();validationRecords.unshift(r);renderValidationRecords();}
function renderValidationRecords(){const el=$('validationRows');if(!el)return;if(!validationRecords.length){el.innerHTML='<tr><td colspan="8">No validation records.</td></tr>';return}el.innerHTML=validationRecords.map(r=>'<tr><td>'+validationEsc(r.id)+'</td><td>'+validationEsc(r.test)+'</td><td>'+validationEsc(r.target)+'</td><td>'+validationEsc(r.measured)+'</td><td>'+validationEsc(r.error)+'</td><td>'+validationEsc(r.flow)+'</td><td>'+validationEsc(r.duration)+'</td><td><span class="tag '+(r.result==='PASS'?'good':r.result==='FAIL'?'bad':'warn')+'">'+validationEsc(r.result)+'</span></td></tr>').join('')}
function setValidationResult(result){setText('valResult',result||'-');const e=$('valResult');if(e)e.className='tag '+(result==='PASS'?'good':result==='FAIL'?'bad':'warn');}
async function startValidationGravimetric(){
  if(validationRunning){alert('A validation test is already running.');return}
  const target=Number($('valTarget').value), rate=Number($('valRate').value), density=Number($('valDensity').value), tolerance=Number($('valTolerance').value);
  if(![target,rate,density,tolerance].every(Number.isFinite)||target<=0||rate<=0||density<=0||tolerance<=0){alert('Enter valid validation parameters.');return}
  validationCfg={target,rate,density,tolerance};
  try{
    const d=await api('/api/gravimetric/start',{volume_ml:target,rate_ml_hr:rate,density_g_ml:density});
    validationRunning=true;validationStartMs=Date.now();
    setText('valRunState','RUNNING');setValidationResult('RUNNING');setText('valMeasured','-');setText('valError','-');setText('valMeasuredFlow','-');setText('valElapsed','0.0 s');
    $('valStart').disabled=true;$('critVolume').textContent='+/-'+tolerance.toFixed(2)+'%';
    setText('valRunState',d.message||'RUNNING');
  }catch(e){alert(e.message)}
}
function updateValidationFromStatus(d){
  setText('valLiveTarget',Number(d.target_volume_ml||0).toFixed(3));setText('valLiveDelivered',Number(d.delivered_volume_ml||0).toFixed(3));setText('valLiveLoad',Number(d.load_cell_grams||0).toFixed(2));setText('valLiveDelta',Number(d.current_delta||0).toFixed(1));
  const pct=Math.max(0,Math.min(100,(Number(d.delivered_volume_ml||0)/(Number(d.target_volume_ml||0)||1))*100));if($('valBar'))$('valBar').style.width=pct.toFixed(1)+'%';
  if(validationRunning){
    const elapsed=(Date.now()-validationStartMs)/1000;setText('valElapsed',elapsed.toFixed(1)+' s');
    if(!d.gravimetric_active && ['complete','alarm','occluded','idle'].includes(d.state) && (d.gravimetric_result||'idle')!=='idle'){
      const measured=Number(d.gravimetric_measured_ml||0), error=Number(d.gravimetric_error_pct||0), duration=elapsed, flow=duration>0?measured/(duration/3600):0;
      const result=(Math.abs(error)<=validationCfg.tolerance && d.gravimetric_result==='complete')?'PASS':'FAIL';
      setText('valMeasured',measured.toFixed(3)+' mL');setText('valError',error.toFixed(2)+'%');setText('valMeasuredFlow',flow.toFixed(3)+' mL/hr');setValidationResult(result);setText('valRunState',String(d.gravimetric_result||d.state).toUpperCase());
      addValidationRecord({test:'Gravimetric volume/flow',target:validationCfg.target.toFixed(3)+' mL',measured:measured.toFixed(3)+' mL',error:error.toFixed(2)+'%',flow:flow.toFixed(3)+' mL/hr',duration:duration.toFixed(1)+' s',result});
      validationRunning=false;$('valStart').disabled=false;
    }
  }
  if(occArmed){
    const delta=Number(d.current_delta||0);if(delta>occPeakDelta)occPeakDelta=delta;setText('occPeak',occPeakDelta.toFixed(1));setText('occState',String(d.state||'-').toUpperCase());
    if(d.state==='occluded'&&!occSeenState){occSeenState=true;const ms=Date.now()-occArmMs;setText('occDetection',(ms/1000).toFixed(3)+' s');setText('occTimer','DETECTED');setText('occResult','OBSERVED');addValidationRecord({test:'Occlusion response',target:'Firmware alarm',measured:'OCCLUDED',error:'-',flow:Number(d.flow_rate_ml_hr||0).toFixed(3)+' mL/hr',duration:(ms/1000).toFixed(3)+' s',result:'PASS'});occArmed=false}}
}
function armOcclusionTest(){if(occArmed)return;occArmed=true;occSeenState=false;occArmMs=Date.now();occPeakDelta=0;setText('occTimer','ARMED');setText('occDetection','-');setText('occPeak','0.0');setText('occResult','WAITING');}
function disarmOcclusionTest(){occArmed=false;setText('occTimer','NOT ARMED');setText('occResult','-');}
function clearValidationRecords(){if(!confirm('Clear browser-session validation records?'))return;validationRecords.length=0;renderValidationRecords()}
function exportValidationCSV(){if(!validationRecords.length){alert('No validation records to export.');return}const rows=[['ID','Timestamp','Test','Target','Measured','Error','Flow','Duration','Result'],...validationRecords.map(r=>[r.id,r.timestamp,r.test,r.target,r.measured,r.error,r.flow,r.duration,r.result])];const csv=rows.map(row=>row.map(v=>'"'+String(v??'').replace(/"/g,'""')+'"').join(',')).join('\n');const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([csv],{type:'text/csv'}));a.download='validation_session.csv';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)}
function exportValidationReport(){
  if(!validationRecords.length){alert('No validation records to report.');return}
  const rows=validationRecords.map(r=>'<tr><td>'+validationEsc(r.id)+'</td><td>'+validationEsc(r.timestamp)+'</td><td>'+validationEsc(r.test)+'</td><td>'+validationEsc(r.target)+'</td><td>'+validationEsc(r.measured)+'</td><td>'+validationEsc(r.error)+'</td><td>'+validationEsc(r.flow)+'</td><td>'+validationEsc(r.duration)+'</td><td>'+validationEsc(r.result)+'</td></tr>').join('');
  const d=window.lastStatus||{};
  const meta='<table class="metaTable"><tr><th>Device</th><td>'+validationEsc(d.device_id||'-')+'</td><th>IP</th><td>'+validationEsc(d.ip||'-')+'</td></tr><tr><th>Firmware state</th><td>'+validationEsc(d.state||'-')+'</td><th>Homed</th><td>'+validationEsc(d.homed?'YES':'NO')+'</td></tr><tr><th>Steps/mL</th><td>'+validationEsc(Number(d.calibrated_steps_per_ml||0).toFixed(4))+'</td><th>HX711 scale</th><td>'+validationEsc(d.hx711_scale_factor??'-')+'</td></tr><tr><th>Density</th><td>'+validationEsc(Number(d.fluid_density_g_ml||0).toFixed(4))+'</td><th>Report generated</th><td>'+validationEsc(validationNow())+'</td></tr></table>';
  const html='<!doctype html><html><head><meta charset="utf-8"><title>Smart Syringe Pump Validation Report</title><style>body{font-family:Arial,sans-serif;margin:32px;color:#17212b}h1{margin-bottom:4px}.meta{color:#5d6b78;margin-bottom:18px}.metaTable,table{border-collapse:collapse;width:100%;font-size:12px}.metaTable th,.metaTable td,table th,table td{border:1px solid #ccd4dc;padding:7px;text-align:left}.metaTable th,table th{background:#eef2f5}.section{margin-top:22px}.note{margin-top:20px;padding:12px;background:#fff3cd}.sign{margin-top:36px;display:grid;grid-template-columns:1fr 1fr;gap:36px}.line{border-bottom:1px solid #555;height:28px}.small{font-size:11px;color:#5d6b78}</style></head><body><h1>Smart Syringe Pump</h1><div class="meta">Validation &amp; Test Report</div><div class="section"><h2>Device / Configuration Snapshot</h2>'+meta+'</div><div class="section"><h2>Validation Records</h2><table><thead><tr><th>ID</th><th>Timestamp</th><th>Test</th><th>Target</th><th>Measured</th><th>Error</th><th>Flow</th><th>Duration</th><th>Result</th></tr></thead><tbody>'+rows+'</tbody></table></div><div class="section"><h2>Notes</h2><ul><li>Flow is calculated from gravimetric volume and elapsed time.</li></ul></div><div class="sign"><div><div class="line"></div><div class="small">Engineer / Tester</div></div><div><div class="line"></div><div class="small">Date / Review</div></div></div></body></html>';
  const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([html],{type:'text/html'}));a.download='validation_report.html';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)
}


let wizardRunning=false,wizardStartMs=0,wizardCfg=null;
let stabilityRunning=false, stabilityCfg=null, stabilityIndex=0, stabilityResults=[], stabilityRunStartMs=0, stabilityWaiting=false;
let alertSnoozeUntil=0,alertSnoozeKey='';function snoozeAlert(){alertSnoozeKey=window.__alertKey||'';alertSnoozeUntil=Date.now()+300000;const b=$('alertBanner');if(b)b.classList.remove('show')}let auditLog=JSON.parse(localStorage.getItem('syringe_audit_log')||'[]');
window.lastStatus={};
async function loadCalibrationWizard(){try{const d=await api('/api/setup');setText('wizCurrentSpm',Number(d.steps_per_ml||0).toFixed(4)+' steps/mL');setText('wizCurrentScale',d.hx711_scale??'-');setText('wizCurrentDensity',Number(d.density_g_ml||0).toFixed(4)+' g/mL');setText('wizPosition',d.position??'-');setText('wizHomed',d.homed?'YES':'NO');$('wizDensity').value=Number(d.density_g_ml||1).toFixed(4);setText('wizCalcCurrent',Number(d.steps_per_ml||0).toFixed(4));}catch(e){alert(e.message)}}
async function startCalibrationRun(){if(wizardRunning){alert('A calibration run is already active.');return}const target=Number($('wizTarget').value),rate=Number($('wizRate').value),density=Number($('wizDensity').value);if(![target,rate,density].every(Number.isFinite)||target<=0||rate<=0||density<=0){alert('Enter valid calibration parameters.');return}if(!$('wizHomed').textContent.includes('YES')){alert('Home the pump before calibration.');return}wizardCfg={target,rate,density};try{const d=await api('/api/gravimetric/start',{volume_ml:target,rate_ml_hr:rate,density_g_ml:density});wizardRunning=true;wizardStartMs=Date.now();$('wizStart').disabled=true;setText('wizRunState','RUNNING');setText('wizMeasured','-');setText('wizError','-');setText('wizApplyStatus','Calibration run started.');}catch(e){alert(e.message)}}
function updateCalibrationWizard(d){if(!d)return;setText('wizCurrentSpm',Number(d.calibrated_steps_per_ml||0).toFixed(4)+' steps/mL');setText('wizCurrentScale',d.hx711_scale_factor??'-');setText('wizCurrentDensity',Number(d.fluid_density_g_ml||0).toFixed(4)+' g/mL');setText('wizPosition',d.position??'-');setText('wizHomed',d.homed?'YES':'NO');if(wizardRunning&&!d.gravimetric_active&&['complete','alarm','occluded','idle'].includes(d.state)&&(d.gravimetric_result||'idle')!=='idle'){const measured=Number(d.gravimetric_measured_ml||0),error=Number(d.gravimetric_error_pct||0),current=Number(d.calibrated_steps_per_ml||0);setText('wizMeasured',measured.toFixed(3)+' mL');setText('wizError',error.toFixed(2)+'%');setText('wizRunState',String(d.gravimetric_result||d.state).toUpperCase());if(measured>0&&current>0&&wizardCfg){const proposed=current*wizardCfg.target/measured;setText('wizCalcCurrent',current.toFixed(4));setText('wizCalcTarget',wizardCfg.target.toFixed(3)+' mL');setText('wizCalcMeasured',measured.toFixed(3)+' mL');setText('wizProposed',proposed.toFixed(4)+' steps/mL');$('wizApply').disabled=false;$('wizApply').dataset.proposed=proposed.toFixed(6);setText('wizApplyStatus','Proposed correction calculated. Apply only after reviewing the test result.')}else{$('wizApply').disabled=true}addValidationRecord({test:'Calibration wizard gravimetric run',target:wizardCfg.target.toFixed(3)+' mL',measured:measured.toFixed(3)+' mL',error:error.toFixed(2)+'%',flow:(measured/((Date.now()-wizardStartMs)/3600000)).toFixed(3)+' mL/hr',duration:((Date.now()-wizardStartMs)/1000).toFixed(1)+' s',result:d.gravimetric_result==='complete'?'OBSERVED':'FAIL'});wizardRunning=false;$('wizStart').disabled=false}}
async function applyWizardCalibration(){const proposed=Number($('wizApply').dataset.proposed||0);if(!Number.isFinite(proposed)||proposed<=0){alert('No valid proposed calibration.');return}if(!confirm('Apply '+proposed.toFixed(4)+' steps/mL to ESP32 NVS? The pump must remain IDLE, homed, and at HOME.'))return;try{const d=await api('/api/config/save',{steps_per_ml:proposed,hx711_scale:Number($('wizCurrentScale').textContent),density_g_ml:Number($('wizCurrentDensity').textContent)});setText('wizApplyStatus',d.message||'Calibration applied.');$('wizApply').disabled=true;await loadCalibrationWizard();await refresh()}catch(e){alert(e.message)}}


function stabilityEsc(v){return esc(v)}
function renderStabilityRuns(){const b=$('stabRows');if(!b)return;b.innerHTML=stabilityResults.length?stabilityResults.map(r=>'<tr><td>'+r.run+'</td><td>'+Number(r.target).toFixed(3)+'</td><td>'+Number(r.measured).toFixed(3)+'</td><td>'+Number(r.error).toFixed(2)+'%</td><td>'+Number(r.flow).toFixed(2)+'</td><td>'+Number(r.duration).toFixed(1)+' s</td><td><span class="tag '+(r.result==='PASS'?'good':'bad')+'">'+r.result+'</span></td></tr>').join(''):'<tr><td colspan="7">No stability runs.</td></tr>'}
function updateStabilitySummary(){const vals=stabilityResults.map(r=>Number(r.measured)).filter(Number.isFinite);const target=Number(stabilityCfg?.target||$('stabTarget')?.value||0);if(!vals.length){['stabMean','stabStd','stabCv','stabMaxErr','stabResult'].forEach(id=>setText(id,'-'));return}const mean=vals.reduce((a,b)=>a+b,0)/vals.length;const variance=vals.length>1?vals.reduce((a,b)=>a+(b-mean)**2,0)/(vals.length-1):0;const sd=Math.sqrt(variance);const cv=mean?sd/mean*100:Infinity;const maxErr=Math.max(...stabilityResults.map(r=>Math.abs(Number(r.error)||0)));const tol=Number(stabilityCfg?.tol||2),cvTol=Number(stabilityCfg?.cvTol||1);setText('stabMean',mean.toFixed(4)+' mL');setText('stabStd',sd.toFixed(4)+' mL');setText('stabCv',Number.isFinite(cv)?cv.toFixed(3)+'%':'-');setText('stabMaxErr',maxErr.toFixed(3)+'%');setText('stabResult',stabilityResults.length===stabilityCfg?.runs&&maxErr<=tol&&cv<=cvTol?'PASS':'IN PROGRESS')}
async function startStabilityTest(){if(stabilityRunning){alert('A stability test is already running.');return}const runs=Math.round(Number($('stabRuns').value)),target=Number($('stabTarget').value),rate=Number($('stabRate').value),tol=Number($('stabTol').value),cvTol=Number($('stabCvTol').value),density=Number($('stabDensity').value);if(![runs,target,rate,tol,cvTol,density].every(Number.isFinite)||runs<2||runs>20||target<=0||rate<=0||tol<=0||cvTol<=0||density<=0){alert('Enter valid stability test parameters.');return}if(!window.lastStatus.homed||window.lastStatus.state!=='idle'){alert('Pump must be IDLE and homed before starting the stability sequence.');return}stabilityCfg={runs,target,rate,tol,cvTol,density};stabilityResults=[];stabilityIndex=0;stabilityRunning=true;stabilityWaiting=false;renderStabilityRuns();updateStabilitySummary();setText('stabRunNo','0 / '+runs);setText('stabState','STARTING');$('stabStart').disabled=true;addAudit('STABILITY_START',JSON.stringify(stabilityCfg));await launchNextStabilityRun()}
async function launchNextStabilityRun(){if(!stabilityRunning)return;if(stabilityIndex>=stabilityCfg.runs){finishStabilityTest();return}stabilityWaiting=false;stabilityRunStartMs=Date.now();setText('stabRunNo',(stabilityIndex+1)+' / '+stabilityCfg.runs);setText('stabState','RUN '+(stabilityIndex+1)+' STARTING');try{await api('/api/gravimetric/start',{volume_ml:stabilityCfg.target,rate_ml_hr:stabilityCfg.rate,density_g_ml:stabilityCfg.density});addAudit('STABILITY_RUN_START','run '+(stabilityIndex+1));setText('stabState','RUNNING')}catch(e){stabilityRunning=false;$('stabStart').disabled=false;setText('stabState','ERROR');addAudit('STABILITY_RUN_START_FAILED',e.message);alert(e.message)}}
function updateStabilityFromStatus(d){if(!stabilityRunning||!stabilityCfg)return;setText('stabState',d.gravimetric_active?'RUNNING':String(d.gravimetric_result||d.state||'WAITING').toUpperCase());if(d.gravimetric_active)return;if(stabilityWaiting){if(d.state==='idle'&&d.homed&&stabilityIndex<stabilityCfg.runs){stabilityWaiting=false;setTimeout(()=>launchNextStabilityRun(),500)}return;}if(stabilityIndex>=stabilityCfg.runs)return;if(!['complete','alarm','occluded'].includes(d.state)&&d.gravimetric_result!=='complete'&&d.gravimetric_result!=='failed')return;const elapsed=(Date.now()-stabilityRunStartMs)/1000;const measured=Number(d.gravimetric_measured_ml||0);const error=stabilityCfg.target>0?(measured-stabilityCfg.target)/stabilityCfg.target*100:0;const flow=elapsed>0?measured/(elapsed/3600):0;const result=(d.gravimetric_result==='complete'&&Math.abs(error)<=stabilityCfg.tol)?'PASS':'FAIL';stabilityResults.push({run:stabilityIndex+1,target:stabilityCfg.target,measured,error,flow,duration:elapsed,result});renderStabilityRuns();updateStabilitySummary();addAudit('STABILITY_RUN_RESULT','run '+(stabilityIndex+1)+' '+result+' error='+error.toFixed(3)+'%');stabilityIndex++;stabilityWaiting=true;if(result!=='PASS' && d.gravimetric_result!=='complete'){stabilityRunning=false;$('stabStart').disabled=false;setText('stabState','ABORTED - REVIEW');return}if(d.state==='complete'){setText('stabState','RE-HOMING BETWEEN RUNS');cmd('/api/home')}else if(d.state!=='idle'){setText('stabState','WAITING FOR IDLE')}else{setTimeout(()=>{stabilityWaiting=false;launchNextStabilityRun()},700)}}
function finishStabilityTest(){stabilityRunning=false;$('stabStart').disabled=false;const pass=stabilityResults.length===stabilityCfg.runs&&$('stabResult').textContent==='PASS';setText('stabState',pass?'PASS':'COMPLETE / REVIEW');setText('stabRunNo',stabilityCfg.runs+' / '+stabilityCfg.runs);addAudit('STABILITY_COMPLETE',pass?'PASS':'REVIEW');alert('Stability sequence complete. Review the recorded runs and acceptance summary.')}
function stopStabilityTest(){if(!stabilityRunning)return;stabilityRunning=false;$('stabStart').disabled=false;addAudit('STABILITY_ABORT','operator requested abort');cmd('/api/stop');setText('stabState','ABORTED')}
function clearStabilityRuns(){if(stabilityRunning){alert('Stop the stability sequence first.');return}if(confirm('Clear stability run records?')){stabilityResults=[];renderStabilityRuns();updateStabilitySummary()}}
function exportStabilityCSV(){if(!stabilityResults.length){alert('No stability records.');return}const head='run,target_ml,measured_ml,error_pct,flow_ml_hr,duration_s,result\n';const rows=stabilityResults.map(r=>[r.run,r.target,r.measured,r.error,r.flow,r.duration,r.result].map(csvCell).join(','));downloadText('stability_test.csv',head+rows.join('\n')+'\n','text/csv;charset=utf-8')}


function showPage(p){document.querySelectorAll('.page').forEach(x=>x.classList.remove('active'));document.querySelectorAll('.nav button').forEach(x=>x.classList.remove('active'));const el=$('page-'+p);if(el)el.classList.add('active');const b=document.querySelector('.nav button[data-page="'+p+'"]');if(b)b.classList.add('active');location.hash=p;if(p==='history')loadHistory();if(p==='alarms')loadAlarms();if(p==='setup')loadSetup();if(p==='calwizard')loadCalibrationWizard();if(p==='audit')renderAuditLog();if(p==='stability')renderStabilityRuns()}
function initPage(){const p=(location.hash||'#dashboard').substring(1);showPage(['dashboard','infusion','safety','setup','calwizard','gravimetric','history','alarms','diagnostics','graphs','validation','stability','audit'].includes(p)?p:'dashboard')}
let graphSamples=[];const MAX_GRAPH_SAMPLES=300;
function addGraphSample(d){const now=Date.now();graphSamples.push({time:now,uptime_s:Number(d.uptime_s||0),flow_target:Number(d.flow_rate_ml_hr||0),flow_actual:Number(d.flow_rate_ml_hr||0),volume_target:Number(d.target_volume_ml||0),volume_actual:Number(d.delivered_volume_ml||0),current:Number(d.current_adc||0),load:Number(d.load_cell_grams||0)});if(graphSamples.length>MAX_GRAPH_SAMPLES)graphSamples.shift();drawGraphs()}
function clearGraphData(){if(confirm('Clear the graph data captured in this browser session?')){graphSamples=[];resetAllGraphs();drawGraphs()}}
function downloadCanvas(id,name){const c=$(id);const a=document.createElement('a');a.href=c.toDataURL('image/png');a.download=name;a.click()}
function downloadAllGraphsPNG(){const ids=['flowCanvas','volumeCanvas','currentCanvas','loadCanvas'];const out=document.createElement('canvas');out.width=1520;out.height=600;const x=out.getContext('2d');x.fillStyle='#091018';x.fillRect(0,0,out.width,out.height);x.fillStyle='#eaf2f8';x.font='bold 20px Arial';x.fillText('Smart Syringe Pump - Graphs',24,30);ids.forEach((id,i)=>{const src=$(id);const dx=(i%2)*760,dy=45+Math.floor(i/2)*275;x.drawImage(src,dx,dy,740,260)});const a=document.createElement('a');a.href=out.toDataURL('image/png');a.download='syringe_pump_engineering_graphs.png';a.click()}
function downloadGraphCSV(){if(!graphSamples.length){alert('No graph samples captured yet.');return}const head='timestamp,uptime_s,flow_target_ml_hr,flow_actual_ml_hr,volume_target_ml,volume_actual_ml,motor_current_adc,load_cell_g\n';const rows=graphSamples.map(x=>[new Date(x.time).toISOString(),x.uptime_s,x.flow_target,x.flow_actual,x.volume_target,x.volume_actual,x.current,x.load].join(','));const blob=new Blob([head+rows.join('\n')+'\n'],{type:'text/csv;charset=utf-8'});const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download='syringe_pump_graph_data.csv';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)}
const graphViews={};
function graphView(id){return graphViews[id]||(graphViews[id]={x0:0,x1:1,yZoom:1,panX:0,panY:0,pointers:new Map(),dragging:false,lastX:0,lastY:0,pinchDist:0})}
function resetGraph(id){const v=graphView(id);v.x0=0;v.x1=1;v.yZoom=1;v.panX=0;v.panY=0;drawGraphs()}
function resetAllGraphs(){Object.keys(graphViews).forEach(resetGraph)}
function clamp(v,a,b){return Math.max(a,Math.min(b,v))}
function graphSeries(id){if(id==='flowCanvas')return [graphSamples.map(x=>x.flow_actual),null,'mL/hr','Commanded flow',null];if(id==='volumeCanvas')return [graphSamples.map(x=>x.volume_actual),graphSamples.map(x=>x.volume_target),'mL','Actual','Target'];if(id==='currentCanvas')return [graphSamples.map(x=>x.current),null,'ADC','Motor current',null];return [graphSamples.map(x=>x.load),null,'g','Load cell',null]}
function graphIndexAt(c,clientX){const r=c.getBoundingClientRect();return clamp((clientX-r.left)/r.width,0,1)}
function zoomGraph(id,factor,cx,cy){const v=graphView(id);let span=v.x1-v.x0;let ns=clamp(span*factor,1/Math.max(1,graphSamples.length-1),1);const center=v.x0+(v.x1-v.x0)*cx;v.x0=clamp(center-ns*cx,0,1-ns);v.x1=v.x0+ns;v.yZoom=clamp(v.yZoom/factor,0.25,20);drawGraphs()}
function panGraph(id,dx,dy){const v=graphView(id);const span=v.x1-v.x0;const shift=dx*span;v.x0=clamp(v.x0-shift,0,1-span);v.x1=v.x0+span;v.panY=clamp(v.panY+dy,-0.8,0.8);drawGraphs()}
function graphPointerDown(e){const c=e.currentTarget,v=graphView(c.id);c.setPointerCapture?.(e.pointerId);v.pointers.set(e.pointerId,{x:e.clientX,y:e.clientY});if(v.pointers.size===1){v.dragging=true;v.lastX=e.clientX;v.lastY=e.clientY;c.classList.add('dragging')}else if(v.pointers.size===2){const pts=[...v.pointers.values()];v.pinchDist=Math.hypot(pts[0].x-pts[1].x,pts[0].y-pts[1].y)}}
function graphPointerMove(e){const c=e.currentTarget,v=graphView(c.id);if(!v.pointers.has(e.pointerId))return;v.pointers.set(e.pointerId,{x:e.clientX,y:e.clientY});if(v.pointers.size===2){const pts=[...v.pointers.values()],d=Math.hypot(pts[0].x-pts[1].x,pts[0].y-pts[1].y);if(v.pinchDist>0){const f=clamp(v.pinchDist/d,0.85,1.18);const mid=(pts[0].x+pts[1].x)/2;zoomGraph(c.id,f,graphIndexAt(c,mid),0.5)}v.pinchDist=d;return}if(v.dragging){const r=c.getBoundingClientRect();const dx=(e.clientX-v.lastX)/r.width;const dy=(e.clientY-v.lastY)/r.height;panGraph(c.id,dx,dy);v.lastX=e.clientX;v.lastY=e.clientY}}
function graphPointerUp(e){const c=e.currentTarget,v=graphView(c.id);v.pointers.delete(e.pointerId);if(v.pointers.size<2)v.pinchDist=0;if(v.pointers.size===0){v.dragging=false;c.classList.remove('dragging')}}
function graphWheel(e){e.preventDefault();const f=e.deltaY<0?0.82:1.22;zoomGraph(e.currentTarget.id,f,graphIndexAt(e.currentTarget,e.clientX),0.5)}
function graphDoubleClick(e){e.preventDefault();resetGraph(e.currentTarget.id)}
function initGraphInteractions(){document.querySelectorAll('.graphCanvas').forEach(c=>{if(c.dataset.interactive)return;c.dataset.interactive='1';c.addEventListener('pointerdown',graphPointerDown);c.addEventListener('pointermove',graphPointerMove);c.addEventListener('pointerup',graphPointerUp);c.addEventListener('pointercancel',graphPointerUp);c.addEventListener('wheel',graphWheel,{passive:false});c.addEventListener('dblclick',graphDoubleClick)})}
function drawGraphs(){drawLine('flowCanvas',graphSamples.map(x=>x.flow_actual),null,'mL/hr','Commanded flow',null);drawLine('volumeCanvas',graphSamples.map(x=>x.volume_actual),graphSamples.map(x=>x.volume_target),'mL','Actual','Target');drawLine('currentCanvas',graphSamples.map(x=>x.current),null,'ADC','Motor current',null);drawLine('loadCanvas',graphSamples.map(x=>x.load),null,'g','Load cell',null);initGraphInteractions()}
function drawLine(id,a,b,unit,labelA,labelB){const c=$(id),ctx=c.getContext('2d'),w=c.width,h=c.height,p=38;ctx.clearRect(0,0,w,h);ctx.font='11px Arial';ctx.fillStyle='#91a4b6';ctx.lineWidth=1;const vals=a.concat(b||[]);if(!vals.length){ctx.fillText('Waiting for samples...',p+10,h/2);return}const v=graphView(id);const n=vals.length;let xs=clamp(Math.floor(v.x0*(n-1)),0,n-1),xe=clamp(Math.ceil(v.x1*(n-1)),xs+1,n-1);const visible=[];for(let i=xs;i<=xe;i++){if(a[i]!=null)visible.push(a[i]);if(b&&b[i]!=null)visible.push(b[i])}let min=Math.min(...visible),max=Math.max(...visible);if(min===max){min-=1;max+=1}const baseSpan=max-min,center=(min+max)/2,span=baseSpan/v.yZoom,ymid=center+v.panY*baseSpan;min=ymid-span/2;max=ymid+span/2;const yspan=max-min;ctx.strokeStyle='#2a3a49';ctx.beginPath();ctx.moveTo(p,15);ctx.lineTo(p,h-p);ctx.lineTo(w-12,h-p);ctx.stroke();for(let i=0;i<5;i++){const y=15+(h-p-15)*(i/4),val=max-yspan*(i/4);ctx.fillStyle='#91a4b6';ctx.fillText(val.toFixed(2),4,y+4);ctx.strokeStyle='#182530';ctx.beginPath();ctx.moveTo(p,y);ctx.lineTo(w-12,y);ctx.stroke()}function plot(data,stroke){if(!data||!data.length)return;ctx.strokeStyle=stroke;ctx.lineWidth=2;ctx.beginPath();let started=false;for(let i=xs;i<=xe;i++){const val=data[i];if(!Number.isFinite(val))continue;const x=p+(w-p-14)*((i-xs)/Math.max(1,xe-xs));const y=15+(h-p-15)*(1-(val-min)/yspan);if(!started){ctx.moveTo(x,y);started=true}else ctx.lineTo(x,y)}ctx.stroke()}plot(a,'#59a7ff');plot(b,'#42d392');ctx.fillStyle='#eaf2f8';ctx.fillText(labelA,p,12);if(labelB){ctx.fillStyle='#42d392';ctx.fillText(labelB,p+85,12)}ctx.fillStyle='#91a4b6';ctx.fillText(unit,w-65,h-7);const t0=graphSamples[xs]?.uptime_s||0,t1=graphSamples[xe]?.uptime_s||0;ctx.fillText(t0.toFixed(0)+' s',p,h-7);ctx.fillText(t1.toFixed(0)+' s',w-55,h-7);if(xs>0||xe<n-1){ctx.fillStyle='#ffca5c';ctx.fillText('ZOOMED',w-110,12)}}
async function logout(){try{await fetch('/api/logout',{method:'POST'});}catch(e){}location.href='/';}

async function api(path,body=null){const opt={method:body===null?'GET':'POST',headers:{'Content-Type':'application/json'}};if(body!==null)opt.body=JSON.stringify(body);const r=await fetch(path,opt);if(r.status===401){location.href='/';throw new Error('Login required');}const d=await r.json();if(!r.ok||d.ok===false)throw new Error(d.message||'Request failed');return d}
function addAudit(action,detail){const d=window.lastStatus||{};auditLog.unshift({timestamp:new Date().toISOString(),action,detail:detail||'',state:d.state||'unknown'});if(auditLog.length>500)auditLog.length=500;localStorage.setItem('syringe_audit_log',JSON.stringify(auditLog));renderAuditLog()}
function renderAuditLog(){const body=$('auditRows');if(!body)return;body.innerHTML=auditLog.length?auditLog.map(r=>'<tr><td>'+esc(r.timestamp)+'</td><td>'+esc(r.action)+'</td><td>'+esc(r.detail)+'</td><td>'+esc(r.state)+'</td></tr>').join(''):'<tr><td colspan="4">No audit events.</td></tr>'}
function exportAuditCSV(){const head='timestamp,action,detail,state\n';const rows=auditLog.map(r=>[r.timestamp,r.action,r.detail,r.state].map(csvCell).join(','));downloadText('syringe_audit_log.csv',head+rows.join('\n')+'\n','text/csv;charset=utf-8')}
function clearAuditLog(){if(!confirm('Clear the browser audit log?'))return;auditLog=[];localStorage.removeItem('syringe_audit_log');renderAuditLog()}
function csvCell(v){const s=String(v??'');return '"'+s.replace(/"/g,'""')+'"'}
function downloadText(name,text,type){const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([text],{type}));a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)}
async function cmd(path,body=null){try{addAudit('COMMAND',path+(body?' '+JSON.stringify(body):''));const d=await api(path,body===null?{}:body);if(d.message)$('event').textContent=d.message;await refresh()}catch(e){addAudit('COMMAND_FAILED',path+' - '+e.message);$('event').textContent='Command failed: '+e.message}}
function startPump(){const volume=Number($('volume').value),rate=Number($('rateIn').value);if(!Number.isFinite(volume)||!Number.isFinite(rate)){alert('Enter valid volume and rate');return}cmd('/api/start',{volume_ml:volume,rate_ml_hr:rate})}
async function saveConfig(){try{const d=await api('/api/config/save',{steps_per_ml:Number($('cfgSpm').value),hx711_scale:Number($('cfgScale').value),density_g_ml:Number($('cfgDensity').value)});alert(d.message);await loadSetup();await refresh()}catch(e){alert(e.message)}}
async function startGrav(){try{const v=Number($('gravVol').value),r=Number($('gravRate').value),den=Number($('gravDensity').value);const d=await api('/api/gravimetric/start',{volume_ml:v,rate_ml_hr:r,density_g_ml:den});$('gravStatus').textContent=d.message||'STARTED';await refresh()}catch(e){alert(e.message)}}
async function loadSetup(){try{const d=await api('/api/setup');$('cfgSpm').value=Number(d.steps_per_ml).toFixed(4);$('cfgScale').value=d.hx711_scale;$('cfgDensity').value=Number(d.density_g_ml).toFixed(4);$('cfgAccel').value=Number(d.acceleration).toFixed(1);$('currentSpm').textContent=Number(d.steps_per_ml).toFixed(4);$('oneMlSteps').textContent=Math.round(d.steps_per_ml)+' steps';$('sixtyMlSteps').textContent=Math.round(d.steps_per_ml*60)+' steps';$('gravDensity').value=Number(d.density_g_ml).toFixed(4)}catch(e){}}
async function loadHistory(){try{const d=await api('/api/history/infusions');const rows=d.records||[];$('infusionRows').innerHTML=rows.length?rows.map(r=>'<tr><td>'+r.id+'</td><td><span class="tag '+(r.result==='complete'?'good':r.result==='alarm'||r.result==='occluded'?'bad':'warn')+'">'+esc(r.result)+'</span></td><td>'+Number(r.target_ml).toFixed(3)+'</td><td>'+Number(r.pump_delivered_ml).toFixed(3)+'</td><td>'+Number(r.rate_ml_hr).toFixed(1)+'</td><td>'+Number(r.duration_s).toFixed(1)+' s</td><td>'+(r.gravimetric?'YES':'NO')+'</td><td>'+Number(r.measured_ml).toFixed(3)+'</td><td>'+Number(r.error_pct).toFixed(2)+'%</td></tr>').join(''):'<tr><td colspan="9">No infusion records.</td></tr>'}catch(e){$('infusionRows').innerHTML='<tr><td colspan="9">'+esc(e.message)+'</td></tr>'}}
async function loadAlarms(){try{const d=await api('/api/history/alarms');const rows=d.records||[];$('alarmRows').innerHTML=rows.length?rows.map(r=>'<tr><td>'+Number(r.uptime_s).toFixed(1)+' s</td><td><span class="tag bad">'+esc(r.state)+'</span></td><td>'+esc(r.message)+'</td></tr>').join(''):'<tr><td colspan="3">No alarm records.</td></tr>'}catch(e){$('alarmRows').innerHTML='<tr><td colspan="3">'+esc(e.message)+'</td></tr>'}}
async function clearHistory(){if(!confirm('Clear infusion and alarm history from RAM?'))return;try{await api('/api/history/clear',{});await loadHistory();await loadAlarms()}catch(e){alert(e.message)}}
function setText(id,v){if($(id))$(id).textContent=v}

function setSafetyCard(id,level,status){const el=$(id);if(!el)return;el.className='safetyCard '+level;const s=el.querySelector('.status');if(s)s.textContent=status}
function formatEta(seconds){if(!Number.isFinite(seconds)||seconds<0||seconds>864000)return '-';const s=Math.round(seconds),h=Math.floor(s/3600),m=Math.floor((s%3600)/60),sec=s%60;return h?`${h}h ${String(m).padStart(2,'0')}m`:m?`${m}m ${String(sec).padStart(2,'0')}s`:`${sec}s`}
function updateSafety(d){const fault=['alarm','occluded'].includes(d.state);setSafetyCard('safeState',fault?'bad':(['paused','homing','priming'].includes(d.state)?'warn':'ok'),String(d.state||'unknown').toUpperCase());setSafetyCard('safeHome',d.home_limit?'warn':'ok',d.home_limit?'ACTIVE':'OPEN');setSafetyCard('safeMax',d.max_limit?'bad':'ok',d.max_limit?'ACTIVE':'OPEN');const occ=d.state==='occluded';setSafetyCard('safeOcc',occ?'bad':d.occlusion_baseline_locked?'ok':'warn',occ?'OCCLUDED':d.occlusion_baseline_locked?'MONITORING':'LEARNING');setSafetyCard('safeLoad',d.load_cell_ready?'ok':'bad',d.load_cell_ready?'READY':'NOT READY');setSafetyCard('safeWifi',d.wifi_connected?'ok':'bad',d.wifi_connected?'CONNECTED':'OFFLINE');setSafetyCard('safeMqtt',d.mqtt_connected?'ok':'warn',d.mqtt_connected?'CONNECTED':'OFFLINE');setSafetyCard('safeHomeValid',d.homed?'ok':'warn',d.homed?'HOMED':'RE-HOME REQUIRED');setText('safeCurrentState',String(d.state||'unknown').toUpperCase());setText('safeDelta',Number(d.current_delta||0).toFixed(1));setText('safeThreshold',d.occlusion_delta_counts??'-');setText('safeBaseline',d.occlusion_baseline_locked?'LOCKED':'LEARNING');setText('safeLastEvent',d.last_event||'-');setText('safeAlarmCount',d.alarm_history_count??0);const msg=d.message||((d.state==='occluded'||d.state==='alarm')?d.last_event:'No active alarm or fault message.');setText('safetyAlarm',msg);const box=$('safetyMessage');if(box){box.className='notice '+(fault?'criticalNote':'mutedNote');box.textContent=fault?'ACTIVE FAULT: '+msg:'No active alarm. Continue monitoring system status.'}const banner=$('alertBanner');if(banner){const alertKey=d.state+'|'+msg;if(!fault)alertSnoozeUntil=0;window.__alertKey=alertKey;banner.classList.toggle('show',fault&&!(Date.now()<alertSnoozeUntil&&alertKey===alertSnoozeKey));setText('alertTitle',d.state==='occluded'?'OCCLUSION / FLOW INTERRUPTION':'CRITICAL PUMP ALARM');setText('alertMessage',msg)}}
function updateInfusion(d){setText('infState',String(d.state||'unknown'));$('infState').className='state '+stateClass(d.state);setText('infHomed','Homed: '+(d.homed?'YES':'NO'));const target=Number(d.target_volume_ml||0),del=Number(d.delivered_volume_ml||0),remaining=Math.max(0,target-del),pct=target>0?Math.min(100,Math.max(0,del/target*100)):0;setText('infTarget',target.toFixed(3));setText('infDelivered',del.toFixed(3));setText('infRemaining',remaining.toFixed(3));setText('infPercent',pct.toFixed(1));$('infBar').style.width=pct+'%';setText('infRate',Number(d.flow_rate_ml_hr||0).toFixed(1)+' mL/hr');setText('infPosition',d.position??'-');setText('infDistance',d.distance_to_go??'-');setText('infCurrent',Number(d.current_adc||0).toFixed(0)+' ADC');setText('infLoad',d.load_cell_ready?(Number(d.load_cell_grams||0).toFixed(2)+' g'):'N/A');const rate=Number(d.flow_rate_ml_hr||0);setText('infEta',rate>0?formatEta(remaining/rate*3600):'-');$('infPause').disabled=d.state!=='infusing';$('infResume').disabled=d.state!=='paused';const max=Math.max(1,Number(d.max_steps||12463)),pos=Math.max(0,Math.min(max,Number(d.position||0))),travelPct=pos/max*100;setText('syringePct',travelPct.toFixed(1)+'%');setText('syringePos',pos);setText('syringeMax',max);setText('syringeAvail',Math.max(0,60-pos/max*60).toFixed(3)+' mL');$('syringeFill').style.height=Math.max(0,100-travelPct)+'%';$('syringePlunger').style.bottom=Math.max(0,Math.min(100,travelPct))+'%'}
async function refresh(){try{const r=await fetch('/api/status',{cache:'no-store'});const d=await r.json();window.lastStatus=d;updateCalibrationWizard(d);updateStabilityFromStatus(d);
setText('state',d.state||'unknown');$('state').className='state '+stateClass(d.state);updateSafety(d);updateValidationFromStatus(d);updateInfusion(d);setText('homed','Homed: '+(d.homed?'YES':'NO'));setText('target',Number(d.target_volume_ml||0).toFixed(3));setText('delivered',Number(d.delivered_volume_ml||0).toFixed(3));setText('rate',Number(d.flow_rate_ml_hr||0).toFixed(1));setText('pos',d.position??'-');
const pct=d.total_steps>0?Math.min(100,Math.max(0,(d.steps_completed/d.total_steps)*100)):0;$('bar').style.width=pct+'%';setText('current',Number(d.current_adc||0).toFixed(0));setText('currentDelta',Number(d.current_delta||0).toFixed(1));setText('load',d.load_cell_ready?(Number(d.load_cell_grams||0).toFixed(2)+' g'):'N/A');setText('home',d.home_limit?'ACTIVE':'OPEN');setText('max',d.max_limit?'ACTIVE':'OPEN');setText('baselineLock',d.occlusion_baseline_locked?'LOCKED':'LEARNING');
setText('device',d.device_id||'-');setText('rssi',d.wifi_connected?(d.rssi+' dBm'):'offline');setText('mqtt',d.mqtt_connected?'CONNECTED':'OFFLINE');setText('accel',Number(d.acceleration||0).toFixed(1)+' steps/s²');setText('spm',Number(d.calibrated_steps_per_ml||0).toFixed(4));setText('maxSteps',d.max_steps??'-');setText('event',(d.last_event||'No event')+'\n'+Math.round((d.last_event_age_ms||0)/1000)+' s ago');setText('net',d.wifi_connected?'SMARTSYRINGE.LOCAL | '+d.ip:'Wi-Fi offline');
$('startBtn').disabled=!(d.state==='idle'&&d.homed);$('pauseBtn').disabled=d.state!=='infusing';$('resumeBtn').disabled=d.state!=='paused';
addGraphSample(d);setText('gravStatus',d.gravimetric_active?'RUNNING':(d.gravimetric_result||'IDLE').toUpperCase());setText('gravInitialR',Number(d.gravimetric_initial_g||0).toFixed(2)+' g');setText('gravFinal',Number(d.gravimetric_final_g||0).toFixed(2)+' g');setText('gravMeasured',Number(d.gravimetric_measured_ml||0).toFixed(3)+' mL');setText('gravPump',Number(d.gravimetric_pump_ml||0).toFixed(3)+' mL');setText('gravTarget',Number(d.gravimetric_target_ml||0).toFixed(3)+' mL');setText('gravError',Number(d.gravimetric_error_pct||0).toFixed(2)+'%');setText('gravDensityR',Number(d.gravimetric_density||0).toFixed(4)+' g/mL');$('gravStart').disabled=!!d.gravimetric_active;
setText('dxState',d.state);setText('dxHomed',d.homed?'YES':'NO');setText('dxPos',d.position);setText('dxTargetPos',d.target_position);setText('dxDistance',d.distance_to_go);setText('dxSpeed',Number(d.stepper_speed||0).toFixed(2));setText('dxAccel',Number(d.acceleration||0).toFixed(2));setText('dxSpm',Number(d.calibrated_steps_per_ml||0).toFixed(4));
setText('dxRaw',d.current_raw);setText('dxFiltered',Number(d.current_adc||0).toFixed(1));setText('dxBaseline',Number(d.current_baseline||0).toFixed(1));setText('dxDelta',Number(d.current_delta||0).toFixed(1));setText('dxThreshold',d.occlusion_delta_counts);setText('dxHold',d.occlusion_hold_ms+' ms');setText('dxGrace',d.occlusion_grace_ms+' ms');setText('dxLocked',d.occlusion_baseline_locked?'YES':'NO');
setText('dxHome',d.home_limit?'ACTIVE':'OPEN');setText('dxMax',d.max_limit?'ACTIVE':'OPEN');setText('dxHx',d.load_cell_ready?'YES':'NO');setText('dxGrams',Number(d.load_cell_grams||0).toFixed(2));setText('dxScale',d.hx711_scale_factor);
setText('dxIp',d.ip||'-');setText('dxRssi',d.wifi_connected?(d.rssi+' dBm'):'offline');setText('dxMqtt',d.mqtt_connected?'CONNECTED':'OFFLINE');setText('dxHeap',d.free_heap+' bytes');setText('dxMinHeap',d.min_free_heap+' bytes');setText('dxCpu',d.cpu_mhz+' MHz');setText('dxUptime',Number(d.uptime_s||0).toFixed(1)+' s');setText('dxMessage',d.message||d.last_event||'-');
}catch(e){setText('net','Dashboard disconnected | smartsyringe.local');setText('state','OFFLINE');$('state').className='state bad'}}
window.addEventListener('hashchange',initPage);initPage();renderAuditLog();renderStabilityRuns();loadSetup();refresh();setInterval(refresh,1000);
</script>
</body>
</html>
)HTML";
