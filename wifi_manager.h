#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "config.h"

static const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mist-Ify WiFi Setup</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;
background:linear-gradient(135deg,#0c1015 0%,#1a2332 100%);
color:#e2e8f0;min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}
.card{background:rgba(255,255,255,0.05);border:1px solid rgba(255,255,255,0.1);
border-radius:20px;padding:40px 30px;max-width:400px;width:100%;
backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px)}
h1{text-align:center;font-size:1.6em;margin-bottom:6px}
.sub{text-align:center;color:#94a3b8;margin-bottom:24px;font-size:.9em}
label{display:block;font-size:.85em;color:#94a3b8;margin-bottom:6px;margin-top:16px}
select,input[type=password]{width:100%;padding:12px 14px;border-radius:10px;border:1px solid rgba(255,255,255,0.15);
background:rgba(255,255,255,0.08);color:#e2e8f0;font-size:1em;outline:none;transition:.2s}
select:focus,input:focus{border-color:#4ade80;box-shadow:0 0 0 3px rgba(74,222,128,0.15)}
button{width:100%;padding:14px;border:none;border-radius:12px;font-size:1em;font-weight:600;
cursor:pointer;margin-top:24px;transition:.2s}
.btn-scan{background:rgba(74,222,128,0.15);color:#4ade80;margin-top:0;margin-bottom:8px}
.btn-scan:hover{background:rgba(74,222,128,0.25)}
.btn-connect{background:linear-gradient(135deg,#22c55e,#16a34a);color:#fff}
.btn-connect:hover{transform:translateY(-1px);box-shadow:0 4px 15px rgba(34,197,94,0.4)}
.msg{text-align:center;margin-top:16px;padding:10px;border-radius:8px;font-size:.9em;display:none}
.msg.ok{display:block;background:rgba(74,222,128,0.15);color:#4ade80}
.msg.err{display:block;background:rgba(239,68,68,0.15);color:#f87171}
.loader{display:none;text-align:center;margin-top:16px;color:#94a3b8}
</style></head><body>
<div class="card">
<h1>Mist-Ify Setup</h1>
<p class="sub">Connect to your home WiFi network</p>
<button class="btn-scan" onclick="scan()">Scan WiFi Networks</button>
<label for="ssid">Select Network</label>
<select id="ssid"><option value="">-- Scan first --</option></select>
<label for="pass">WiFi Password</label>
<input type="password" id="pass" placeholder="Enter password">
<button class="btn-connect" onclick="save()">Connect</button>
<div class="loader" id="loader">Connecting...</div>
<div id="msg"></div>
</div>
<script>
function scan(){
  document.getElementById('loader').style.display='block';
  fetch('/wifi-scan').then(r=>r.json()).then(d=>{
    document.getElementById('loader').style.display='none';
    let s=document.getElementById('ssid');
    s.innerHTML='';
    d.forEach(n=>{let o=document.createElement('option');o.value=n.ssid;o.textContent=n.ssid+' ('+n.rssi+'dBm)';s.appendChild(o)});
  }).catch(()=>{showMsg('Scan failed','err');document.getElementById('loader').style.display='none'});
}
function save(){
  let ssid=document.getElementById('ssid').value;
  let pass=document.getElementById('pass').value;
  if(!ssid){showMsg('Please select a network','err');return}
  document.getElementById('loader').style.display='block';
  fetch('/wifi-save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
  body:'ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass)})
  .then(r=>r.json()).then(d=>{
    document.getElementById('loader').style.display='none';
    if(d.ok){showMsg('Saved! ESP32 will restart and connect to '+ssid,'ok');
    }else{showMsg('Failed to save','err')}
  }).catch(()=>{showMsg('Error','err');document.getElementById('loader').style.display='none'});
}
function showMsg(t,c){let m=document.getElementById('msg');m.className='msg '+c;m.textContent=t;m.style.display='block'}
</script></body></html>
)rawliteral";

class WiFiManager {
private:
  Preferences _prefs;
  bool _apMode = false;

public:
  bool begin() {
    _prefs.begin("wifi", false);
    String ssid = _prefs.getString("ssid", "");
    String pass = _prefs.getString("pass", "");

    if (ssid.length() == 0) {
      startAP();
      return false;
    }

    Serial.printf("[WiFi] Connecting to %s ...\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(OTA_HOSTNAME);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
      _apMode = false;
      return true;
    }

    Serial.println("[WiFi] Connection failed, starting AP...");
    startAP();
    return false;
  }

  void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    _apMode = true;
    Serial.printf("[WiFi] AP started: %s @ %s\n", AP_SSID, WiFi.softAPIP().toString().c_str());
  }

  void saveCredentials(const String& ssid, const String& pass) {
    _prefs.putString("ssid", ssid);
    _prefs.putString("pass", pass);
    Serial.printf("[WiFi] Credentials saved for: %s\n", ssid.c_str());
  }

  void clearCredentials() {
    _prefs.remove("ssid");
    _prefs.remove("pass");
    Serial.println("[WiFi] Credentials cleared");
  }

  String scanNetworksJSON() {
    int n = WiFi.scanNetworks();
    String j = "[";
    for (int i = 0; i < n; i++) {
      if (i > 0) j += ",";
      j += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    j += "]";
    WiFi.scanDelete();
    return j;
  }

  bool isAPMode()    { return _apMode; }
  bool isConnected() { return WiFi.status() == WL_CONNECTED; }
  String getIP()     { return _apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString(); }
  String getSSID()   { return WiFi.SSID(); }
  int getRSSI()      { return WiFi.RSSI(); }

  const char* getSetupHTML() { return SETUP_HTML; }
};

#endif
