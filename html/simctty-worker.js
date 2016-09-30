importScripts("../build-js/simctty/simctty.js");
var sys_load_image = Module.cwrap('sys_load_image', 'number', ['array', 'number', 'number']);
var sys_run = Module.cwrap('sys_run', '', ['string', 'number', 'number']);
var sys_can_read = Module.cwrap('sys_can_read', 'number', []);
var sys_read = Module.cwrap('sys_read', 'number', []);
var sys_keypress = Module.cwrap('sys_keypress', '', ['number']);

onmessage = function(e) {
  if (e.data) {
    switch (e.data.cmd) {
    case "load_image":
      sys_load_image(e.data.data, e.data.data.length, 0x100);
      break;
    case "run":
      sys_run();

      while (sys_can_read()) {
        postMessage({cmd: "read", data: String.fromCharCode(sys_read())});
      }
      postMessage({cmd: "done"});
      break;
    case "keypress":
      for (var i = 0; i < e.data.keypress.length; i++) {
        sys_keypress(e.data.keypress.charCodeAt(i));
      }
      break;
    }
  }
}

