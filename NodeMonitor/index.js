var fs = require('fs');
var express = require('express');
var app = express();

var childProcess = require('child_process');
var exec = childProcess.exec;
var execFile = childProcess.execFile;

var http = require('http');
var socketIo = require('socket.io');

var httpServer = http.createServer(app);
var io = socketIo(httpServer);

var SerialPort = require("serialport");
var serialPort = new SerialPort("COM5", {
  baudRate: 300
});

var serialPortRxLine = '';
var serialPortRxLineNum = 0;
var serialPortTimeoutTimer = null;
var cmdQueue = [];
var currentSWVer = null;
var currentSettings = null;
var currentTCalOffset = null;

// requests info from the attached unit by queueing commands
function requestUnitInfo ()
{
    cmdQueue = [];
    cmdQueue.push("ver");
    cmdQueue.push("settings");
    cmdQueue.push("get tcaloffset");
    cmdQueue.push("status");
    sendCommandToMicrocontroller(cmdQueue[0]);
}

function serialPortTimedOut ()
{
    console.log('Serial port timeout');
    serialPortRxLine = '';
    requestUnitInfo();
    if (currentSettings !== null) {
        currentSWVer = null;
        currentSettings = null;
        currentTCalOffset = null;
        sendSWVersionToClient(currentSWVer);
        sendSettingsToClient(currentSettings);
        sendTCalOffsetToClient(currentTCalOffset);
    }
}

 function sendCommandToMicrocontroller (cmd) {
    serialPort.write(cmd + "\r");
    // start timeout timer
    serialPortTimeoutTimer = setTimeout(serialPortTimedOut, 3000);
 }

 function queryStatus (res) {
 }
 

//
//  routes
//
app.get('/', function(req, res){
    res.sendFile(__dirname + '/index.html');
});
app.get('/home', function(req, res){
    res.sendFile(__dirname + '/home.html');
});
app.get('/status', function(req, res) {
    queryStatus(res);
});
app.get('/w3.css', function(req, res){
    res.sendFile(__dirname + '/w3.css');
});

// convert temperature from Centigrade to Fahrenheit
function convertTempToF (
    degC)
{
    var degF = ((parseFloat(degC) * 9.0) / 5.0) + 32.0;
    return degF.toFixed(1) + '&#x2109';
}

function sendSettingsToClient (
    settings)
{
    if (settings == null) {
        settings = {
            "ID": "-",
            "Mode": "-",
            "Dark": "-",
            "Auto": "-",
            "Manual": "-"
        };
    }
    var eventJSON =
        '{ "type": "event", "data": ' + '{' +
            '"name": "ID", ' +
            '"value": "' + settings.ID + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
    eventJSON =
        '{ "type": "event", "data": ' + '{' +
            '"name": "Mode", ' +
            '"value": "' + settings.Mode + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
    eventJSON =
        '{ "type": "event", "data": ' + '{' +
            '"name": "Dark", ' +
            '"value": "' + settings.Dark + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
    eventJSON =
        '{ "type": "event", "data": ' + '{' +
            '"name": "Auto", ' +
            '"value": "' + settings.Auto + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
    eventJSON =
        '{ "type": "event", "data": ' + '{' +
            '"name": "Manual", ' +
            '"value": "' + settings.Manual + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
}

function sendSWVersionToClient (
    swver)
{
    if (swver == null) {
        swver = '-';
    }
    var eventJSON =
        '{ "type": "event", "data": ' + '{' +
            '"name": "swver", ' +
            '"value": "' + swver + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
}

function sendTCalOffsetToClient (
    tcaloffset)
{
    if (tcaloffset == null) {
        tcaloffset = '-'
    }
    var eventJSON =
        '{ "type": "event", "data": ' + '{' +
            '"name": "Tcal", ' +
            '"value": "' + tcaloffset + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
}

