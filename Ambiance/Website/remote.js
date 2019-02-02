var title = "Ambiance\u00AE";
var x = 4;

var classicBtns = [
    ["#ffffff", "font-size:2em;", "\u21D1", "up=rgb"],
    ["#ffffff", "font-size:2em;", "\u21D3", "down=rgb"],
    ["#000000", "color:#ffffff; font-size:2em;", "\u22B3", "play"],
    ["#ff0000", "color:#ffffff; font-size:2em;", "\u233D", "power"],
    ["#ff0000", "font-size:2em;", "R", "preset=1"],
    ["#00ff00", "font-size:2em;", "G", "preset=2"],
    ["#0000ff", "font-size:2em;", "B", "preset=3"],
    ["#ffffff", "font-size:2em;", "W", "preset=4"],
    ["#ff5500", "", "", "preset=5"],
    ["#00ff55", "", "", "preset=6"],
    ["#5500ff", "", "", "preset=7"],
    ["#e78bcc", "", "", "preset=8"],
    ["#ffaa00", "", "", "preset=9"],
    ["#00ffaa", "", "", "preset=10"],
    ["#aa00ff", "", "", "preset=11"],
    ["#f0ffe0", "", "", "preset=12"],
    ["#ffff00", "", "", "preset=13"],
    ["#00ffff", "", "", "preset=14"],
    ["#ff00ff", "", "", "preset=15"],
    ["#78d7d7", "", "", "preset=16"],
    ["#aaff00", "", "", "preset=17"],
    ["#00aaff", "", "", "preset=18"],
    ["#ff00aa", "", "", "preset=19"],
    ["#aaf5f5", "", "", "preset=20"],
    ["#ffffff", "color:#ff0000; font-size:2em;", "\u2191", "up=r"],
    ["#ffffff", "color:#00ff00; font-size:2em;", "\u2191", "up=g"],
    ["#ffffff", "color:#0000ff; font-size:2em;", "\u2191", "up=b"],
    ["#ffffff", "", "QUICK", "up=s"],
    ["#ffffff", "color:#ff0000; font-size:2em;", "\u2193", "down=r"],
    ["#ffffff", "color:#00ff00; font-size:2em;", "\u2193", "down=g"],
    ["#ffffff", "color:#0000ff; font-size:2em;", "\u2193", "down=b"],
    ["#ffffff", "", "SLOW", "down=s"],
    ["#ffffff", "", "DIY1", "diy=1"],
    ["#ffffff", "", "DIY2", "diy=2"],
    ["#ffffff", "", "DIY3", "diy=3"],
    ["#ffffff", "", "AUTO", "auto"],
    ["#ffffff", "", "DIY4", "diy=4"],
    ["#ffffff", "", "DIY5", "diy=5"],
    ["#ffffff", "", "DIY6", "diy=6"],
    ["#ffffff", "", "FLASH", "flash"],
    ["#ffffff", "", "JUMP3", "jump=3"],
    ["#ffffff", "", "JUMP7", "jump=7"],
    ["#ffffff", "", "FADE3", "fade=3"],
    ["#ffffff", "", "FADE7", "fade=7"]
];
var classic;

var modernBtns = [
    ["#000000", "color:#ffffff;", "WiFi", "diy=6"],
    ["#222222", "", "", "ignore"],
    ["#000000", "color:#ffffff; font-size:2em;", "\u22B3", "play"],
    ["#ff0000", "color:#ffffff; font-size:2em;", "\u233D", "power"],
    ["#ffffff", "", "Fire", "diy=1"],
    ["#ffffff", "", "Sunset", "diy=2"],
    ["#ffffff", "", "Kerst", "diy=4"],
    ["#ffffff", "", "Quick", "up=s"],
    ["#ffffff", "", "Auto", "auto"],
    ["#ffffff", "", "Fade", "fade=3"],
    ["#ffffff", "", "Flash", "flash"],
    ["#ffffff", "", "Slow", "down=s"],
    ["#ff0000", "font-size:2em;", "R", "preset=1"],
    ["#00ff00", "font-size:2em;", "G", "preset=2"],
    ["#0000ff", "font-size:2em;", "B", "preset=3"],
    ["#ffffff", "font-size:2em;", "W", "preset=4"]
];
var modern;


