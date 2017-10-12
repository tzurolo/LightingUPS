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

var statusResQueue = [];
var responseLine = '';
var responseLineNum = 0;
var readingStatus = false;
var statusMessageDataJSON = [];
var insertDevEventStmt;

 var lastMicrocontrollerCommand = '';
 function sendCommandToMicrocontroller (cmd) {
    lastMicrocontrollerCommand = cmd;
    serialPort.write(cmd + "\r");
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
    // console.log("microcontroller: '" + message + '"');
    var tokens = message.trim().split(/\s+/);
    switch (tokens[0]) {
        case 'UI' :
        case 'UB' :
            console.log('got status string');
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
            sendCommandToMicrocontroller('status');
            break;
        case 'ID:' :
            var eventJSON =
                '{ "type": "event", "data": ' + '{' +
                   '"name": "ID", ' +
                   '"value": "' + tokens[1] + '"}' +
                '}';
            io.emit('controllerMessage', eventJSON);
            break;
        case 'Manual:' :
            var eventJSON =
                '{ "type": "event", "data": ' + '{' +
                   '"name": "Manual", ' +
                   '"value": "' + tokens[1] + '"}' +
                '}';
            io.emit('controllerMessage', eventJSON);
            break;
        case 'offset:' :
            console.log('temp cal offset ' + tokens[1]);
            var eventJSON =
                '{ "type": "event", "data": ' + '{' +
                   '"name": "Tcal", ' +
                   '"value": "' + tokens[1] + '"}' +
                '}';
            io.emit('controllerMessage', eventJSON);
            sendCommandToMicrocontroller('status');
            break;
        case '}' :
            console.log('end of status');
            sendCommandToMicrocontroller('get tcaloffset');
            //io.emit('controllerMessage', eventJSON);
            break;
        case 'unrecognized' :
            // command not recognized by microcontroller.
            if (lastMicrocontrollerCommand.length > 0) {
                // retry sending command
                sendCommandToMicrocontroller(lastMicrocontrollerCommand);
            }
            break;
        default :
            if (readingStatus) { 
            } else {
                // not reading status
            }
            break;
    }
}

io.on('connection', function(socket){
    console.log('got a connection.');
  socket.on('command', function(msg){
      sendCommandToMicrocontroller(msg);
      console.log('got command: ' + msg);
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
	      if (responseLine.length > 0) {
                  console.log(++responseLineNum + ': ' + responseLine);
		  processMicrocontrollerMessage(responseLine);
		  responseLine = '';
	      }
	      break;
	  default:
	      responseLine += dataStrChar;
	  }
     }
    // console.log('data received: ' + data);
  });
  serialPort.on('close', function() {
    console.log('serial port closed');
  });

  sendCommandToMicrocontroller('settings');
});


var port = 5401;
httpServer.listen(port, function(){
  console.log('listening on *:' + port);
});