function processMicrocontrollerMessage (
    message)
{
    clearTimeout(serialPortTimeoutTimer);
    var gotExpectedResponse = true; // empty message is a valid response
    if (message.length > 0) {
        switch (message[0]) {
            case 'U' :
                console.log('got status string');
                var tokens = message.trim().split(/\s+/);
                var eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "BV", ' +
                       '"value": "' + tokens[1] + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "AC", ' +
                       '"value": "' + tokens[2] + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "C", ' +
                       '"value": "' + tokens[3] + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Light", ' +
                       '"value": "' + tokens[4] + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Motion", ' +
                       '"value": "' + tokens[5] + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Temp", ' +
                       '"value": "' + tokens[6] + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                break;
            case 'V' :
                currentSWVer = message;
                console.log('sw version: ' + currentSWVer);
                sendSWVersionToClient(currentSWVer);
                break;
            case '{' :
                console.log('got settings');
                if (message[message.length-1] == '}') {
                    currentSettings = JSON.parse(message);
                    sendSettingsToClient(currentSettings);
                    console.log('end of status');
                } else {
                    gotExpectedResponse = false;
                }
                break;
            case 't' :
                var tokens = message.trim().split(/\s+/);
                currentTCalOffset = tokens[1];
                console.log('temp cal offset ' + currentTCalOffset);
                sendTCalOffsetToClient(currentTCalOffset);
                break;
            default :
                gotExpectedResponse = false;
                break;
        }
    }
    if (gotExpectedResponse) {
        cmdQueue.shift();
    } else {
        console.log('retrying ' + cmdQueue[0]);
    }
    if (cmdQueue.length == 0) {
        cmdQueue.push('status');
    }
    if (cmdQueue.length > 0) {
        sendCommandToMicrocontroller(cmdQueue[0]);
    }
}

io.on('connection', function(socket){
    console.log('got a connection.');
    socket.on('serverCommand', function(msg){
        console.log('got server command from client: ' + msg);
        switch (msg) {
            case 'refresh' :
                sendSWVersionToClient(currentSWVer);
                sendSettingsToClient(currentSettings);
                sendTCalOffsetToClient(currentTCalOffset);
                break;
            case 'reflash' :
                //  child process for re-flashing firmware with avrdude
                var child = execFile('avrdude', [
                    '-p', 'attiny84',
                    '-P', 'COM4',
                    '-c', 'avrispv2',
                    '-e',
                    '-U',
                    'flash:w:C:\\files\\LightingUPS\\firmware\\default\\LightingUPS.hex:i' ]);
                child.stdin.setEncoding('utf-8');

                child.stdout.on('data', function (data) {
                    console.log('child stdout: "' + data + '"');
                });

                child.stderr.on('data', function (data) {
                    var logmsg = '';
                    for (i = 0; i < data.length; ++i) {
                        var c = data.charCodeAt(i);
                        if ((c < 32) || (c > 126)) {
                            logmsg += ('(' + c + ')');
                        } else {
                            logmsg += data[i];
                        }
                    }
                    console.log('child stderr: "' + logmsg + '"');
                });

                child.on('close', function () {
                    console.log('child exited');
                });

                break;
            default :
                console.log('unrecognized server command');
        }
  });

    socket.on('unitCommand', function(msg){
      cmdQueue.push(msg);
      console.log('got unit command from client: ' + msg);
  });
  socket.on('disconnect', function () {
    console.log('lost connection.');
  });
});

serialPort.on("open", function () {
  console.log('serial port open');
  serialPort.on('data', function(data) {
      var dataStr = data.toString();
      for (i = 0; i < dataStr.length; i++) {
	  var dataStrChar = dataStr[i];
	  switch(dataStrChar) {
	  case '\n' :
	      break;
	  case '\r' :
	      if (serialPortRxLine.length > 0) {
                  console.log(++serialPortRxLineNum + ': ' + serialPortRxLine);
		  processMicrocontrollerMessage(serialPortRxLine);
		  serialPortRxLine = '';
	      }
	      break;
	  default:
	      serialPortRxLine += dataStrChar;
	  }
     }
    // console.log('data received: ' + data);
  });
  serialPort.on('close', function() {
    console.log('serial port closed');
  });

  requestUnitInfo();
});

var port = 5401;
httpServer.listen(port, function(){
  console.log('listening on *:' + port);
});