window.onload = function(){
    document.title = title;
    httpGet("color");
    // switchRemote("classic");
    switchRemote("modern");
};


function createClassicRemote () {
    var row;
    var btn;
    
    classic = document.createDocumentFragment();
    
    row = document.createElement("div");
    row.className = "header";
    row.textContent = title;
    classic.appendChild(row);
    
    for (var i = 0; i < classicBtns.length; i++) {
        if (i % x == 0) {
            classic.appendChild(row);
            row = document.createElement("div");
            row.className = "button-row";
        }
        btn = document.createElement("div");
        btn.id = i;
        btn.className = "button noselect";
        btn.style.cssText = classicBtns[i][1];
        btn.style.backgroundColor = classicBtns[i][0];
        btn.textContent = classicBtns[i][2];
        btn.onclick = function() {classicBtnAction(this);};
        row.appendChild(btn);
    }
    classic.appendChild(row);
    
    row = document.createElement("div");
    row.className = "footer";
    classic.appendChild(row);
}


function classicBtnAction (btn) {
    httpGet(classicBtns[btn.id][3]);
};


function createModernRemote () {
    var row;
    var btn;
    var wheel;
    var colorPicker;
    
    modern = document.createDocumentFragment();
    
    row = document.createElement("div");
    row.className = "header";
    row.textContent = title;
    modern.appendChild(row);
    
    for (var i = 0; i < modernBtns.length; i++) {
        if (i % x == 0) {
            modern.appendChild(row);
            row = document.createElement("div");
            row.className = "button-row";
        }
        btn = document.createElement("div");
        btn.id = i;
        btn.className = "button noselect";
        btn.style.cssText = modernBtns[i][1];
        btn.style.backgroundColor = modernBtns[i][0];
        btn.textContent = modernBtns[i][2];
        btn.onclick = function() {modernBtnAction(this);};
        row.appendChild(btn);
    }
    modern.appendChild(row);
    
    wheel = document.createElement("div");
    wheel.id = "colorwheel";
    wheel.className = "button-row-wheel";
    modern.appendChild(wheel);
    
    row = document.createElement("div");
    row.className = "footer";
    modern.appendChild(row);
}


function modernBtnAction (btn) {
    httpGet(modernBtns[btn.id][3]);
};


function switchRemote (type) {
    var switchBtn = document.getElementById("remote-switch");
    var remote = document.getElementById("remote");
    if (remote.hasChildNodes()) {
        remote.innerHTML = '';
    }
    if (type == "classic") {
        createClassicRemote();
        remote.appendChild(classic);
        switchBtn.textContent = "[Switch to modern remote]";
        switchBtn.onclick = function() {switchRemote("modern");};
    }
    if (type == "modern") {
        createModernRemote();
        remote.appendChild(modern);
        colorPicker = new iro.ColorPicker("#colorwheel", {});
        colorPicker.on("color:change", function(color, changes) {
            httpGet("rgb="+color.rgb.r+","+color.rgb.g+","+color.rgb.b);
        });
        switchBtn.textContent = "[Switch to classic remote]";
        switchBtn.onclick = function() {switchRemote("classic");};
    }
};


function httpGet (theReq) {
    var theUrl = "/?" + theReq + "&version=" + Math.random();
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (xhr.readyState == XMLHttpRequest.DONE) {
            if (xhr.responseText.startsWith("rgb")) {
                document.body.style.backgroundColor = xhr.responseText;
            }
        }
    }
    xhr.open("GET", theUrl, false);
    xhr.send();
}



/*!
 * iro.js
 * ----------------
 * Author: James Daniel (github.com/jaames | rakujira.jp)
 * Last updated: Fri Aug 11 2017
 */
