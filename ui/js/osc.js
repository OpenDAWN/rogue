var PI = 3.14159265359;
var M_PI = PI;
var TWO_PI = 2 * PI;

function tanh (arg) {
  return (Math.exp(arg) - Math.exp(-arg)) / (Math.exp(arg) + Math.exp(-arg));
}

function sin2(x) { 
    return Math.sin(TWO_PI * x);     
}

function cos2(x) {
  return Math.cos(TWO_PI * x);
}

// virtual analog

// based on csound atone
// http://sourceforge.net/p/csound/csound5-git/ci/master/tree/OOps/ugens5.c
var highpass = function() {
  var prev = 0;
  
  return function(x, f) {
    var b = 2.0 - Math.cos(2.0 * PI * f);
    var c2 = b - Math.sqrt(b*b - 1.0);
    
    var y = c2 * (prev + x)
    prev = y - x
    return y;
  }
}

var hp1 = highpass();
var hp2 = highpass();
var hp3 = highpass();

function va_saw(x, w, t) {
  var y = gb(x);
  return hp1(y, t);
}

function va_tri(x, w, t) {
  var y = gb(tri(x, w));
  return hp2(y, t);
}

function va_pulse(x, w, t) {
  var y = gb(gpulse(x, w));
  return hp3(y, t);
}

// phase distortion
// based on https://github.com/smbolton/whysynth
// based on http://www.amazona.de/workshop-casio-czvz-und-die-grundlagen-der-phase-distortion-synthesis/3/

function pd(x, w) {
  if (x < w) {
    return 0.5 * x / w;    
  } else {
    return 0.5 + 0.5 * (x-w) / (1.0 - w); 
  }  
}

function pd_saw(x, w) {
  var mod = 0.5 - w * 0.5;
  return cos2(pd(x, mod));
}

function pd_square(x, w) {
  var mod = 0.5 - w * 0.5;
  var x2 = 0.0;
  if (x < mod) {
    x2 = x * 0.5 / mod;
  } else if (x < 0.5) {
    x2 = 0.5;
  } else if (x < 0.5 + mod) {
    x2 = (x - 0.5) * 0.5 / mod + 0.5;
  } else {
    x2 = 1.0;
  }
  return cos2(x2);
}

function pd_pulse(x, w) {
  var mod = 1.0 - w;
  var x2 = 0;
  if (x < mod) {
    x2 = x / mod;
  } else {
    x2 = 1.0;
  }
  return cos2(x2);
}

function pd_double_sine(x, w) {
  var mod = 1.0 - w;
  var x2 = 0;
  if (x < 0.5) {
    x2 = 2.0 * x;
  } else {
    x2 = 1.0 - (x - 0.5) / (0.5 * mod);
    if (x2 < 0) x2 = 0;
  }
  return cos2(x2);
}

function pd_saw_pulse(x, w) {
  var mod = 1.0 - w;
  var x2 = 0;
  if (x < 0.5) {
    x2 = x;
  } else {
    x2 = 0.5 - (x - 0.5) / mod;
    if (x2 < 0) x2 = 0;
  }
  return cos2(x2);
}

function pd_res1(x, w) {
  var mod = Math.exp(w * 6.0); // FIXME
  var x2 = x * mod;
  var window = 1.0 - x;
  return 1.0 - window * (1.0 - cos2(x2));
}

function pd_res2(x, w) {
  var mod = Math.exp(w * 6.0); // FIXME
  var x2 = x * mod;
  var window = x < 0.5 ? 2.0 * x : 2.0 * (1.0 - x);
  return 1.0 - window * (1.0 - cos2(x2));
}

function pd_res3(x, w) {
  var mod = Math.exp(w * 6.0); // FIXME
  var x2 = x * mod;
  var window = x < 0.5 ? 1.0 : 2.0 * (1.0 - x);
  return 1.0 - window * (1.0 - cos2(x2));
}

function pd_sin_half(x, w) {
  var mod = 0.5 + w * 0.5;
  return gb(sin2(0.5 * pd(x, mod))); 
}

// electronic

function el_saw(x) {
  return gb(x);
}

function el_double_saw(x, w) {
  if (x < w) {
    return gb(x / w);
  } else {
    return gb((x - w) / (1.0 - w));
  }
}

function tri(x, w) {
  if (x < w) {
    return x / w;
  } else {
    return 1 - (x-w) / (1-w);
  }
}

function el_tri(x, w) {
  return gb(tri(x, w));
}

function el_tri2(x, w) {
  var p = tri(x, w);
  var p2 = p * Math.sqrt(p) + (1-p) * p;
  return gb(p2);
}

