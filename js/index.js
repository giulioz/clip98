const ncp = require("copy-paste");
const SerialPort = require("serialport");
const Delimiter = require("@serialport/parser-delimiter");

const port = new SerialPort("/dev/ttys000");
const parser = new Delimiter({ delimiter: "\f" });
port.pipe(parser);

let lastClipboard = null;

parser.on("data", (data) => {
  const text = data.toString("utf-8");
  lastClipboard = text;
  console.log(text, data);
  ncp.copy(text);
});

setInterval(() => {
  const data = ncp.paste();
  if (lastClipboard !== data) {
    port.write(ncp.paste(), "ascii");
    lastClipboard = data;
  }
}, 1000);
