
var SimCTTY = function(filename, term) {
  this.filename = filename;
  this.term = term;
  this.working = false;

  var self = this;
  this.term.on("data", function(data) {
    if (self.worker) {
      self.worker.postMessage({cmd: "keypress", keypress: data});
    }
  });
}

SimCTTY.prototype.load = function() {
  this.print("skip.org/linux\r\n");

  if (this.binary) {
    this.start();
    return;
  }

  this.print("Loading " + this.filename + "...");

  var self = this;
  var fetcher = new XMLHttpRequest();
  fetcher.open("GET", this.filename, true);
  fetcher.responseType = "arraybuffer";

  fetcher.onload = function(e) {
    if (this.status == 200) {
      self.binary = new Uint8Array(fetcher.response);
      setTimeout(function() {self.start()}, 400);
    }
  }

  fetcher.onprogress = function(e) {
    self.print("...");
  }

  fetcher.send();
}

SimCTTY.prototype.start = function() {
  var self = this;

  if (this.worker) {
    this.stop();
    this.print("\r\n\r\n");
  }

  this.working = true;

  this.term.reset();
  this.term.focus();

  this.worker = new Worker("simctty-worker.js");
  this.worker.addEventListener("message", function(e) {
    switch (e.data.cmd) {
    case "read":
      self.print(e.data.data);
      break;
    case "done":
      if (self.working) {
        self.worker.postMessage({cmd: "run"});
      }
      break;
    }
  }, false);
  this.worker.postMessage({cmd: "load_image", data: this.binary});
  this.worker.postMessage({cmd: "run"});
}

SimCTTY.prototype.stop = function() {
  if (!this.worker) {
    return;
  }

  this.working = false;
  this.worker = undefined;
}

SimCTTY.prototype.print = function(text) {
  self.term.write(text);
}