function el_tri3(x, w) {
  var p = tri(x, w);
  var p2 = 0;
  if (x < w) {
    p2 = Math.sqrt(p);    
  } else {    
    p2 = p * p * p;
  }
  return gb(p2);
}

function el_pulse(x, w) {
  return gb(gpulse(x, w));
}

function el_pulse2(x, w) {
  // 0.75
  var min = 0.5 - 0.5 * w;
  var max = 0.5 + 0.5 * w;
  return (x > min && x < max) ? 1 : -1;
}

function el_pulse_saw(x, w) {
  var x2 = pd(x, w);
  if (x < w) {
    return x2 * 2.0;
  } else {
    return (x2 - 0.5) * -2.0;
  }
}

function el_slope(x, w) {
  return gb(gvslope(x, w));
}

function el_alpha1(x, w) {
  var pw = gpulse((2.0 * x) % 1.0, w);
  var saw = el_saw(x);
  return pw * (saw + 1.0) - 1.0;
}

function el_alpha2(x, w) {
  var pw = gpulse((4.0 * x) % 1.0, w);
  var saw = el_saw(x);
  return pw * (saw + 1.0) - 1.0;
}

function mix(f1, f2, c, x, w, t) {
  var s1 = f1(x, w, t);
  var s2 = f1((x + t) % 1.0, w, t);
  var p = f2((c * x) % 1.0, w, t);
  return s1 * p + s2 * (1-p);
}

function el_beta1(x, w, t) {
  return mix(el_saw, gpulse, 2.0, x, w, t);
}

function el_beta2(x, w, t) {
  return mix(el_saw, gpulse, 4.0, x, w, t);
}

function el_pulse_tri(x, w, t) {
  var pw = el_pulse2(x, w, t);
  var tw = el_tri(x, 0.5, t);
  return t * pw + (1-t) * tw;
}

// copied from lmms triple oscillator
function el_exp(x, w, t) {
  if (x > 0.5) {
    return -1.0 + 8.0 * (1.0 - x) * (1.0 - x);
  } else {
    return -1.0 + 8.0 * x * x;
  }
}

// fm

function fm1(x, w, t) {
  return sin2(x);
}

function fm2(x, w, t) {
  return sin2(0.5 * x);
}

function fm3(x, w, t) {
  var y = sin2(x);
  if (x > 0.25 && x < 0.75)
    return -y;
  else
    return y;  
}

function fm4(x, w, t) {
  if (x < 0.5)
    return sin2(2 * x)
  else
    return 0;
}

function fm5(x, w, t) {
  if (x < 0.5)
    return sin2(x)
  else
    return 0;
}

function fm6(x, w, t) {
  if (x < 0.25)
    return sin2(2*x, w, t)
  else if (x > 0.5 && x < 0.75)
    return sin2(2*(x-0.25), w, t)
  else
    return 0;
}

function fm7(x, w, t) {
  if (x < 0.25 || x > 0.5 && x < 0.75)
    return sin2(x, w, t)
  else
    return 0;
}

function fm8(x, w, t) {
  if (x < 0.25 || x > 0.5 && x < 0.75) 
    return sin2(x % 0.25, w, t);
  else
    return 0;
}

// additive synthesis

function as_saw(x, w, t) {
  var y = 0;
  for (var i = 1.0; i < (40.0 * t); i++) {
    y += sin2(i * x) * 1.0/i;
  }
  return -2/PI * y;
}

function as_square(x, w, t) {
  var y = 0;
  for (var i = 1.0; i < (40.0 * t); i += 2) {
    y += sin2(i * x) * 1.0/i;
  }
  return 4/PI * y;
}

function as_impulse(x, w, t) {
  var y = 0;
  for (var i = 1.0; i < (40.0 * t); i++) {
    y += sin2(i * x);
  }
  return 0.05 * y;
}

// noise

function no_white(x) {
  return gb(Math.random());
}

function update_plot() {
  var tone = parseFloat($("#tone").val());
  var width = parseFloat($("#width").val());
  
  $(".plot").each(function() {
    var fn = window[$(this).attr("data-fn")];    
    var arr = [];
    for (var p = 0.0; p < 1.5; p += 0.01) {
      arr.push([p, fn(fmod(p, 1.0), width, tone)])
    }
    $.plot(this, [arr]);
  });
}
 
$(document).ready(function() {
//  var d2 = [[0, 3], [4, 8], [8, 5], [9, 13]];
//  var d3 = [[0, 12], [7, 12], null, [7, 2.5], [12, 2.5]];
//  $.plot("#plot", [ d2, d3 ]);
  
  update_plot();
  $("input").change(update_plot);
});