var iro=function(t){function e(n){if(r[n])return r[n].exports;var s=r[n]={i:n,l:!1,exports:{}};return t[n].call(s.exports,s,s.exports,e),s.l=!0,s.exports}var r={};return e.m=t,e.c=r,e.i=function(t){return t},e.d=function(t,r,n){e.o(t,r)||Object.defineProperty(t,r,{configurable:!1,enumerable:!0,get:n})},e.n=function(t){var r=t&&t.t?function(){return t.default}:function(){return t};return e.d(r,"a",r),r},e.o=function(t,e){return Object.prototype.hasOwnProperty.call(t,e)},e.p="/test",e(e.s=11)}([function(t,e,r){"use strict";var n=Math.round;t.exports={name:"rgb",e:function(t){var e,r,s,i,a,o,u,c,h=t.h/360,f=t.s/100,l=t.v/100;switch(i=Math.floor(6*h),a=6*h-i,o=l*(1-f),u=l*(1-a*f),c=l*(1-(1-a)*f),i%6){case 0:e=l,r=c,s=o;break;case 1:e=u,r=l,s=o;break;case 2:e=o,r=l,s=c;break;case 3:e=o,r=u,s=l;break;case 4:e=c,r=o,s=l;break;case 5:e=l,r=o,s=u}return{r:n(255*e),g:n(255*r),b:n(255*s)}},u:function(t){var e,r=t.r/255,s=t.g/255,i=t.b/255,a=Math.max(r,s,i),o=Math.min(r,s,i),u=a-o;switch(a){case o:e=0;break;case r:e=(s-i)/u+(s<i?6:0);break;case s:e=(i-r)/u+2;break;case i:e=(r-s)/u+4}return e/=6,{h:n(360*e),s:n(0===a?0:u/a*100),v:n(100*a)}}}},function(t,e,r){"use strict";function n(t){return t&&t.t?t:{default:t}}var s=r(3),i=n(s),a=r(0),o=n(a),u=r(4),c=n(u),h=r(10),f=n(h),l=r(9),d=n(l),v=[i.default,o.default,c.default,f.default,d.default],p=function t(e){var r=this;if(!(this instanceof t))return new t(e);this.f=!1,this._={h:void 0,s:void 0,v:void 0},this.register("hsv",{get:this.get,set:this.set}),v.forEach(function(t){r.register(t.name,{set:function(e){this.hsv=t.u(e)},get:function(){return t.e(this.hsv)}})}),e&&this.fromString(e)};p.prototype={register:function(t,e){Object.defineProperty(this,t,e)},watch:function(t,e){this.f=t,e&&this.forceUpdate()},unwatch:function(){this.watch(!1)},forceUpdate:function(){var t=this._;this.f(t,t,{h:!0,s:!0,v:!0})},set:function(t){var e={},r=this._;for(var n in r)t.hasOwnProperty(n)||(t[n]=r[n]),e[n]=!(t[n]==r[n]);this._=t;var s=this.f;(e.h||e.s||e.v)&&"function"==typeof s&&s(t,r,e)},get:function(){return this._},fromString:function(t){/^rgb/.test(t)?this.rgbString=t:/^hsl/.test(t)?this.hslString=t:/^#[0-9A-Fa-f]/.test(t)&&(this.hexString=t)}},t.exports=p},function(t,e,r){"use strict";var n=r(7),s=function(t){return t&&t.t?t:{default:t}}(n),i=document,a=function(t){var e=s.default.create("style");s.default.M(e,i.createTextNode("")),s.default.M(i.head,e),this.style=e;var r=e.sheet;this.sheet=r,this.rules=r.rules||r.cssRules,this.map={}};a.prototype={on:function(){this.sheet.disabled=!1},off:function(){this.sheet.disabled=!0},setRule:function(t,e,r){var n=this.sheet,s=n.rules||n.cssRules,i=this.map;if(e=e.replace(/([A-Z])/g,function(t){return"-"+t.toLowerCase()}),i.hasOwnProperty(t))i[t].setProperty(e,r);else{var a=s.length,o=e+": "+r;try{n.insertRule(t+" {"+o+";}",a)}catch(e){n.addRule(t,o,a)}finally{s=n.rules||n.cssRules,i[t]=s[a].style}}},getCss:function(){var t=this.map,e={};for(var r in t){var n=t[r];e[r]={};for(var s=0;s<n.length;s++){var i=n[s];e[r][i]=n.getPropertyValue(i)}}return e},getCssText:function(){var t=this.map,e=[];for(var r in t)e.push(r.replace(/,\W/g,",\n")+" {\n\t"+t[r].cssText.replace(/;\W/g,";\n\t")+"\n}");return e.join("\n")}},t.exports=a},function(t,e,r){"use strict";var n=Math.round;t.exports={name:"hsl",e:function(t){var e=t.s/100,r=t.v/100,s=(2-e)*r;return e=0==e?0:e*r/(s<1?s:2-s),{h:t.h,s:n(100*e),l:n(50*s)}},u:function(t){var e=t.s/50,r=t.l/100;return e*=r<=1?r:2-r,{h:t.h,s:n(2*e/(r+e)*100),v:n(100*(r+e))}}}},function(t,e,r){"use strict";var n=r(3),s=function(t){return t&&t.t?t:{default:t}}(n);t.exports={name:"hslString",e:function(t){var e=s.default.e(t);return"hsl"+(e.a?"a":"")+"("+e.h+", "+e.s+"%, "+e.l+"%"+(e.a?", "+e.a:"")+")"},u:function(t){var e=t.match(/(hsla?)\((\d+)(?:\D+?)(\d+)(?:\D+?)(\d+)(?:\D+?)?([0-9\.]+?)?\)/i);return s.default.u({h:parseInt(e[2]),s:parseInt(e[3]),l:parseInt(e[4])})}}},function(t,e,r){"use strict";function n(t,e){for(stop in e)t.addColorStop(stop,e[stop]);return t}t.exports={k:function(t,e,r,s,i,a){return n(t.createLinearGradient(e,r,s,r),a)},H:function(t,e,r,s,i,a){return n(t.createRadialGradient(e,r,s,e,r,i),a)}}},function(t,e,r){"use strict";var n=function(t,e){this.opts=e,this.S=t,this.T=!1};n.prototype={P:function(t,e,r,n){var s=this.S;s.lineWidth=n,s.beginPath(),s.strokeStyle=r,s.arc(t,e,this.opts.r,0,2*Math.PI),s.stroke()},move:function(t,e){var r=this.T,n=this.opts.r+4;r&&this.S.clearRect(r.x-n,r.y-n,2*n,2*n),this.P(t,e,"#333",4),this.P(t,e,"#fff",2),this.T={x:t,y:e}}},t.exports=n},function(t,e,r){"use strict";function n(t,e){t="string"==typeof t?[t]:t,t.forEach(e)}var s=document;t.exports={$:function(t){return s.querySelector(t)},create:function(t){return s.createElement(t)},M:function(t,e){return t.appendChild(e)},C:function(t,e){return t.getAttribute(e)},R:function(t,e,r){n(e,function(e){t.addEventListener(e,r)})},I:function(t,e,r){n(e,function(e){t.removeEventListener(e,r)})},D:function(t){var e=this;"complete"==s.readyState?t():e.R(s,"readystatechange",function r(n){"complete"==s.readyState&&(t(),e.I(s,"readystatechange",r))})}}},function(t,e,r){"use strict";function n(t){return t&&t.t?t:{default:t}}var s=r(13),i=n(s),a=r(12),o=n(a),u=r(7),c=n(u),h=r(1),f=n(h),l=r(2),d=n(l),v=!1;c.default.R(document,["mousemove","touchmove"],function(t){v&&v.W(t)}),c.default.R(document,["mouseup","touchend"],function(t){v&&(t.preventDefault(),v.emit("input:end"),v.L=!1,v=!1)});var p=function t(e,r){if(!(this instanceof t))return new t(e,r);this.O={},this.L=!1,this.f=!1,this.stylesheet=new d.default,this.css=r.css||r.styles||void 0,this.color=new f.default(r.color||"#fff"),c.default.D(function(){e="string"==typeof e?c.default.$(e):e,e.style.cssText+="position:relative";var t=r.width||parseInt(c.default.C(e,"width"))||320,n=r.height||parseInt(c.default.C(e,"height"))||320,s=devicePixelRatio||1,a=t*s,u=n*s,h={};["main","over"].forEach(function(r,i){var o=c.default.M(e,c.default.create("canvas")),f=o.getContext("2d"),l=o.style;o.width=a,o.height=u,l.cssText+="width:"+t+"px;height:"+n+"px",f.scale(s,s),0!=i&&(l.cssText+="position:absolute;top:0;left:0"),h[r]={ctx:f,canvas:o}}),this.el=e,this.layers=h;var f=r.padding+2||6,l=r.borderWidth||0,d=r.markerRadius||8,v=r.sliderMargin||24,p=r.sliderHeight||2*d+2*f+2*l,g=Math.min(n-p-v,t),m=g/2-l,_=(t-g)/2,x={r:d},y={w:l,color:r.borderColor||"#fff"};this.ui=[new i.default(h,{cX:_+g/2,cY:g/2,r:m,rMax:m-(d+f),marker:x,border:y}),new o.default(h,{sliderType:"v",x:_+l,y:g+v,w:g-2*l,h:p-2*l,r:p/2-l,marker:x,border:y})],this.color.watch(this.X.bind(this),!0),c.default.R(e,["mousedown","touchstart"],this.Y.bind(this))}.bind(this))};p.prototype={watch:function(t,e){this.on("color:change",t),this.f=t,e&&t(this.color)},unwatch:function(){this.off("color:change",this.f)},on:function(t,e){var r=this.O;(r[t]||(r[t]=[])).push(e)},off:function(t,e){var r=this.O;r[t]&&r[t].splice(r[t].indexOf(e),1)},emit:function(t,e){var r=this.O;(r[t]||[]).map(function(t){t(e)}),(r["*"]||[]).map(function(t){t(e)})},j:function(t){t.preventDefault();var e=t.touches?t.changedTouches[0]:t,r=this.layers.main.canvas.getBoundingClientRect();return{x:e.clientX-r.left,y:e.clientY-r.top}},A:function(t,e){this.color.set(this.L.input(t,e))},Y:function(t){var e=this,r=this.j(t),n=r.x,s=r.y;this.ui.forEach(function(t){t.checkHit(n,s)&&(v=e,e.L=t,e.emit("input:start"),e.A(n,s))})},W:function(t){if(this==v){var e=this.j(t);this.A(e.x,e.y)}},X:function(t,e,r){var n=this.color,s=n.rgbString,i=this.css;this.ui.forEach(function(t){t.update(n,r)});for(var a in i){var o=i[a];for(var u in o)this.stylesheet.setRule(a,u,s)}this.emit("color:change",n)}},t.exports=p},function(t,e,r){"use strict";var n=r(0),s=function(t){return t&&t.t?t:{default:t}}(n);t.exports={name:"hexString",e:function(t){var e=s.default.e(t),r=e.r,n=e.g,i=e.b,a=r%17==0&&n%17==0&&i%17==0,o=a?17:1,u=a?4:8,c=a?4:7,h=r/o<<2*u|n/o<<u|i/o,f=h.toString(16);return"#"+new Array(c-f.length).join("0")+f},u:function(t){t=t.replace(/#/g,"");var e=parseInt("0x"+t),r=3==t.length,n=r?15:255,i=r?4:8,a=r?17:1;return s.default.u({r:(e>>2*i&n)*a,g:(e>>i&n)*a,b:(e&n)*a})}}},function(t,e,r){"use strict";var n=r(0),s=function(t){return t&&t.t?t:{default:t}}(n);t.exports={name:"rgbString",e:function(t){var e=s.default.e(t);return"rgb"+(e.a?"a":"")+"("+e.r+", "+e.g+", "+e.b+(e.a?", "+e.a:"")+")"},u:function(t){var e=t.match(/(rgba?)\((\d+)(?:\D+?)(\d+)(?:\D+?)(\d+)(?:\D+?)?([0-9\.]+?)?\)/i);return s.default.u({r:parseInt(e[2]),g:parseInt(e[3]),b:parseInt(e[4])})}}},function(t,e,r){"use strict";function n(t){return t&&t.t?t:{default:t}}var s=r(8),i=n(s),a=r(1),o=n(a),u=r(2),c=n(u);t.exports={Color:o.default,ColorPicker:i.default,Stylesheet:c.default,ColorWheel:i.default}},function(t,e,r){"use strict";function n(t){return t&&t.t?t:{default:t}}var s=r(5),i=n(s),a=r(6),o=n(a),u=r(4),c=n(u),h=function(t,e){this.S=t.main.ctx,e.x1=e.x,e.y1=e.y,e.x2=e.x+e.w,e.y2=e.y+e.h,e.range={min:e.x+e.r,max:e.x2-e.r,w:e.w-2*e.r},e.sliderType=e.sliderType||"v",this.type="slider",this.marker=new o.default(t.over.ctx,e.marker),this.q=e};h.prototype={draw:function(t){var e=this.S,r=this.q,n=r.x1,s=r.y1,a=r.x2,o=r.y2,u=r.w,h=r.h,f=r.r,l=r.border,d=l.w;e.clearRect(n-d,s-d,u+2*d,h+2*d),e.beginPath(),e.moveTo(n+f,s),e.arcTo(a,s,a,o,f),e.arcTo(a,o,n,o,f),e.arcTo(n,o,n,s,f),e.arcTo(n,s,a,s,f),e.closePath();var v;"v"==r.sliderType&&(v=i.default.k(e,n,s,a,o,{0:"#000",1:c.default.e({h:t.h,s:t.s,v:100})})),d&&(e.strokeStyle=l.color,e.lineWidth=2*d,e.stroke()),e.fillStyle=v,e.fill()},update:function(t,e){var r=this.q,n=r.range,s=t.hsv;if("v"==r.sliderType&&((e.h||e.s)&&this.draw(s),e.v)){var i=s.v/100;this.marker.move(n.min+i*n.w,r.y1+r.h/2)}},input:function(t,e){var r=this.q,n=r.range,s=Math.max(Math.min(t,n.max),n.min)-n.min;return{v:Math.round(100/n.w*s)}},checkHit:function(t,e){var r=this.q;return t>r.x1&&t<r.x2&&e>r.y1&&e<r.y2}},t.exports=h},function(t,e,r){"use strict";function n(t){return t&&t.t?t:{default:t}}var s=r(5),i=n(s),a=r(6),o=n(a),u=Math.PI,c=Math.pow,h=Math.sqrt,f=Math.abs,l=Math.round,d=function(t,e){this.S=t.main.ctx,this.q=e,this.type="wheel",this.marker=new o.default(t.over.ctx,e.marker)};d.prototype={draw:function(t){var e=this.S,r=this.q,n=r.cX,s=r.cY,a=r.border,o=a.w,c=r.r;e.clearRect(n-c-o,s-c-o,2*(c+o),2*(c+o)),o&&(e.lineWidth=c+2*o,e.strokeStyle=a.color,e.beginPath(),e.arc(n,s,c/2,0,2*u),e.stroke()),e.lineWidth=c;for(var h=2*u/360,f=0,l=0;f<360;f++,l+=h)e.strokeStyle="hsl("+f+",100%,"+t/2+"%)",e.beginPath(),e.arc(n,s,c/2,l,l+h+.04),e.stroke();var d="hsla(0,0%,"+t+"%,";e.fillStyle=i.default.H(e,n,s,0,r.rMax,{0:d+"1)",1:d+"0)"}),e.fillRect(n-c,s-c,2*c,2*c)},update:function(t,e){var r=this.q,n=t.hsv;if(e.v&&this.draw(n.v),e.h||e.s){var s=n.h*(u/180),i=n.s/100*r.rMax;this.marker.move(r.cX+i*Math.cos(s),r.cY+i*Math.sin(s))}},input:function(t,e){var r=this.q,n=r.cX,s=r.cY,i=(r.r,r.rMax),a=Math.atan2(t-n,e-s),o=360-(l(a*(180/u))+270)%360,f=Math.min(h(c(n-t,2)+c(s-e,2)),i);return{h:o,s:l(100/i*f)}},checkHit:function(t,e){var r=this.q,n=f(t-r.cX),s=f(e-r.cY);return h(n*n+s*s)<r.r}},t.exports=d}]);
//# sourceMappingURL=iro.min.js.map

