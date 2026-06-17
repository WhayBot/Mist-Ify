#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>

static const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mist-Ify Controller</title>
<style>
:root{--bg:#0c1015;--card:rgba(255,255,255,0.04);--border:rgba(255,255,255,0.08);
--text:#e2e8f0;--muted:#94a3b8;--green:#4ade80;--blue:#60a5fa;--amber:#fbbf24;
--red:#f87171;--glow-g:rgba(74,222,128,0.15);--glow-b:rgba(96,165,250,0.15)}
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
background:var(--bg);color:var(--text);min-height:100vh;padding:16px}
header{text-align:center;padding:20px 0 10px}
header h1{font-size:1.5em;font-weight:700}
header h1 span{font-size:1.3em}
.badge{display:inline-block;padding:3px 10px;border-radius:20px;font-size:.7em;
margin-left:8px;vertical-align:middle}
.badge.on{background:var(--glow-g);color:var(--green)}
.badge.off{background:rgba(239,68,68,0.15);color:var(--red)}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));
gap:16px;max-width:1200px;margin:16px auto}
.card{background:var(--card);border:1px solid var(--border);border-radius:16px;
padding:20px;backdrop-filter:blur(12px);-webkit-backdrop-filter:blur(12px);
transition:border-color .3s}
.card:hover{border-color:rgba(255,255,255,0.15)}
.card h3{font-size:.85em;color:var(--muted);text-transform:uppercase;letter-spacing:1px;margin-bottom:12px}
.card.span2{grid-column:span 2}
@media(max-width:700px){.card.span2{grid-column:span 1}}
.big{font-size:2.8em;font-weight:700;line-height:1}
.big.temp{color:var(--blue)}
.big.hum{color:var(--green)}
.unit{font-size:.4em;color:var(--muted);font-weight:400}
.state-label{font-size:1.4em;font-weight:600;padding:6px 16px;border-radius:10px;display:inline-block}
.state-IDLE{background:var(--glow-g);color:var(--green)}
.state-PUMPING{background:var(--glow-b);color:var(--blue)}
.state-SOAKING{background:rgba(251,191,36,0.15);color:var(--amber)}
.state-MISTING{background:rgba(139,92,246,0.15);color:#a78bfa}
.state-ALARM{background:rgba(239,68,68,0.15);color:var(--red);animation:pulse 1s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}
.mode-tag{font-size:.75em;margin-top:8px;color:var(--muted)}
.gauge-wrap{position:relative;width:180px;height:100px;margin:0 auto 8px}
.gauge-wrap canvas{width:180px;height:100px}
.gauge-val{position:absolute;bottom:0;left:50%;transform:translateX(-50%);
font-size:1.8em;font-weight:700;color:var(--green)}
.ctrl-row{display:flex;align-items:center;justify-content:space-between;
padding:10px 0;border-bottom:1px solid var(--border)}
.ctrl-row:last-child{border-bottom:none}
.ctrl-label{font-size:.95em}
.ctrl-status{font-size:.8em;padding:3px 10px;border-radius:6px}
.ctrl-status.on{background:var(--glow-g);color:var(--green)}
.ctrl-status.off{background:rgba(255,255,255,0.06);color:var(--muted)}
.btn{padding:8px 18px;border:none;border-radius:10px;font-size:.85em;font-weight:600;
cursor:pointer;transition:.2s}
.btn:hover{transform:translateY(-1px)}
.btn-green{background:var(--green);color:#0c1015}
.btn-red{background:var(--red);color:#fff}
.btn-outline{background:transparent;border:1px solid var(--border);color:var(--muted)}
.btn-outline:hover{border-color:var(--green);color:var(--green)}
.btn-amber{background:var(--amber);color:#0c1015}
.btn-block{width:100%;margin-top:10px}
.btn:disabled{opacity:.4;cursor:not-allowed;transform:none}
.chart-container{position:relative;width:100%;height:200px}
.chart-container canvas{width:100%;height:200px}
.log-scroll{max-height:250px;overflow-y:auto;font-size:.82em}
.log-scroll::-webkit-scrollbar{width:4px}
.log-scroll::-webkit-scrollbar-thumb{background:var(--border);border-radius:2px}
.log-row{display:flex;gap:10px;padding:6px 0;border-bottom:1px solid var(--border)}
.log-time{color:var(--muted);white-space:nowrap;min-width:70px}
.log-msg{color:var(--text)}
.info-row{display:flex;justify-content:space-between;padding:5px 0;font-size:.88em;
border-bottom:1px solid var(--border)}
.info-row:last-child{border-bottom:none}
.info-k{color:var(--muted)}
.info-v{font-weight:500}
.ota-input{width:100%;padding:10px;border:2px dashed var(--border);border-radius:10px;
background:transparent;color:var(--text);margin-bottom:10px;cursor:pointer}
.ota-input:hover{border-color:var(--green)}
.progress-bar{height:6px;background:rgba(255,255,255,0.06);border-radius:3px;overflow:hidden;display:none}
.progress-fill{height:100%;background:var(--green);border-radius:3px;width:0%;transition:width .3s}
.ota-msg{font-size:.85em;margin-top:8px;text-align:center}
</style></head><body>
<header>
<h1>Mist-Ify Controller</h1>
<span class="badge" id="conn-badge">---</span>
</header>
<div class="grid">
<div class="card" id="temp-card">
<h3>Temperature</h3>
<div class="big temp" id="v-temp">--<span class="unit">°C</span></div>
</div>
<div class="card" id="hum-card">
<h3>Humidity</h3>
<div class="gauge-wrap"><canvas id="gauge" width="180" height="100"></canvas>
<div class="gauge-val" id="v-hum">--%</div></div>
<div style="text-align:center;font-size:.75em;color:var(--muted)">Target: 75% - 80%</div>
</div>
<div class="card" style="text-align:center">
<h3>System Status</h3>
<div class="state-label state-IDLE" id="v-state">IDLE</div>
<div class="mode-tag" id="v-mode">Mode: Auto</div>
</div>
<div class="card">
<h3>Manual Control</h3>
<div class="ctrl-row">
<span class="ctrl-label">Water Pump</span>
<span class="ctrl-status off" id="s-pump">OFF</span>
<button class="btn btn-outline" id="b-pump" onclick="api('pump')" disabled>Toggle</button>
</div>
<div class="ctrl-row">
<span class="ctrl-label">Mist Maker</span>
<span class="ctrl-status off" id="s-mist">OFF</span>
<button class="btn btn-outline" id="b-mist" onclick="api('mist')" disabled>Toggle</button>
</div>
<div style="display:flex;gap:8px;margin-top:12px">
<button class="btn btn-amber" style="flex:1" id="b-manual" onclick="api('manual')">Manual Mode</button>
<button class="btn btn-green" style="flex:1" id="b-auto" onclick="api('auto')">Auto Mode</button>
</div>
<button class="btn btn-red btn-block" id="b-alarm" onclick="api('alarm')" style="display:none">Reset Alarm</button>
</div>
<div class="card span2">
<h3>24 Hour Chart</h3>
<div class="chart-container"><canvas id="chart"></canvas></div>
</div>
<div class="card">
<h3>Event Log</h3>
<div class="log-scroll" id="events"><div style="color:var(--muted);text-align:center;padding:20px">No events yet</div></div>
</div>
<div class="card">
<h3>System Info</h3>
<div id="sysinfo">
<div class="info-row"><span class="info-k">IP Address</span><span class="info-v" id="i-ip">-</span></div>
<div class="info-row"><span class="info-k">WiFi RSSI</span><span class="info-v" id="i-rssi">-</span></div>
<div class="info-row"><span class="info-k">Uptime</span><span class="info-v" id="i-up">-</span></div>
<div class="info-row"><span class="info-k">Free Heap</span><span class="info-v" id="i-ram">-</span></div>
<div class="info-row"><span class="info-k">Firmware</span><span class="info-v" id="i-fw">-</span></div>
</div>
</div>
<div class="card">
<h3>Update Firmware (OTA)</h3>
<form id="ota-form" enctype="multipart/form-data">
<input type="file" accept=".bin" class="ota-input" id="ota-file">
<div class="progress-bar" id="ota-bar"><div class="progress-fill" id="ota-fill"></div></div>
<button type="submit" class="btn btn-green btn-block">Upload Firmware</button>
<div class="ota-msg" id="ota-msg"></div>
</form>
</div>
<div class="card" style="text-align:center">
<h3>WiFi</h3>
<p style="font-size:.85em;color:var(--muted);margin-bottom:12px">Reset WiFi to enter AP mode for setup</p>
<button class="btn btn-red" onclick="if(confirm('Reset WiFi settings?'))api('wifi-reset')">Reset WiFi Settings</button>
</div>
</div>
<script>
let chartData=[];
function $(id){return document.getElementById(id)}
function fmtTime(ts){
  if(!ts||ts<1000000)return'--:--';
  let d=new Date(ts*1000);
  return String(d.getHours()).padStart(2,'0')+':'+String(d.getMinutes()).padStart(2,'0');
}
function fmtUptime(s){
  let h=Math.floor(s/3600),m=Math.floor((s%3600)/60);
  return h+'h '+m+'m';
}
function drawGauge(val){
  let c=$('gauge'),ctx=c.getContext('2d');
  let W=180,H=100,cx=W/2,cy=H-5,r=75;
  ctx.clearRect(0,0,W,H);
  ctx.beginPath();ctx.arc(cx,cy,r,Math.PI,0);
  ctx.lineWidth=14;ctx.strokeStyle='rgba(255,255,255,0.06)';ctx.lineCap='round';ctx.stroke();
  let zones=[{s:0,e:60,c:'#ef4444'},{s:60,e:75,c:'#fbbf24'},{s:75,e:85,c:'#4ade80'},{s:85,e:100,c:'#60a5fa'}];
  zones.forEach(z=>{
    let a1=Math.PI+(z.s/100)*Math.PI,a2=Math.PI+(z.e/100)*Math.PI;
    ctx.beginPath();ctx.arc(cx,cy,r,a1,a2);
    ctx.lineWidth=14;ctx.strokeStyle=z.c;ctx.globalAlpha=0.25;ctx.lineCap='butt';ctx.stroke();
    ctx.globalAlpha=1;
  });
  if(val>=0){
    let ang=Math.PI+(Math.min(val,100)/100)*Math.PI;
    let col=val<60?'#ef4444':val<75?'#fbbf24':val<85?'#4ade80':'#60a5fa';
    ctx.beginPath();ctx.arc(cx,cy,r,Math.PI,ang);
    ctx.lineWidth=14;ctx.strokeStyle=col;ctx.lineCap='round';ctx.stroke();
    ctx.shadowColor=col;ctx.shadowBlur=15;
    ctx.beginPath();ctx.arc(cx,cy,r,ang-0.05,ang);
    ctx.lineWidth=14;ctx.strokeStyle=col;ctx.stroke();
    ctx.shadowBlur=0;
  }
  if(val>=0){
    let ang=Math.PI+(Math.min(val,100)/100)*Math.PI;
    let nx=cx+r*Math.cos(ang),ny=cy+r*Math.sin(ang);
    ctx.beginPath();ctx.arc(nx,ny,5,0,2*Math.PI);ctx.fillStyle='#fff';ctx.fill();
  }
}
function drawChart(data){
  let c=$('chart'),ctx=c.getContext('2d');
  let dpr=window.devicePixelRatio||1;
  let W=c.parentElement.clientWidth,H=200;
  c.width=W*dpr;c.height=H*dpr;
  c.style.width=W+'px';c.style.height=H+'px';
  ctx.scale(dpr,dpr);
  if(!data||data.length<2){
    ctx.fillStyle='#94a3b8';ctx.font='13px sans-serif';ctx.textAlign='center';
    ctx.fillText('Not enough data',W/2,H/2);return;
  }
  let pad={t:20,r:50,b:30,l:45};
  let gW=W-pad.l-pad.r,gH=H-pad.t-pad.b;
  let temps=data.map(d=>d.tp),hums=data.map(d=>d.hm);
  let tMin=Math.floor(Math.min(...temps))-2,tMax=Math.ceil(Math.max(...temps))+2;
  let hMin=Math.floor(Math.min(...hums))-5,hMax=Math.ceil(Math.max(...hums))+5;
  if(tMax-tMin<5){tMin-=3;tMax+=3}
  if(hMax-hMin<10){hMin-=5;hMax+=5}
  ctx.strokeStyle='rgba(255,255,255,0.05)';ctx.lineWidth=1;
  for(let i=0;i<=4;i++){
    let y=pad.t+gH*(i/4);
    ctx.beginPath();ctx.moveTo(pad.l,y);ctx.lineTo(pad.l+gW,y);ctx.stroke();
  }
  ctx.fillStyle='#60a5fa';ctx.font='11px sans-serif';ctx.textAlign='right';
  for(let i=0;i<=4;i++){
    let v=tMax-(i/4)*(tMax-tMin);
    ctx.fillText(v.toFixed(0)+'°',pad.l-6,pad.t+gH*(i/4)+4);
  }
  ctx.fillStyle='#4ade80';ctx.textAlign='left';
  for(let i=0;i<=4;i++){
    let v=hMax-(i/4)*(hMax-hMin);
    ctx.fillText(v.toFixed(0)+'%',pad.l+gW+6,pad.t+gH*(i/4)+4);
  }
  ctx.fillStyle='#94a3b8';ctx.textAlign='center';ctx.font='10px sans-serif';
  let step=Math.max(1,Math.floor(data.length/6));
  for(let i=0;i<data.length;i+=step){
    let x=pad.l+(i/(data.length-1))*gW;
    ctx.fillText(fmtTime(data[i].t),x,H-6);
  }
  function line(arr,min,max,color){
    ctx.beginPath();
    arr.forEach((v,i)=>{
      let x=pad.l+(i/(arr.length-1))*gW;
      let y=pad.t+gH*(1-(v-min)/(max-min));
      i===0?ctx.moveTo(x,y):ctx.lineTo(x,y);
    });
    ctx.strokeStyle=color;ctx.lineWidth=2;ctx.stroke();
    let last=arr.length-1;
    ctx.lineTo(pad.l+(last/(arr.length-1))*gW,pad.t+gH);
    ctx.lineTo(pad.l,pad.t+gH);ctx.closePath();
    let grd=ctx.createLinearGradient(0,pad.t,0,pad.t+gH);
    grd.addColorStop(0,color.replace(')',',0.2)').replace('rgb','rgba'));
    grd.addColorStop(1,color.replace(')',',0)').replace('rgb','rgba'));
    ctx.fillStyle=grd;ctx.fill();
  }
  line(temps,tMin,tMax,'rgb(96,165,250)');
  line(hums,hMin,hMax,'rgb(74,222,128)');
  [75,80].forEach(th=>{
    let y=pad.t+gH*(1-(th-hMin)/(hMax-hMin));
    if(y>pad.t&&y<pad.t+gH){
      ctx.setLineDash([4,4]);ctx.strokeStyle='rgba(74,222,128,0.3)';ctx.lineWidth=1;
      ctx.beginPath();ctx.moveTo(pad.l,y);ctx.lineTo(pad.l+gW,y);ctx.stroke();
      ctx.setLineDash([]);
      ctx.fillStyle='rgba(74,222,128,0.5)';ctx.font='9px sans-serif';ctx.textAlign='left';
      ctx.fillText(th+'%',pad.l+3,y-3);
    }
  });
}
function updateUI(d){
  $('v-temp').innerHTML=d.temp.toFixed(1)+'<span class="unit">°C</span>';
  $('v-hum').textContent=d.hum.toFixed(1)+'%';
  drawGauge(d.hum);
  let sl=$('v-state');
  sl.textContent=d.state;
  sl.className='state-label state-'+d.state;
  $('v-mode').textContent='Mode: '+(d.manual?'Manual':'Auto');
  let sp=$('s-pump'),sm=$('s-mist');
  sp.textContent=d.pump?'ON':'OFF';sp.className='ctrl-status '+(d.pump?'on':'off');
  sm.textContent=d.mist?'ON':'OFF';sm.className='ctrl-status '+(d.mist?'on':'off');
  $('b-pump').disabled=!d.manual;
  $('b-mist').disabled=!d.manual;
  $('b-alarm').style.display=d.state==='ALARM'?'block':'none';
  $('i-ip').textContent=d.ip;
  $('i-rssi').textContent=d.rssi+' dBm';
  $('i-up').textContent=fmtUptime(d.uptime);
  $('i-ram').textContent=Math.round(d.heap/1024)+' KB';
  $('i-fw').textContent=d.ver;
  let b=$('conn-badge');
  b.textContent='Online';b.className='badge on';
}
function fetchStatus(){
  fetch('/api/status').then(r=>r.json()).then(updateUI)
  .catch(()=>{
    let b=$('conn-badge');b.textContent='Offline';b.className='badge off';
  });
}
function fetchSensors(){
  fetch('/api/sensors').then(r=>r.json()).then(d=>{chartData=d;drawChart(d)}).catch(()=>{});
}
function fetchEvents(){
  fetch('/api/events').then(r=>r.json()).then(d=>{
    let el=$('events');
    if(!d.length){el.innerHTML='<div style="color:var(--muted);text-align:center;padding:20px">No events yet</div>';return}
    let html='';
    for(let i=d.length-1;i>=0;i--){
      html+='<div class="log-row"><span class="log-time">'+fmtTime(d[i].t)+'</span><span class="log-msg">'+d[i].m+'</span></div>';
    }
    el.innerHTML=html;
  }).catch(()=>{});
}
function api(action){
  fetch('/api/'+action,{method:'POST'}).then(()=>fetchStatus()).catch(()=>{});
}
$('ota-form').addEventListener('submit',function(e){
  e.preventDefault();
  let file=$('ota-file').files[0];
  if(!file){$('ota-msg').textContent='Please select a .bin file';return}
  let xhr=new XMLHttpRequest();
  let bar=$('ota-bar'),fill=$('ota-fill'),msg=$('ota-msg');
  bar.style.display='block';msg.textContent='Uploading...';
  xhr.upload.onprogress=function(ev){
    if(ev.lengthComputable)fill.style.width=Math.round(ev.loaded/ev.total*100)+'%';
  };
  xhr.onload=function(){
    if(xhr.status===200){msg.textContent='Update successful! Restarting...';msg.style.color='var(--green)'}
    else{msg.textContent='Update failed';msg.style.color='var(--red)'}
  };
  xhr.onerror=function(){msg.textContent='Upload error';msg.style.color='var(--red)'};
  let fd=new FormData();fd.append('firmware',file);
  xhr.open('POST','/api/ota');xhr.send(fd);
});
window.addEventListener('resize',()=>{if(chartData.length)drawChart(chartData)});
fetchStatus();fetchSensors();fetchEvents();
setInterval(fetchStatus,2000);
setInterval(fetchSensors,60000);
setInterval(fetchEvents,10000);
</script>
</body></html>
)rawliteral";

#endif
