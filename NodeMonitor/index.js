var fs = require('fs');
var express = require('express');
var app = express();

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
    requestUnitInfo();
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
                var eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "swver", ' +
                       '"value": "' + currentSWVer + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                break;
            case '{' :
                console.log('got settings');
                var currentSettings = JSON.parse(message);
                var eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "ID", ' +
                       '"value": "' + currentSettings.ID + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                var eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Mode", ' +
                       '"value": "' + currentSettings.Mode + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                var eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Dark", ' +
                       '"value": "' + currentSettings.Dark + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                var eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Auto", ' +
                       '"value": "' + currentSettings.Auto + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Manual", ' +
                       '"value": "' + currentSettings.Manual + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
                console.log('end of status');
                //io.emit('controllerMessage', eventJSON);
                break;
            case 't' :
                var tokens = message.trim().split(/\s+/);
                currentTCalOffset = tokens[1];
                console.log('temp cal offset ' + currentTCalOffset);
                var eventJSON =
                    '{ "type": "event", "data": ' + '{' +
                       '"name": "Tcal", ' +
                       '"value": "' + currentTCalOffset + '"}' +
                    '}';
                io.emit('controllerMessage', eventJSON);
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
  socket.on('command', function(msg){
      cmdQueue.push(msg);
      console.log('got command from client: ' + msg);
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

