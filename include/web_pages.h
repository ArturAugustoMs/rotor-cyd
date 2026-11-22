#pragma once
// HTML/CSS/JS do portal web. Servido diretamente da flash.

namespace web::pages {

static const char kCss[] = R"CSS(
:root{--bg:#08111f;--card:#0f1b2e;--card2:#13223a;--line:#22365a;--tx:#e9eef7;--dim:#8d9dba;--ac:#3ddc84;--pri:#2f9bff;--dg:#ff5a5a;--r:12px}
*{box-sizing:border-box}
html{background:var(--bg)}
body{background:linear-gradient(180deg,#0c1a31 0,var(--bg) 320px) no-repeat,var(--bg);color:var(--tx);font:16px/1.4 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;margin:0;-webkit-text-size-adjust:100%}
.wrap{max-width:560px;margin:0 auto;padding:12px 16px 90px}
.hd{display:flex;align-items:center;justify-content:space-between;padding:6px 0 10px;gap:10px}
.brand{display:flex;align-items:center;gap:10px;color:var(--tx);font-weight:700;font-size:1.15rem;text-decoration:none;min-width:0}
.brand:before{content:"";width:22px;height:22px;flex:none;border-radius:50%;border:2px solid var(--ac);background:conic-gradient(from 30deg,rgba(61,220,132,.85),transparent 30%)}
.brand span{overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.pill{padding:3px 10px;border-radius:20px;background:var(--card2);color:var(--ac);font-size:.78rem;font-weight:700;border:1px solid var(--line);white-space:nowrap}
.tabs{display:flex;background:var(--card);border:1px solid var(--line);border-radius:var(--r);padding:4px;gap:4px;margin-bottom:14px}
.tabs a{flex:1;text-align:center;padding:8px 4px;border-radius:9px;color:var(--dim);font-weight:600;font-size:.9rem;text-decoration:none}
.tabs a.on{background:var(--card2);color:var(--tx);box-shadow:inset 0 0 0 1px var(--line)}
.card,.pr-sec{background:var(--card);border:1px solid var(--line);border-radius:var(--r);padding:14px;margin:0 0 12px}
h2{font-size:1.02rem;margin:0 0 .7rem}
h3{margin:0 0 .3rem;font-size:1rem}
small,.hint{color:var(--dim);font-size:.85rem}
label{display:block;color:var(--dim);font-size:.85rem;margin:2px 0 4px}
input{background:#0a1426;border:1px solid var(--line);color:var(--tx);border-radius:10px;padding:11px 12px;font:inherit;width:100%;margin:0 0 10px}
input:focus{outline:2px solid var(--pri);border-color:transparent}
button{background:var(--pri);border:0;border-radius:10px;color:#fff;font:600 1rem/2.75rem system-ui,sans-serif;width:100%;cursor:pointer;display:block;text-align:center;text-decoration:none;padding:0 14px;margin:0}
button.G{background:transparent;box-shadow:inset 0 0 0 1px var(--line);color:var(--tx)}
button.D{background:var(--dg)}
button.sm{width:auto;line-height:2.1rem;font-size:.88rem;padding:0 12px}
button:disabled{opacity:.5}
a{color:var(--pri);font-weight:600;text-decoration:none}
.tiles{display:grid;gap:10px;margin:0 0 12px}
.tile{display:block;background:var(--card);border:1px solid var(--line);border-radius:var(--r);padding:14px;color:var(--tx);text-decoration:none}
.tile b{display:block;font-size:1rem}.tile small{display:block;margin-top:2px}
.tile:hover{border-color:var(--pri)}
.row{display:flex;gap:10px}.row>*{flex:1}
canvas{width:100%;max-width:520px;aspect-ratio:1;display:block;margin:0 auto;border-radius:var(--r)}
.arrows{display:flex;gap:12px;margin-top:12px;justify-content:center}
.arrows button{width:140px;max-width:45%;font-size:1.7rem;line-height:3.4rem;touch-action:none;user-select:none;-webkit-user-select:none}
.arrows button.on{background:var(--ac);color:#06121f}
.arrows button.cw.on{background:#ffb020}
.presets h3{margin:14px 0 8px;padding-top:12px;border-top:1px solid var(--line)}
.plist{display:flex;flex-direction:column;gap:6px;margin:0 0 10px}
.prow{display:flex;align-items:center;gap:6px;background:var(--card2);border:1px solid var(--line);border-radius:10px;padding:4px}
.prow.sel{border-color:var(--ac)}
.prow button{flex:none;width:auto;line-height:2rem;font-size:.85rem;padding:0 10px;margin:0}
.prow button.go{flex:1;background:transparent;color:var(--tx);text-align:left;display:flex;justify-content:space-between;gap:8px;box-shadow:none}
.prow button.go span{overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.prow button.go b{color:var(--ac);white-space:nowrap}
.nets{display:flex;flex-direction:column;gap:8px;margin:10px 0 0}
button.net{box-sizing:border-box;display:flex;align-items:center;gap:12px;width:100%;min-height:48px;margin:0;padding:8px 12px;background:var(--card2);box-shadow:inset 0 0 0 1px var(--line);border-radius:10px;color:var(--tx);text-align:left;font:600 .9rem/1.3 system-ui,sans-serif}
button.net:hover{box-shadow:inset 0 0 0 1px var(--pri)}
button.net:active{background:var(--card)}
.net .bars{display:flex;align-items:flex-end;gap:2px;width:20px;height:16px;flex:none}
.net .bars i{display:block;width:4px;border-radius:1px;background:var(--line)}
.net .bars i:nth-child(1){height:5px}
.net .bars i:nth-child(2){height:8px}
.net .bars i:nth-child(3){height:12px}
.net .bars i:nth-child(4){height:16px}
.net .bars i.on{background:var(--ac)}
.net .nm{flex:1;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.net svg{flex:none;display:block;color:var(--dim)}
.pw{position:relative}
.pw input{margin-bottom:10px;padding-right:52px}
.pw .eye{position:absolute;right:5px;top:5px;width:40px;height:34px;padding:0;margin:0;background:transparent;box-shadow:none;color:var(--dim);display:flex;align-items:center;justify-content:center}
.pw .eye svg{display:block;pointer-events:none}
details.about{background:var(--card);border:1px solid var(--line);border-radius:var(--r);margin:0 0 12px;overflow:hidden}
details.about summary{cursor:pointer;list-style:none;padding:13px 14px;font-weight:600;color:var(--tx);display:flex;align-items:center;gap:9px}
details.about summary::-webkit-details-marker{display:none}
details.about summary:before{content:"+";display:flex;align-items:center;justify-content:center;width:22px;height:22px;border-radius:7px;background:var(--card2);color:var(--ac);font-size:1.05rem}
details.about[open] summary:before{content:"\2212"}
.about-body{padding:0 14px 14px}
.about-brand{font-weight:800;letter-spacing:.14em;font-size:.8rem;color:var(--ac);margin:2px 0 10px}
.about-body p{display:flex;justify-content:space-between;align-items:center;gap:10px;margin:0 0 6px;padding:9px 12px;background:var(--card2);border-radius:9px}
.about-body p span{color:var(--dim);font-size:.83rem}
.about-body p b{font-size:.88rem;font-weight:600}
.palettes{display:flex;flex-wrap:wrap;gap:10px}
.pal{flex:1 1 120px;max-width:170px;background:var(--card2);border:2px solid var(--line);border-radius:10px;padding:9px;color:var(--tx);cursor:pointer;text-align:left;font:600 .85rem/1.3 system-ui,sans-serif}
.pal.on{border-color:var(--ac)}
.sw{display:flex;gap:3px;margin-bottom:7px}
.sw i{height:20px;flex:1;border-radius:4px}
.toast{position:fixed;left:50%;bottom:70px;transform:translateX(-50%);background:var(--card2);border:1px solid var(--ac);color:var(--tx);padding:10px 16px;border-radius:10px;opacity:0;transition:.3s;pointer-events:none;z-index:9}
.toast.on{opacity:1}
.wrap>form{margin:0 0 10px}
form[action='/wifi'] button,form[action='/info'] button,form[action='/update'] button,form[action='/restart'] button,form[action='/exit'] button,form[action='/erase'] button{background:transparent;box-shadow:inset 0 0 0 1px var(--line);color:var(--tx)}
a[data-ssid]{display:block;color:var(--tx);padding:12px;background:var(--card);border:1px solid var(--line);border-radius:10px;margin:0 0 6px;text-decoration:none}
.ft{margin:26px 0 4px;padding:18px 12px 10px;border-top:1px solid var(--line);text-align:center;color:var(--dim);font-size:.78rem;line-height:1.6}
.ft .ft-b{display:flex;align-items:center;justify-content:center;gap:8px;color:var(--tx);font-weight:700;letter-spacing:.14em;font-size:.82rem}
.ft .ft-b:before{content:"";width:11px;height:11px;flex:none;border-radius:50%;border:2px solid var(--ac);background:conic-gradient(from 30deg,rgba(61,220,132,.9),transparent 30%)}
.ft .ft-v{color:var(--ac);letter-spacing:0;font-weight:700}
.ft .ft-d{display:flex;justify-content:center;gap:6px;flex-wrap:wrap;margin-top:9px}
.ft .ft-d span{background:var(--card);border:1px solid var(--line);border-radius:20px;padding:3px 10px}
.ft .ft-d b{color:var(--tx);font-weight:600}
)CSS";

static const char kHead[] = R"HTML(<link rel="stylesheet" href="/pr.css"><meta name="theme-color" content="#08111f"><script>
document.addEventListener('DOMContentLoaded',function(){var w=document.querySelector('.wrap');if(!w||document.querySelector('.hd'))return;
var p=location.pathname,t=p.indexOf('/settings')==0?1:p.indexOf('/rede')==0?2:0;
var h=document.createElement('div');h.innerHTML="<header class='hd'><a class='brand' href='/'><span id='hname'>ROTOR ANTENA</span></a><span class='pill' id='hcall'>&hellip;</span></header><nav class='tabs'><a href='/'>Inicio</a><a href='/settings'>Configurar</a><a href='/rede'>Rede</a></nav>";
w.insertBefore(h,w.firstChild);h.querySelectorAll('nav a')[t].className='on';
var d=new Date(),f=document.createElement('footer');f.className='ft';
f.innerHTML="<div class='ft-b'>ROTOR CYD <span class='ft-v' id='ftVer'>&hellip;</span></div><div class='ft-d'><span>Indicativo <b id='ftCall'>&hellip;</b></span><span id='ftName'>&hellip;</span><span id='ftDate'></span><span id='ftYear'></span></div>";
w.appendChild(f);
document.getElementById('ftDate').textContent=d.toLocaleDateString('pt-BR');
document.getElementById('ftYear').textContent=d.getFullYear();
fetch('/api/state').then(function(r){return r.json()}).then(function(j){document.getElementById('hname').textContent=j.antenna;document.getElementById('hcall').textContent=j.callsign;if(j.version)document.getElementById('ftVer').textContent='v'+j.version;if(j.owner_callsign)document.getElementById('ftCall').textContent=j.owner_callsign;if(j.owner_name)document.getElementById('ftName').textContent=j.owner_name}).catch(function(){})});
</script>)HTML";

static const char kMenu[] = R"HTML(<div class="tiles">
<a class="tile" href="/settings"><b>Configurar</b><small>Nome, indicativo e paleta de cores da tela</small></a>
<a class="tile" href="/rede"><b>Rede local (Wi-Fi)</b><small>Escolher a rede que o ESP32 conecta e usa</small></a>
</div>
)HTML";

static const char kShellStart[] = R"HTML(<!DOCTYPE html><html lang="pt-br"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Rotor CYD</title>)HTML";

static const char kShellBody[] = R"HTML(</head><body><div class="wrap"><div class="toast" id="toast"></div><script>
function $(i){return document.getElementById(i)}
function toast(t){var e=$('toast');e.textContent=t;e.classList.add('on');clearTimeout(e._t);e._t=setTimeout(function(){e.classList.remove('on')},2200)}
function api(m,u,b){return fetch(u,{method:m,headers:b?{'Content-Type':'application/json'}:{},body:b?JSON.stringify(b):undefined}).then(function(r){return r.json().then(function(j){if(!r.ok)throw new Error(j.error||r.status);return j})})}
</script>)HTML";

static const char kShellEnd[] = R"HTML(</div></body></html>)HTML";

static const char kSettings[] = R"HTML(
<div class="card"><h2>Identifica&ccedil;&atilde;o</h2>
<p class="hint">Antena e indicativo aparecem no cabe&ccedil;alho. Os dados do propriet&aacute;rio aparecem no quadro &ldquo;Sobre&rdquo;.</p>
<label for="antenna">Nome da antena</label>
<input id="antenna" maxlength="24" placeholder="Ex.: ANTENA PRINCIPAL" autocomplete="off">
<label for="callsign">Indicativo</label>
<input id="callsign" maxlength="12" placeholder="Ex.: PY2ABC" autocomplete="off" style="text-transform:uppercase">
<label for="ownerCall">Indicativo (Sobre)</label>
<input id="ownerCall" maxlength="12" placeholder="Ex.: PY0XXX" autocomplete="off" style="text-transform:uppercase">
<label for="ownerName">Nome (Sobre)</label>
<input id="ownerName" maxlength="24" placeholder="Ex.: Seu Nome" autocomplete="off">
<button id="save">Salvar</button>
</div>
<div class="card"><h2>Apar&ecirc;ncia do CYD</h2>
<p class="hint">Escolha a paleta de cores da tela. A mudan&ccedil;a &eacute; aplicada na hora.</p>
<div class="palettes" id="palettes"></div>
</div>
<details class="about"><summary>Sobre</summary>
<div class="about-body">
<div class="about-brand">ROTOR CYD</div>
<p><span>Indicativo</span><b id="aboutCall">&mdash;</b></p>
<p><span>Nome</span><b id="aboutName">&mdash;</b></p>
<p><span>Data</span><b id="aboutDate">&mdash;</b></p>
<p><span>Ano</span><b id="aboutYear">&mdash;</b></p>
<p><span>Vers&atilde;o</span><b id="aboutVersion">&mdash;</b></p>
</div>
</details>
<script>
(function(){var d=new Date();$('aboutDate').textContent=d.toLocaleDateString('pt-BR');$('aboutYear').textContent=d.getFullYear()})();

var PALS=[
 {n:'Gelo / Teal',c:['#040a1c','#00c8ff','#2dd4bf']},
 {n:'Floresta',c:['#061410','#3cdc6e','#2dd4bf']},
 {n:'Solar',c:['#1c0e04','#ffb020','#ff783c']},
 {n:'Magenta',c:['#16061c','#ff50c8','#a06eff']},
 {n:'Grafite',c:['#0c0e12','#c8d7eb','#78b4dc']}
];
function renderPals(cur){
 var box=$('palettes');box.innerHTML='';
 PALS.forEach(function(p,i){
  var b=document.createElement('button');b.className='pal'+(i===cur?' on':'');
  b.innerHTML='<span class="sw"><i style="background:'+p.c[0]+'"></i><i style="background:'+p.c[1]+'"></i><i style="background:'+p.c[2]+'"></i></span>'+p.n;
  b.onclick=function(){api('POST','/api/palette',{palette:i}).then(function(){renderPals(i);toast('Paleta aplicada no CYD')}).catch(function(e){toast('Erro: '+e.message)})};
  box.appendChild(b);
 });
}
api('GET','/api/state').then(function(j){$('antenna').value=j.antenna;$('callsign').value=j.callsign;
 $('ownerCall').value=j.owner_callsign||'';$('ownerName').value=j.owner_name||'';
 if(j.version)$('aboutVersion').textContent='v'+j.version;
 if(j.owner_callsign)$('aboutCall').textContent=j.owner_callsign;
 if(j.owner_name)$('aboutName').textContent=j.owner_name;
 var n=Math.min(PALS.length,j.palette_count||PALS.length),cur=Math.max(0,Math.min(n-1,j.palette|0));
 PALS.length=n;renderPals(cur)}).catch(function(){renderPals(0)});
$('save').onclick=function(){var b={antenna:$('antenna').value.trim(),callsign:$('callsign').value.trim(),owner_callsign:$('ownerCall').value.trim(),owner_name:$('ownerName').value.trim()};
if(!b.antenna||!b.callsign){toast('Preencha nome da antena e indicativo');return}
api('POST','/api/settings',b).then(function(){toast('Salvo!');$('hname').textContent=b.antenna;$('hcall').textContent=b.callsign;if(b.owner_callsign)$('aboutCall').textContent=b.owner_callsign;if(b.owner_name)$('aboutName').textContent=b.owner_name}).catch(function(e){toast('Erro: '+e.message)})};
</script>
)HTML";

static const char kWifi[] = R"HTML(
<div class="card"><h2>Rede local (Wi-Fi)</h2>
<p class="hint" id="netinfo">Consultando...</p>
<label for="ssid">Nome da rede (SSID)</label>
<input id="ssid" maxlength="32" placeholder="Nome da sua rede" autocomplete="off">
<label for="pass">Senha da rede</label>
<div class="pw"><input id="pass" type="password" maxlength="64" placeholder="Senha do Wi-Fi" autocomplete="new-password"><button id="eye" class="eye" type="button" aria-label="Mostrar senha"><svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M2 12s3.6-6.5 10-6.5S22 12 22 12s-3.6 6.5-10 6.5S2 12 2 12z"/><circle cx="12" cy="12" r="2.8"/></svg></button></div>
<button id="connect">Conectar e salvar</button>
<p class="hint" style="margin:10px 0 0">A rede fica gravada no ESP32 e ser&aacute; usada automaticamente quando ele ligar.</p>
</div>
<div class="card"><h2>Redes dispon&iacute;veis</h2>
<p class="hint">Toque em uma rede para preencher o nome acima.</p>
<button id="rescan" class="G sm">Buscar novamente</button>
<div class="nets" id="nets"><p class="hint">Buscando...</p></div>
</div>
<script>
function esc(s){return String(s).replace(/[&<>"']/g,function(c){return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]})}
function netinfo(j){
 if(j.wifi_connected){$('netinfo').textContent='Conectado: '+j.wifi_ssid+'  -  IP '+j.wifi_ip}
 else{$('netinfo').textContent=j.wifi_ssid?('Rede salva: '+j.wifi_ssid+' (desconectado)'):'Nenhuma rede salva. Use o Access Point "Rotor-Setup".'}
}
var LOCK='<svg width="14" height="16" viewBox="0 0 16 18" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M4 8V5.5a4 4 0 0 1 8 0V8"/><rect x="2.5" y="8" width="11" height="8" rx="2"/></svg>';
function renderNets(list){
 var box=$('nets');box.innerHTML='';
 if(!list||!list.length){box.innerHTML='<p class="hint">Nenhuma rede encontrada.</p>';return}
 list.forEach(function(n){
  var bars=Math.max(1,Math.min(4,Math.round((n.rssi+92)/16)));
  var signal='<span class="bars">';
  for(var i=1;i<=4;i++) signal+='<i class="'+(i<=bars?'on':'')+'"></i>';
  signal+='</span>';
  var b=document.createElement('button');b.type='button';b.className='net';
  b.innerHTML=signal+'<span class="nm">'+esc(n.ssid)+'</span>'+(n.secure?LOCK:'');
  b.onclick=function(){$('ssid').value=n.ssid;if(n.secure)$('pass').focus();toast('Rede selecionada')};
  box.appendChild(b);
 });
}
function scan(force){
 $('nets').innerHTML='<p class="hint">Buscando redes...</p>';
 api('GET','/api/wifi/scan'+(force?'?force=1':'')).then(function(j){
  if(j.scanning){setTimeout(scan,800);return}
  renderNets(j.networks)
 }).catch(function(e){$('nets').innerHTML='<p class="hint">Erro: '+esc(e.message)+'</p>'});
}
api('GET','/api/state').then(function(j){netinfo(j);$('ssid').value=j.wifi_ssid||'';scan()}).catch(function(){scan()});
$('rescan').onclick=function(){scan(true)};
var EYE_ON='<svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M2 12s3.6-6.5 10-6.5S22 12 22 12s-3.6 6.5-10 6.5S2 12 2 12z"/><circle cx="12" cy="12" r="2.8"/><path d="M3 3l18 18"/></svg>';
var EYE_OFF='<svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M2 12s3.6-6.5 10-6.5S22 12 22 12s-3.6 6.5-10 6.5S2 12 2 12z"/><circle cx="12" cy="12" r="2.8"/></svg>';
$('eye').onclick=function(){var p=$('pass'),show=p.type==='password';p.type=show?'text':'password';
 this.innerHTML=show?EYE_ON:EYE_OFF;this.setAttribute('aria-label',show?'Ocultar senha':'Mostrar senha')};
$('connect').onclick=function(){
 var b={ssid:$('ssid').value.trim(),password:$('pass').value};
 if(!b.ssid){toast('Informe o nome da rede');return}
 api('POST','/api/wifi',b).then(function(){toast('Conectando...');var n=0,t=setInterval(function(){api('GET','/api/state').then(function(j){netinfo(j);if(j.wifi_connected||++n>10)clearInterval(t)})},2000)}).catch(function(e){toast('Erro: '+e.message)})
};
</script>
)HTML";

static const char kControl[] = R"HTML(
<div class="card">
<canvas id="compass" width="520" height="520"></canvas>
<div class="arrows">
<button id="ccw" aria-label="Girar no sentido anti-hor&aacute;rio">&#9664;</button>
<button id="cw" class="cw" aria-label="Girar no sentido hor&aacute;rio">&#9654;</button>
</div>
<p class="hint" style="margin:10px 0 0">Segure a seta para girar. O rel&eacute; desliga ao soltar (ou ap&oacute;s 1 s sem comando).</p>
<div class="presets">
<h3>Presets</h3>
<div id="presetList" class="plist"></div>
<p class="hint" id="targetStatus" style="margin:0 0 10px">Rota&ccedil;&atilde;o livre</p>
<div class="row">
<input id="presetName" maxlength="20" placeholder="Nome (ex.: Europa)">
<input id="presetAngle" type="number" min="0" max="360" step="1" inputmode="numeric" placeholder="&Acirc;ngulo 0-360">
</div>
<div class="row">
<button id="presetSave" class="sm">Adicionar</button>
<button id="presetCancel" class="G sm" style="display:none">Cancelar</button>
</div>
<p class="hint" style="margin:10px 0 0">O preset gira at&eacute; o &acirc;ngulo e para. Nunca cruza os fins de curso (0&deg;/360&deg;). Qualquer seta cancela o alvo.</p>
</div>
</div>
<script>
(function(){
var cv=$('compass'),c=cv.getContext('2d'),az=0,name='',call='';
function line(a,r1,r2,color,w){c.strokeStyle=color;c.lineWidth=w;c.beginPath();c.moveTo(260+Math.cos(a)*r1,260+Math.sin(a)*r1);c.lineTo(260+Math.cos(a)*r2,260+Math.sin(a)*r2);c.stroke()}
function draw(){
c.fillStyle='#0a1222';c.fillRect(0,0,520,520);
c.strokeStyle='#2ad4bf';c.lineWidth=5;c.beginPath();c.arc(260,260,210,0,6.2832);c.stroke();
c.strokeStyle='#7891ad';c.lineWidth=3;c.beginPath();c.arc(260,260,198,0,6.2832);c.stroke();
for(var a=0;a<360;a+=15){var r=(a-90)*Math.PI/180;line(r,a%45===0?180:188,203,a%90===0?'#e9eef7':'#7891ad',a%45===0?5:3)}
c.font='bold 44px system-ui,sans-serif';c.textAlign='center';c.textBaseline='middle';c.fillStyle='#3ddc84';
['N','E','S','W'].forEach(function(t,i){var r=(i*90-90)*Math.PI/180;c.fillText(t,260+Math.cos(r)*237,260+Math.sin(r)*237)});
var r=(az-90)*Math.PI/180,tx=260+Math.cos(r)*202,ty=260+Math.sin(r)*202,bx=260+Math.cos(r)*160,by=260+Math.sin(r)*160,px=-Math.sin(r)*16,py=Math.cos(r)*16;
c.fillStyle='#ff5a5a';c.beginPath();c.moveTo(tx,ty);c.lineTo(bx+px,by+py);c.lineTo(bx-px,by-py);c.closePath();c.fill();
c.font='bold 96px system-ui,sans-serif';c.fillStyle='#3ddc84';c.fillText(('00'+az).slice(-3),260,245);
c.font='600 24px system-ui,sans-serif';c.fillStyle='#8d9dba';c.fillText('GRAUS',260,300);
c.font='600 26px system-ui,sans-serif';c.fillStyle='#e9eef7';c.fillText(name,260,350);
c.font='600 22px system-ui,sans-serif';c.fillStyle='#3ddc84';c.fillText(call,260,385);
}
function poll(){api('GET','/api/state').then(function(j){az=j.azimuth;name=j.antenna;call=j.callsign;var ts=$('targetStatus');if(ts)ts.textContent=j.target_active?('Indo para '+j.target+'\u00b0'):'Rota\u00e7\u00e3o livre';draw()}).catch(function(){})}
setInterval(poll,200);poll();
function cmd(d){return api('POST','/api/rotate',{dir:d}).catch(function(){})}
function bind(id,d){
 var b=$(id),timer=null;
 function start(e){if(e&&e.preventDefault)e.preventDefault();if(timer!==null)return;
  cmd(d);b.classList.add('on');timer=setInterval(function(){cmd(d)},400)}
 function stop(){if(timer===null)return;clearInterval(timer);timer=null;
  b.classList.remove('on');cmd(0)}
 b._stop=stop;
 b.addEventListener('pointerdown',start);
 ['pointerup','pointercancel','pointerleave'].forEach(function(ev){b.addEventListener(ev,stop)});
}
bind('ccw',-1);bind('cw',1);
function stopAll(){['ccw','cw'].forEach(function(id){var b=$(id);if(b&&b._stop)b._stop()});cmd(0)}
window.addEventListener('blur',stopAll);
document.addEventListener('visibilitychange',function(){if(document.hidden)stopAll()});

var presets=[],editIndex=-1;
function renderPresets(){
 var box=$('presetList');box.innerHTML='';
 if(!presets.length){var empty=document.createElement('p');empty.className='hint';empty.textContent='Nenhum preset cadastrado.';box.appendChild(empty);return}
 presets.forEach(function(p,i){
  var row=document.createElement('div');row.className='prow'+(editIndex===i?' sel':'');
  var go=document.createElement('button');go.className='go';
  var nm=document.createElement('span');nm.textContent=p.name;
  var ag=document.createElement('b');ag.textContent=p.angle+'\u00b0';
  go.appendChild(nm);go.appendChild(ag);
  go.onclick=function(){stopAll();api('POST','/api/preset',{index:i}).then(function(){toast('Indo para '+p.name+' '+p.angle+'\u00b0')}).catch(function(e){toast('Erro: '+e.message)})};
  var ed=document.createElement('button');ed.className='G sm';ed.textContent='\u270e';
  ed.onclick=function(){editIndex=i;$('presetName').value=p.name;$('presetAngle').value=p.angle;$('presetSave').textContent='Salvar altera\u00e7\u00e3o';$('presetCancel').style.display='';renderPresets()};
  var del=document.createElement('button');del.className='D sm';del.textContent='\u00d7';
  del.onclick=function(){api('POST','/api/presets',{op:'remove',index:i}).then(function(){if(editIndex===i)resetPresetForm();loadPresets();toast('Preset removido')}).catch(function(e){toast('Erro: '+e.message)})};
  row.appendChild(go);row.appendChild(ed);row.appendChild(del);box.appendChild(row);
 });
}
function resetPresetForm(){editIndex=-1;$('presetName').value='';$('presetAngle').value='';$('presetSave').textContent='Adicionar';$('presetCancel').style.display='none';renderPresets()}
function loadPresets(){api('GET','/api/presets').then(function(j){presets=j.presets||[];renderPresets()}).catch(function(){})}
$('presetSave').onclick=function(){
 var name=$('presetName').value.trim(),angle=parseInt($('presetAngle').value,10);
 if(!name){toast('Informe o nome do preset');return}
 if(isNaN(angle)||angle<0||angle>360){toast('O \u00e2ngulo deve estar entre 0 e 360');return}
 var body=editIndex>=0?{op:'edit',index:editIndex,name:name,angle:angle}:{op:'add',name:name,angle:angle};
 api('POST','/api/presets',body).then(function(){resetPresetForm();loadPresets();toast('Preset salvo')}).catch(function(e){toast('Erro: '+e.message)});
};
$('presetCancel').onclick=resetPresetForm;
loadPresets();
})();
</script>
)HTML";

}  // namespace web::pages
